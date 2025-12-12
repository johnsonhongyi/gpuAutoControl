// GPU频率控制辅助函数实现
#include "stdafx.h"
#include "Core.h"

// 温度保护判断
bool CCore::ShouldReduceForTemperature()
{
	return m_GpuInfo.m_nGPU_Temp >= m_config.downTemplimit
		&& m_GpuInfo.m_nGraphicsClock > m_config.baseClockLimit;
}

// 低利用率降频判断
bool CCore::ShouldReduceForLowUtil()
{
	return m_GpuInfo.m_nGPU_Util > m_config.utilIdleThreshold
		&& m_GpuInfo.m_nGPU_Util < m_config.downClockPercent
		&& m_GpuInfo.m_nGraphicsClock > m_config.baseClockLimit;
}

// 高利用率升频判断
bool CCore::ShouldIncreaseFrequency()
{
	return m_config.LockGPUFrequency
		&& m_GpuInfo.m_nGPU_Util > m_config.upClockPercent
		&& m_GpuInfo.m_nGraphicsClock >= m_config.lowClockLimit;
}

// 空闲解锁判断
bool CCore::ShouldUnlockForIdle()
{
	return m_config.LockGPUFrequency
		&& m_GpuInfo.m_nGPU_Util < m_config.utilIdleThreshold
		&& m_GpuInfo.m_nGPU_Temp < m_config.upTemplimit;
}

// 降频计算
int CCore::ReduceFrequency(int ratio)
{
	int newFreq = int(m_GpuInfo.m_nGraphicsClock * ratio / 100.0);
	newFreq = RoundToNearest10(newFreq);
	return max(newFreq, m_config.baseClockLimit);
}

// 升频计算(根据温度)
int CCore::IncreaseFrequency()
{
	int ratio;
	if (m_GpuInfo.m_nGPU_Util >= m_config.downClockPercent 
		&& m_GpuInfo.m_nGPU_Temp <= m_config.tempCoolThreshold)
		ratio = m_config.aggressiveUpRatio; // 激进升频
	else if (m_GpuInfo.m_nGPU_Temp <= m_config.upTemplimit)
		ratio = m_config.upClockRatio; // 保守升频
	else if (m_GpuInfo.m_nGPU_Temp <= m_config.downTemplimit)
		return m_GpuInfo.m_nGraphicsClock; // 维持
	else
		return ReduceFrequency(m_config.downClockRatio); // 降频
	
	int newFreq = int(m_GpuInfo.m_nGraphicsClock * ratio / 100.0);
	newFreq = RoundToNearest10(newFreq);
	return min(newFreq, m_config.upClocklimit);
}

// 频率取整到10MHz
int CCore::RoundToNearest10(int freq)
{
	return ((freq + 5) / 10) * 10;
}

// 应用频率变化并记录日志
void CCore::ApplyFrequencyChange(int newFreq, const char* reason)
{
	LOG("GPU频率调整: %d → %d MHz (原因: %s, 占用率: %d%%, 温度: %d°C)",
		m_GpuInfo.m_nGraphicsClock, newFreq, reason,
		m_GpuInfo.m_nGPU_Util, m_GpuInfo.m_nGPU_Temp);
	
	m_config.GPUFrequency = newFreq;
	m_config.LockGPUFrequency = (newFreq > 0);
}

// 比例判断辅助函数
bool CCore::CheckRatio(std::vector<bool>& history, bool condition, int window, double ratio)
{
	history.push_back(condition);
	
	// 保持历史记录长度不超过窗口大小
	if (history.size() > (size_t)window)
	{
		history.erase(history.begin());
	}
	
	// 如果由于窗口变小导致历史记录过长
	while (history.size() > (size_t)window)
	{
		history.erase(history.begin());
	}

	// 只有当积累了足够的数据（至少达到窗口大小）才进行判断
	// 或者您可以根据需求决定是否允许由于初期数据不足时的判断
	// 这里假设必须填满窗口
	if (history.size() < (size_t)window)
		return false;

	int hits = 0;
	for (bool b : history)
	{
		if (b) hits++;
	}

	double currentRatio = (double)hits / history.size();
	if (currentRatio >= ratio) // 超过设定比例(如80%)
	{
		history.clear(); // 触发后清空历史，重新开始计数
		return true;
	}

	return false;
}

// 计算目标频率(主控制函数)
int CCore::CalculateTargetFrequency()
{
	if (!m_config.TakeOver) return 0;
	
	// 每次都记录当前状态(用于调试)
	static int logCounter = 0;
	if (++logCounter >= 10) // 每10次记录一次,避免日志过多
	{
		LOG("GPU状态: 频率=%dMHz, 占用率=%d%%, 温度=%d°C, 锁定=%d",
			m_GpuInfo.m_nGraphicsClock, m_GpuInfo.m_nGPU_Util, m_GpuInfo.m_nGPU_Temp,
			m_config.LockGPUFrequency);
		logCounter = 0;
	}
	
	// 1. 温度保护 (维持立即触发，因为这是安全机制)
	if (ShouldReduceForTemperature())
	{
		// 清空其他历史记录，避免冲突
		m_vHistoryDown.clear();
		m_vHistoryUp.clear();
		m_vHistoryLock.clear();
		
		int newFreq = ReduceFrequency(m_config.downClockRatio);
		ApplyFrequencyChange(newFreq, "温度保护");
		return newFreq;
	}
	
	// 获取timelimit作为窗口大小，最小为3
	int windowSize = max(3, m_config.timelimit);
	double triggerRatio = 0.8; // 80% 触发阈值

	// 2. 降频节能
	bool bDownCondition = ShouldReduceForLowUtil();
	if (CheckRatio(m_vHistoryDown, bDownCondition, windowSize, triggerRatio))
	{
		int newFreq = ReduceFrequency(m_config.downClockRatio);
		ApplyFrequencyChange(newFreq, "低利用率节能(>80%)");
		// 触发后重置其他相关的历史记录
		m_vHistoryUp.clear();
		return newFreq;
	}
	
	// 3. 升频提升性能
	bool bUpCondition = ShouldIncreaseFrequency();
	if (CheckRatio(m_vHistoryUp, bUpCondition, windowSize, triggerRatio))
	{
		int newFreq = IncreaseFrequency();
		ApplyFrequencyChange(newFreq, "高利用率升频(>80%)");
		// 触发后重置其他相关的历史记录
		m_vHistoryDown.clear();
		return newFreq;
	}
	
	// 4. 空闲解锁
	bool bLockCondition = ShouldUnlockForIdle();
	if (CheckRatio(m_vHistoryLock, bLockCondition, windowSize, triggerRatio))
	{
		int newFreq = m_config.GPU_LockClock;
		ApplyFrequencyChange(newFreq, "空闲解锁(>80%)");
		return newFreq;
	}
	
	return 0; // 无变化
}
