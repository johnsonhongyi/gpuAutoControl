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

// 计算目标频率(主控制函数)
int CCore::CalculateTargetFrequency()
{
	if (!m_config.TakeOver) return 0;
	
	// 每次都记录当前状态(用于调试)
	static int logCounter = 0;
	if (++logCounter >= 10) // 每10次记录一次,避免日志过多
	{
		LOG("GPU状态: 频率=%dMHz, 占用率=%d%%, 温度=%d°C, 锁定=%d, Down=%d, Up=%d, Lock=%d",
			m_GpuInfo.m_nGraphicsClock, m_GpuInfo.m_nGPU_Util, m_GpuInfo.m_nGPU_Temp,
			m_config.LockGPUFrequency, m_config.TakeOverDown, m_config.TakeOverUp, m_config.TakeOverLock);
		logCounter = 0;
	}
	
	// 温度保护优先
	if (ShouldReduceForTemperature())
	{
		int newFreq = ReduceFrequency(m_config.downClockRatio);
		ApplyFrequencyChange(newFreq, "温度保护");
		return newFreq;
	}
	
	// 降频节能
	if (ShouldReduceForLowUtil())
	{
		m_config.TakeOverDown++;
		if (m_config.TakeOverDown >= m_config.timelimit)
		{
			int newFreq = ReduceFrequency(m_config.downClockRatio);
			ApplyFrequencyChange(newFreq, "低利用率节能");
			m_config.TakeOverDown = 0;
			m_config.TakeOverUp = 0;
			return newFreq;
		}
	}
	else
	{
		m_config.TakeOverDown = 0; // 条件不满足时重置计数器
	}
	
	// 升频提升性能
	if (ShouldIncreaseFrequency())
	{
		m_config.TakeOverUp++;
		if (m_config.TakeOverUp >= m_config.timelimit)
		{
			int newFreq = IncreaseFrequency();
			ApplyFrequencyChange(newFreq, "高利用率升频");
			m_config.TakeOverUp = 0;
			m_config.TakeOverDown = 0;
			return newFreq;
		}
	}
	else
	{
		m_config.TakeOverUp = 0; // 条件不满足时重置计数器
	}
	
	// 空闲解锁
	if (ShouldUnlockForIdle())
	{
		m_config.TakeOverLock++;
		if (m_config.TakeOverLock >= m_config.timelimit)
		{
			int newFreq = m_config.GPU_LockClock;
			ApplyFrequencyChange(newFreq, "空闲解锁");
			m_config.TakeOverLock = 0;
			return newFreq;
		}
	}
	else
	{
		m_config.TakeOverLock = 0; // 条件不满足时重置计数器
	}
	
	return 0; // 无变化
}
