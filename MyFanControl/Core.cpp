#include "stdafx.h"
#include "Core.h"

#include <Windows.h>
#include <direct.h>
#include <iostream>

//#include <wtsapi32.h> 
#include <tlhelp32.h>
#include <tchar.h>

int TEMP_LIST[10] = { 90, 85, 80, 75, 70, 65, 60, 55, 50, 45 };


int GetTime(tm *pt, int offset)
{
	tm t = { 0 };
	time_t tt;
	if (!pt)
		pt = &t;
	time(&tt);
	tt += offset;
	localtime_s(pt, &tt);
	return (pt->tm_hour * 10000 + pt->tm_min * 100 + pt->tm_sec);
}
int GetTimeInterval(int a, int b, int *p)
{
	//时间差，输入两个6位数时间，如开盘时间91500，得到a-b，并转化为6位数时间，指针p接受以秒计的时间差
	int a1 = a / 10000;
	int a2 = (a % 10000) / 100;
	int a3 = a % 100;

	int b1 = b / 10000;
	int b2 = (b % 10000) / 100;
	int b3 = b % 100;

	int c = (a1 - b1) * 3600 + (a2 - b2) * 60 + a3 - b3;
	if (p)
		*p = c;
	int sgn = (c >= 0) ? 1 : -1;
	c = abs(c);

	int d = (c / 3600) * 10000 + (c % 3600) / 60 * 100 + c % 60;
	return d*sgn;
}
CString GetExePath(){
	char pathbuf[1024] = { 0 };
	int pathlen = ::GetModuleFileName(NULL, pathbuf, 1024);

	while (TRUE)
	{
		if (pathbuf[pathlen--] == '\\')
		{
			break;
		}
	}
	pathbuf[++pathlen] = 0x0;
	CString fname = pathbuf;
	return fname;
}

int GetDisplayFrequency()
{
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);
	if (0 != EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
	{
		return dm.dmDisplayFrequency;
	}
	else
		return 0;
}

int GetBatteryLevel()
{
	SYSTEM_POWER_STATUS status;
	GetSystemPowerStatus(&status);
	return status.BatteryLifePercent;
}

int GetBatteryACLineStatus()
{
	SYSTEM_POWER_STATUS status;
	GetSystemPowerStatus(&status);
	return status.ACLineStatus;
}

BOOL FileExists(CString fileName)
{
	DWORD fileAttr;
	if (strlen(fileName) > 1)
	{
		fileAttr = GetFileAttributes(fileName);
		if (0xFFFFFFFF == fileAttr && GetLastError() == ERROR_FILE_NOT_FOUND)
			return false;
		return true;
	}
	return false;
}

CString StringToCString(string str)
 
{
 
	CString result;
 
	for (int i=0;i<(int)str.length();i++)
 
	{
 
	   result+=str[i];
 
	}
 
	return result;
 
}

string CStringToString(CString cstr)

{
	string result(cstr.GetLength(), 'e');
	for (int i = 0;i < cstr.GetLength();i++)
	{
		result[i] = (char)cstr[i];
	}
	return result;
}

std::wstring ANSIToUnicode(const std::string& from)
{
	if (from.empty())
	{
		return std::wstring();
	}

	size_t unicodeLen = 0;
	wchar_t* pUnicode = nullptr;
	if (unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, from.c_str(), -1, NULL, 0))
	{
		pUnicode = new wchar_t[unicodeLen + 1];
		memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
		if (!::MultiByteToWideChar(CP_ACP, 0, from.c_str(), -1, (LPWSTR)pUnicode, unicodeLen))
		{
			delete[] pUnicode;//clean up if failed;  
			return std::wstring();
		}
	}
	else
	{
		return std::wstring();
	}
	std::wstring strRet(pUnicode);
	delete[] pUnicode;//clean up if success;

	return std::move(strRet);
}


DWORD FindProcessIDByName(const std::string& processName)//0 not found ; other found; processName "processName.exe"
{
	//processName = CStringToString(processName);
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(0);
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(0);
	}
	DWORD processId = 0;
	do
	{
		//if (std::wstring(pe32.szExeFile) == ANSIToUnicode(processName))//进程名称
		//printf("name:%ls",ANSIToUnicode(processName));
		//printf("pe32:%ls",std::wstring(pe32.szExeFile));
		//if (std::wstring(pe32.szExeFile) == ANSIToUnicode(processName))//进程名称
		if (std::string(pe32.szExeFile) == (processName))//进程名称


		{
			processId = pe32.th32ProcessID;//进程ID
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
	return(processId);
}

CGPUInfo::CGPUInfo()
{
	if (Init())
	{
		Update();

		TRACE0("成功加载NVGPU_DLL.dll。\n");

	}
	//

}

bool NvApi = true, Adl = false;
bool LogFileEnable = true;
unsigned int gpuBusId = 0;
void* NvApiGpuHandles[128] = { 0 };

//unsigned int AdlGpuIndexes[128] = { 0 };

//void* AdlContext = 0;

// #define LOG(expression) Log(#expression, strrchr(__FILE__, '\\') + 1, __LINE__, (intptr_t) (expression))
// ==== LOG宏 ====


//#define LOG_EXPR(expression) \
//    LogExpr(#expression, strrchr(__FILE__, '\\') + 1, __LINE__, (intptr_t)(expression))
//#define LOG_PRINTF(...) LogPrintf(__VA_ARGS__)
//#define GET_MACRO(_1,_2,_3,_4,_5,_6,NAME,...) NAME
//#define LOG(...) \
//    GET_MACRO(__VA_ARGS__, \
//              LOG_PRINTF, LOG_PRINTF, LOG_PRINTF, LOG_PRINTF, LOG_PRINTF, LOG_EXPR)(__VA_ARGS__)


#include <cstdarg>

//extern FILE* LogFile;
//FILE* LogFile = nullptr;

FILE* LogFile = 0;

//#define LOG(X, Y) fprintf (LogFile, #X ": Time:%s, File:%s(%d) " #Y  "\n", __TIMESTAMP__, __FILE__, __LINE__)
//#define LOG(X, Y) fprintf (LogFile, #X ": Time:%s, File:%s(%d) " #Y  "\n", __TIMESTAMP__, __FILE__, __LINE__)


#define MAKE_NVAPI_VERSION(type, version) ((unsigned int)(sizeof(type) | ((version) << 16)))

struct NV_GPU_PERF_PSTATES20_PARAM_DELTA
{
	int value;

	struct
	{
		int min;
		int max;
	} valueRange;
};

struct NV_GPU_PSTATE20_BASE_VOLTAGE_ENTRY
{
	unsigned int domainId;
	unsigned int editable : 1;
	unsigned int reserved : 31;
	unsigned int voltageUV;
	NV_GPU_PERF_PSTATES20_PARAM_DELTA voltageDeltaUV;
};

struct NV_GPU_PSTATE20_CLOCK_ENTRY
{
	unsigned int domainId;
	unsigned int typeId;
	unsigned int editable : 1;
	unsigned int reserved : 31;
	NV_GPU_PERF_PSTATES20_PARAM_DELTA frequencyDeltaKHz;

	union
	{
		struct
		{
			unsigned int frequencyKHz;
		} single;

		struct
		{
			unsigned int minFrequencyKHz;
			unsigned int maxFrequencyKHz;
			unsigned int domainId;
			unsigned int minVoltageUV;
			unsigned int maxVoltageUV;
		} range;
	} data;
};

struct NV_GPU_PERF_PSTATES20_INFO
{
	unsigned int version;
	unsigned int editable : 1;
	unsigned int reserved : 31;
	unsigned int numPStates;
	unsigned int numClocks;
	unsigned int numBaseVoltages;

	struct
	{
		unsigned int pStateId;
		unsigned int editable : 1;
		unsigned int reserved : 31;
		NV_GPU_PSTATE20_CLOCK_ENTRY clocks[8];
		NV_GPU_PSTATE20_BASE_VOLTAGE_ENTRY baseVoltages[4];
	} pStates[16];

	struct
	{
		unsigned int numVoltages;
		NV_GPU_PSTATE20_BASE_VOLTAGE_ENTRY voltages[4];
	} overVoltage;
};

struct NV_GPU_CLOCK_LOCK
{
	unsigned int version;
	unsigned int flags;
	unsigned int count;

	struct
	{
		unsigned int index;
		unsigned int unknown1;
		unsigned int mode;
		unsigned int unknown2;
		unsigned int voltageUV;
		unsigned int unknown3;
	} entries[32];
};

struct NV_GPU_POWER_STATUS
{
	unsigned int version;
	unsigned int count;

	struct
	{
		unsigned int unknown1;
		unsigned int unknown2;
		unsigned int power;
		unsigned int unknown3;
	} entries[4];
};

struct NV_GPU_THERMAL_LIMIT
{
	unsigned int version;
	unsigned int count;

	struct
	{
		unsigned int controller;
		unsigned int value;
		unsigned int flags;
	} entries[4];
};

struct NV_GPU_COOLER_LEVEL
{
	unsigned int version;

	struct
	{
		unsigned int level;
		unsigned int policy;
	} coolers[3];
};

struct NV_GPU_SET_ILLUMINATION_PARM
{
	unsigned int version;
	void* gpuHandle;
	unsigned int attribute;
	unsigned int value;
};

struct NV_GPU_CLOCK_MASKS
{
	unsigned int version;
	unsigned char mask[32];
	unsigned char unknown1[32];

	struct
	{
		unsigned int clockType;
		unsigned char enabled;
		unsigned char unknown2[19];
	} clocks[255];
};

struct NV_GPU_VFP_CURVE
{
	unsigned int version;
	unsigned char mask[32];
	unsigned char unknown1[32];

	struct
	{
		unsigned int clockType;
		unsigned int frequencyKHz;
		unsigned int voltageUV;
		unsigned char unknown2[16];
	} clocks[255];
};

struct NV_GPU_CLOCK_TABLE
{
	unsigned int version;
	unsigned char mask[32];
	unsigned char unknown1[32];

	struct
	{
		unsigned int clockType;
		unsigned char unknown2[16];
		int frequencyDeltaKHz;
		unsigned char unknown3[12];
	} clocks[255];
};

void* (__cdecl* NvAPI_QueryInterface)(unsigned int offset) = 0;
int(__cdecl* NvAPI_Initialize)() = 0;
int(__cdecl* NvAPI_Unload)() = 0;
int(__cdecl* NvAPI_RestartDisplayDriver)() = 0;
int(__cdecl* NvAPI_EnumPhysicalGPUs)(void** gpuHandles, unsigned int* gpuCount) = 0;
int(__cdecl* NvAPI_EnumTCCPhysicalGPUs)(void** gpuHandles, unsigned int* gpuCount) = 0;
int(__cdecl* NvAPI_GPU_SetPstates20)(void* gpuHandle, NV_GPU_PERF_PSTATES20_INFO* pStatesInfo) = 0;
int(__cdecl* NvAPI_GPU_GetClockBoostLock)(void* gpuHandle, NV_GPU_CLOCK_LOCK* clockLock) = 0;
int(__cdecl* NvAPI_GPU_SetClockBoostLock)(void* gpuHandle, NV_GPU_CLOCK_LOCK* clockLock) = 0;
int(__cdecl* NvAPI_GPU_ClientPowerPoliciesSetStatus)(void* gpuHandle, NV_GPU_POWER_STATUS* powerStatus) = 0;
int(__cdecl* NvAPI_GPU_ClientThermalPoliciesSetLimit)(void* gpuHandle, NV_GPU_THERMAL_LIMIT* thermalLimit) = 0;
int(__cdecl* NvAPI_GPU_SetCoolerLevels)(void* gpuHandle, unsigned int coolerIndex, NV_GPU_COOLER_LEVEL* coolerLevel) = 0;
int(__cdecl* NvAPI_GPU_SetIllumination)(NV_GPU_SET_ILLUMINATION_PARM* illuminationInfo) = 0;
int(__cdecl* NvAPI_GPU_SetPstate)(void* gpuHandle, unsigned int pState, unsigned int setType) = 0;
int(__cdecl* NvAPI_GPU_SetPstateClientLimits)(void* gpuHandle, unsigned int limitType, unsigned int pState) = 0;
int(__cdecl* NvAPI_GPU_EnableDynamicPstates)(void* gpuHandle, unsigned int dynamicPStates) = 0;
int(__cdecl* NvAPI_GPU_EnableOverclockedPstates)(void* gpuHandle, unsigned int overclockedPStates) = 0;
int(__cdecl* NvAPI_GPU_GetBusId)(void* gpuHandle, unsigned int* busId) = 0;
int(__cdecl* NvAPI_GPU_GetClockBoostMask)(void* gpuHandle, NV_GPU_CLOCK_MASKS* clockMasks) = 0;
int(__cdecl* NvAPI_GPU_GetVFPCurve)(void* gpuHandle, NV_GPU_VFP_CURVE* vfpCurve) = 0;
int(__cdecl* NvAPI_GPU_GetClockBoostTable)(void* gpuHandle, NV_GPU_CLOCK_TABLE* clockTable) = 0;
int(__cdecl* NvAPI_GPU_SetClockBoostTable)(void* gpuHandle, NV_GPU_CLOCK_TABLE* clockTable) = 0;



intptr_t Log(const char* expression, const char* fileName, unsigned int line, intptr_t result)
{
	if (LogFile)
	{
		time_t t = time(0);
		tm* local = localtime(&t);

		//fprintf(LogFile, "[%02d.%02d.%04d %02d:%02d:%02d][%8s:%04d] %-102s %-20zd (0x%0*zX)\n",
			//local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec, fileName, line, expression, result, (unsigned int)sizeof(result) * 2, result);
		
		//fprintf(LogFile, expression,"\n");
		//fprintf(LogFile, "[%02d.%02d.%04d %02d:%02d:%02d][%8s:%04d] %-60s %-6zd\n",
		fprintf(LogFile, "[%02d.%02d.%04d %02d:%02d:%02d][%8s:%04d]",
			local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec, fileName, line);
			//local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec, fileName, line, expression, result);
		fprintf(LogFile,"set: %-12zd ", result);
		fprintf(LogFile,expression );
		fprintf(LogFile, " \n");


		//fprintf(LogFile, #X ": Time:%s, File:%s(%d) " #Y  "\n", __TIMESTAMP__, __FILE__, __LINE__)
	
		fflush(LogFile);
	}

	return result;
}

#include <ctime>
#include <cstdio>

intptr_t LogExpr(const char* expr,
	const char* fileName,
	unsigned int line,
	intptr_t result)
{
	if (!LogFile)
		return result;

	time_t t = time(nullptr);
	tm local{};
	localtime_s(&local, &t);

	fprintf(LogFile,
		"[%02d.%02d.%04d %02d:%02d:%02d][%s:%04u] ",
		local.tm_mday,
		local.tm_mon + 1,
		local.tm_year + 1900,
		local.tm_hour,
		local.tm_min,
		local.tm_sec,
		fileName,
		line);

	fprintf(LogFile, "result:%td  %s\n", result, expr);

	fflush(LogFile);
	return result;
}

void LogPrintf(const char* fileName, unsigned int line, const char* fmt, ...)
{
	if (!LogFile)
		return;

	time_t t = time(nullptr);
	tm local{};
	localtime_s(&local, &t);

	fprintf(LogFile,
		"[%02d.%02d.%04d %02d:%02d:%02d][%s:%04u] ",
		local.tm_mday,
		local.tm_mon + 1,
		local.tm_year + 1900,
		local.tm_hour,
		local.tm_min,
		local.tm_sec,
		fileName,
		line);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(LogFile, fmt, ap);
	va_end(ap);

	fprintf(LogFile, "\n");
	fflush(LogFile);
}


int NvApiLoad()
{
	int result = -1;

	HMODULE nvapi = 0;

#if defined _WIN64
	char nvapiDllName[] = "nvapi64.dll";
#else
	char nvapiDllName[] = "nvapi.dll";
#endif

	LOG(nvapi = LoadLibraryA(nvapiDllName));

	result = !(nvapi != 0);

	if (result == 0)
	{
		(NvAPI_QueryInterface = (void* (__cdecl*)(unsigned int)) GetProcAddress(nvapi, "nvapi_QueryInterface"));

		result = !(NvAPI_QueryInterface != 0);

		if (result == 0)
		{
			NvAPI_Initialize = (int(__cdecl*)()) NvAPI_QueryInterface(0x0150E828);
			NvAPI_Unload = (int(__cdecl*)()) NvAPI_QueryInterface(0xD22BDD7E);
			NvAPI_RestartDisplayDriver = (int(__cdecl*)()) NvAPI_QueryInterface(0xB4B26B65);
			NvAPI_EnumPhysicalGPUs = (int(__cdecl*)(void**, unsigned int*)) NvAPI_QueryInterface(0xE5AC921F);
			NvAPI_EnumTCCPhysicalGPUs = (int(__cdecl*)(void**, unsigned int*)) NvAPI_QueryInterface(0xD9930B07);
			NvAPI_GPU_SetPstates20 = (int(__cdecl*)(void*, NV_GPU_PERF_PSTATES20_INFO*)) NvAPI_QueryInterface(0x0F4DAE6B);
			NvAPI_GPU_GetClockBoostLock = (int(__cdecl*)(void*, NV_GPU_CLOCK_LOCK*)) NvAPI_QueryInterface(0xE440B867);
			NvAPI_GPU_SetClockBoostLock = (int(__cdecl*)(void*, NV_GPU_CLOCK_LOCK*)) NvAPI_QueryInterface(0x39442CFB);
			NvAPI_GPU_ClientPowerPoliciesSetStatus = (int(__cdecl*)(void*, NV_GPU_POWER_STATUS*)) NvAPI_QueryInterface(0xAD95F5ED);
			NvAPI_GPU_ClientThermalPoliciesSetLimit = (int(__cdecl*)(void*, NV_GPU_THERMAL_LIMIT*)) NvAPI_QueryInterface(0x34C0B13D);
			NvAPI_GPU_SetCoolerLevels = (int(__cdecl*)(void*, unsigned int, NV_GPU_COOLER_LEVEL*)) NvAPI_QueryInterface(0x891FA0AE);
			NvAPI_GPU_SetIllumination = (int(__cdecl*)(NV_GPU_SET_ILLUMINATION_PARM*)) NvAPI_QueryInterface(0x0254A187);
			NvAPI_GPU_SetPstate = (int(__cdecl*)(void*, unsigned int, unsigned int)) NvAPI_QueryInterface(0x025BFB10);
			NvAPI_GPU_SetPstateClientLimits = (int(__cdecl*)(void*, unsigned int, unsigned int)) NvAPI_QueryInterface(0xFDFC7D49);
			NvAPI_GPU_EnableDynamicPstates = (int(__cdecl*)(void*, unsigned int)) NvAPI_QueryInterface(0xFA579A0F);
			NvAPI_GPU_EnableOverclockedPstates = (int(__cdecl*)(void*, unsigned int)) NvAPI_QueryInterface(0xB23B70EE);
			NvAPI_GPU_GetBusId = (int(__cdecl*)(void*, unsigned int*)) NvAPI_QueryInterface(0x1BE0B8E5);
			NvAPI_GPU_GetClockBoostMask = (int(__cdecl*)(void*, NV_GPU_CLOCK_MASKS*)) NvAPI_QueryInterface(0x507B4B59);
			NvAPI_GPU_GetVFPCurve = (int(__cdecl*)(void*, NV_GPU_VFP_CURVE*)) NvAPI_QueryInterface(0x21537AD4);
			NvAPI_GPU_GetClockBoostTable = (int(__cdecl*)(void*, NV_GPU_CLOCK_TABLE*)) NvAPI_QueryInterface(0x23F1B133);
			NvAPI_GPU_SetClockBoostTable = (int(__cdecl*)(void*, NV_GPU_CLOCK_TABLE*)) NvAPI_QueryInterface(0x0733E009);
		}
	}

	return result;
}

int NvApiInit()
{
	int result = -1;

	if (NvAPI_Initialize)
	{
		LOG(result = NvAPI_Initialize());
	}

	return result;
}

int NvApiFree()
{
	int result = -1;

	if (NvAPI_Unload)
	{
		LOG(result = NvAPI_Unload());
	}

	return result;
}

int NvApiRestartDriver()
{
	int result = -1;

	if (NvAPI_RestartDisplayDriver)
	{
		LOG(result = NvAPI_RestartDisplayDriver());
	}

	return result;
}
unsigned int busId = 0;

int NvApiEnumGpus()
{
	int result = -1;

	if ((NvAPI_EnumPhysicalGPUs) && (NvAPI_GPU_GetBusId))
	{
		void* handles[64] = { 0 };
		unsigned int count = 0;

		LOG(result = NvAPI_EnumPhysicalGPUs(handles, &count));

		if (result == 0)
		{
			for (unsigned int i = 0; i < count; ++i)
			{
				//unsigned int busId = 0;
				int busIdResult = 0;

				LOG(busIdResult = NvAPI_GPU_GetBusId(handles[i], &busId));
				if (busIdResult == 0)
				{
					LOG("init GpuGetBusId:%d\n", busId);
					gpuBusId = busId;
					NvApiGpuHandles[busId] = handles[i];
				}
				else
					{
						LOG("init GpuGetBusId :%d not find Gpu ,GetBusId is wrong\n",i);
					}

			}
		}
	}

	return result;
}

int NvApiEnumTccGpus()
{
	int result = -1;

	if ((NvAPI_EnumTCCPhysicalGPUs) && (NvAPI_GPU_GetBusId))
	{
		void* handles[64] = { 0 };
		unsigned int count = 0;

		(result = NvAPI_EnumTCCPhysicalGPUs(handles, &count));

		if (result == 0)
		{
			for (unsigned int i = 0; i < count; ++i)
			{
				unsigned int busId = 0;
				int busIdResult = 0;

				(busIdResult = NvAPI_GPU_GetBusId(handles[i], &busId));

				if (busIdResult == 0)
					NvApiGpuHandles[busId] = handles[i];
			}
		}
	}

	return result;
}
int NvApiGetCurve(unsigned int gpuBusId, unsigned int* count, unsigned int* voltageUV, int* frequencyDeltaKHz, unsigned int checkCount=128)
{
	int result = -1;

	if ((NvAPI_GPU_GetClockBoostMask) && (NvAPI_GPU_GetVFPCurve) && (NvAPI_GPU_GetClockBoostTable))
	{
		NV_GPU_CLOCK_MASKS clockMasks = { 0 };

		clockMasks.version = MAKE_NVAPI_VERSION(clockMasks, 1);

		(result = NvAPI_GPU_GetClockBoostMask(NvApiGpuHandles[gpuBusId], &clockMasks));

		if (result == 0)
		{
			NV_GPU_VFP_CURVE vfpCurve = { 0 };

			vfpCurve.version = MAKE_NVAPI_VERSION(vfpCurve, 1);
			memcpy(vfpCurve.mask, clockMasks.mask, sizeof(vfpCurve.mask));

			(result = NvAPI_GPU_GetVFPCurve(NvApiGpuHandles[gpuBusId], &vfpCurve));

			if (result == 0)
			{
				NV_GPU_CLOCK_TABLE clockTable = { 0 };

				clockTable.version = MAKE_NVAPI_VERSION(clockTable, 1);
				memcpy(clockTable.mask, clockMasks.mask, sizeof(clockTable.mask));

				(result = NvAPI_GPU_GetClockBoostTable(NvApiGpuHandles[gpuBusId], &clockTable));

				if (result == 0)
				{
					if (count)
						*count = 0;

					if (checkCount != 128)
						for (unsigned int i = 0; i < checkCount; ++i)
						{
							if ((clockMasks.clocks[i].enabled == 1) && (vfpCurve.clocks[i].clockType == 0))
							{
								if (count)
								{
									if (voltageUV)
										voltageUV[*count] = vfpCurve.clocks[i].voltageUV;

									if (frequencyDeltaKHz)
										frequencyDeltaKHz[*count] = clockTable.clocks[i].frequencyDeltaKHz / 2;

									++(*count);
								}
							}
						}
					else
						for (unsigned int i = 0; i < 255; ++i)
						{
							if ((clockMasks.clocks[i].enabled == 1) && (vfpCurve.clocks[i].clockType == 0))
							{
								if (count)
								{
									if (voltageUV)
										voltageUV[*count] = vfpCurve.clocks[i].voltageUV;

									if (frequencyDeltaKHz)
										frequencyDeltaKHz[*count] = clockTable.clocks[i].frequencyDeltaKHz / 2;

									++(*count);
								}
							}
						}
				}
			}
		}
	}

	return result;
}
int NvApiSetCurve(unsigned int gpuBusId, unsigned int count, unsigned int* voltageUV, int* frequencyDeltaKHz)
{
	int result = -1;

	if ((NvAPI_GPU_GetClockBoostMask) && (NvAPI_GPU_GetVFPCurve) && (NvAPI_GPU_GetClockBoostTable) && (NvAPI_GPU_SetClockBoostTable))
	{
		NV_GPU_CLOCK_MASKS clockMasks = { 0 };

		clockMasks.version = MAKE_NVAPI_VERSION(clockMasks, 1);

		(result = NvAPI_GPU_GetClockBoostMask(NvApiGpuHandles[gpuBusId], &clockMasks));

		if (result == 0)
		{
			NV_GPU_CLOCK_TABLE clockTable = { 0 };

			clockTable.version = MAKE_NVAPI_VERSION(clockTable, 1);
			memcpy(clockTable.mask, clockMasks.mask, sizeof(clockTable.mask));

			if (count > 0)
			{
				NV_GPU_VFP_CURVE vfpCurve = { 0 };

				vfpCurve.version = MAKE_NVAPI_VERSION(vfpCurve, 1);
				memcpy(vfpCurve.mask, clockMasks.mask, sizeof(vfpCurve.mask));

				(result = NvAPI_GPU_GetVFPCurve(NvApiGpuHandles[gpuBusId], &vfpCurve));

				if (result == 0)
				{
					(result = NvAPI_GPU_GetClockBoostTable(NvApiGpuHandles[gpuBusId], &clockTable));

					if (result == 0)
					{
						for (unsigned int i = 0; i < 255; ++i)
						{
							if ((clockMasks.clocks[i].enabled == 1) && (vfpCurve.clocks[i].clockType == 0))
							{
								if (voltageUV)
								{
									for (unsigned int j = 0; j < count; ++j)
									{
										if (voltageUV[j] == vfpCurve.clocks[i].voltageUV)
										{
											if (frequencyDeltaKHz)
												clockTable.clocks[i].frequencyDeltaKHz = frequencyDeltaKHz[j] * 2;

											break;
										}
									}
								}
							}
						}

						(result = NvAPI_GPU_SetClockBoostTable(NvApiGpuHandles[gpuBusId], &clockTable));
					}
				}
			}
			else
			{
				(result = NvAPI_GPU_SetClockBoostTable(NvApiGpuHandles[gpuBusId], &clockTable));
			}
		}
	}

	return result;
}

CGPUInfo::~CGPUInfo()
{
	if (m_hGPUdll != NULL)
	{
		LockFrequency();//还原GPU频率设置
		m_pfnCloseGPU_API();
		FreeLibrary(m_hGPUdll);
		m_hGPUdll = NULL;
		if (NvApi)
			NvApiFree();
			//FreeLibrary(nvapi);

		if (LogFile)
			fclose(LogFile);
	}
}



BOOL CGPUInfo::Init()
{
	//add nvapioc
	//#define BUF_SIZE 260	
	//char buffer[BUF_SIZE];
	//char* p;

	//p = _getcwd(buffer, BUF_SIZE);
	//if (p == NULL)
	//{
	//	printf("The directory isError");
	//}

	//LOG("The RunDir: %s\n", buffer);
	unsigned int voltageUV[255] = { 0 };
	int frequencyDeltaKHz[255] = { 0 };

	CString dirPath = GetExePath();
	CString logPath = dirPath + "\\log.txt";


	//int ch = _chdir(dirPath); //change working directory
	if (LogFileEnable)
		//LogFile = fopen(logPath, "a");
		LogFile = fopen(logPath, "w");

	nv_Api_init = 0; //初始化check 128
	if (NvApi)
	{
		NvApi = false;

		if (NvApiLoad() == 0)
		{
			if (NvApiInit() == 0)
			{
				NvApiEnumGpus();
				NvApiEnumTccGpus();

				NvApi = true;
				LOG("NvApiInit OK");

				//nv_Api_init = 1; //init OK

			}
			else
			{
				LOG("NvApiInit fail");
				nv_Api_init = 2; //init fail

			}
		}
	}

	//unsigned int gpuBusId = atoi(argv[++arg]);
	//unsigned int gpuBusId = 1;
	int ret = -1;
	//int count = atoi(argv[++arg]);
	int count = -1;



	//save curve
	if (NvApiGpuHandles[gpuBusId] != 0)
	{
		ret = NvApiGetCurve(gpuBusId, (unsigned int*)&count, voltageUV, frequencyDeltaKHz);

		if (ret == 0)
		{
			//#define BUF_SIZE 260	
			//char buffer[BUF_SIZE];
			//char* p;

			//p = _getcwd(buffer, BUF_SIZE);
			//if (p == NULL)
			//{
			//	printf("The directory isError");
			//}

			//LOG("The RunDir: %s\n", buffer);

			FILE* curve = 0;
			CString curvePath = dirPath + "\\curve.bat";

			LOG(curve = fopen(curvePath, "w"));

			if (curve)
			{
				//fprintf(curve, "cd \"%s\"\n", buffer);
				fprintf(curve, "\"nvapioc.exe\" -curve %d %d", gpuBusId, count);

				for (int i = 0; i < count; ++i)
					fprintf(curve, " %d %d", voltageUV[i], frequencyDeltaKHz[i]);

				fprintf(curve, "\n");
				//fprintf(curve, "ping -n 2 127.0.0.1\n");
				LOG(fclose(curve));
				//LOG("fclose:%d\n", ret);
			}
			else
				LOG("NvApi init no FILE curve \n");
		}
		else
			LOG("NvApi Run fail:%d\n", ret);
	}
	else
	{
		LOG("CURVE not find gpuBusId\n");
	}



	TRACE0("开始加载NVGPU_DLL.dll。\n");
	m_hGPUdll = NULL;
	CString dllpth = GetExePath() + "\\NVGPU_DLL.dll";
	m_hGPUdll = LoadLibrary(dllpth);
	if (m_hGPUdll == NULL)
	{
		TRACE0("无法加载" + dllpth + "\n");
		LOG("m_hGPUdll isNULL");
		return FALSE;
	}

	m_pfnInitGPU_API = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "InitGPU_API");
	m_pfnSet_GPU_Number = (In_1_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Set_GPU_Number");
	m_pfnGet_GPU_Base_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Base_Clock");

	//add new
	//m_pfnGet_GPU_Pstate = (In_0_Out_n_Func *)::GetProcAddress(m_hGPUdll, "Get_GPU_Pstate");
	//m_pfnGet_GPU_F = (In_0_Out_n_Func *)::GetProcAddress(m_hGPUdll, "Get_GPU_F");
	//m_pfnGet_GPU_Pstate_Freq = (In_0_Out_n_Func *)::GetProcAddress(m_hGPUdll, "Get_GPU_Pstate_Freq");
	//m_pfnGet_GPU_Volt = (In_0_Out_n_Func *)::GetProcAddress(m_hGPUdll, "Get_GPU_Volt");
	//m_pfnCheckGPU_Thermal = (In_0_Out_n_Func *)::GetProcAddress(m_hGPUdll, "CheckGPU_Thermal");
	m_pfnGet_GPU_ALLInfo = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_defaultMaxTemp");
	//m_pfnGet_GPU_TotalNumber = (In_0_Out_n_Func *)::GetProcAddress(m_hGPUdll, "Get_GPU_TotalNumber");
	m_pfnGet_GPU_Temp = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Temp");
	//

	m_pfnGet_GPU_Boost_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Boost_Clock");
	m_pfnCheck_GPU_VRAM_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Check_GPU_VRAM_Clock");
	m_pfnGet_GPU_Graphics_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Graphics_Clock");
	m_pfnGet_GPU_Memory_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Memory_Clock");
	m_pfnGet_Memory_OC_max = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_Memory_OC_max");
	m_pfnGet_GPU_Util = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Util");
	m_pfnGet_GPU_name = (In_0_Out_s_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_name");
	m_pfnGet_GPU_TotalNumber = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_TotalNumber");
	m_pfnGet_GPU_Overclock_range = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Overclock_range");
	m_pfnGet_Memory_range = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_Memory_range");
	m_pfnGet_GPU_Overclock_rangeMax = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Overclock_rangeMax");
	m_pfnGet_GPU_Overclock_rangeMin = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Overclock_rangeMin");
	m_pfnGet_Memory_range_max = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_Memory_range_max");
	m_pfnGet_Memory_range_min = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_Memory_range_min");
	m_pfnGet_NVDeviceID = (In_1_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_NVDeviceID");
	m_pfnLock_Frequency = (In_2_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Lock_Frequency");
	m_pfnLock_Frequency_MEM = (In_2_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Lock_Frequency_MEM");
	m_pfnSet_CoreOC = (In_2_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Set_CoreOC");
	m_pfnSet_MEMOC = (In_2_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Set_MEMOC");
	m_pfnCloseGPU_API = (In_0_Out_0_Func*)::GetProcAddress(m_hGPUdll, "CloseGPU_API");
	if (m_pfnInitGPU_API())
	{
		TRACE0("InitGPU_API初始化失败。\n");
		LOG("m_hGPUdll InitGPU_API初始化失败。\n");
		FreeLibrary(m_hGPUdll);
		//LOG("m_hGPUdll InitGPU_API初始化失败");
		m_hGPUdll = NULL;
		return FALSE;
	}
	m_pfnSet_GPU_Number(0);
	m_nBaseClock = m_pfnGet_GPU_Base_Clock();
	m_nGet_GPU_TotalNumber = m_pfnGet_GPU_TotalNumber();
	//add new 

	//m_nGet_GPU_Pstate = m_pfnGet_GPU_F();
	//m_nGet_GPU_F = m_pfnGet_GPU_Pstate_Freq();
	//m_nGet_GPU_Pstate_Freq = m_pfnGet_GPU_Pstate_Freq();
	//m_nGet_GPU_Volt = m_pfnGet_GPU_Volt();
	//m_nCheckGPU_Thermal = m_pfnCheckGPU_Thermal();

	//m_nGPU_Temp = m_pfnGet_GPU_Temp();
	//m_nGPU_Util = m_pfnGet_GPU_Util();
	m_nGPU_UtilCount = 0;
	//m_nGet_GPU_ALLInfo = m_pfnGet_GPU_ALLInfo();
	m_nGraphicsClock = m_pfnGet_GPU_Graphics_Clock();
	m_nMemoryClock = m_pfnGet_GPU_Memory_Clock();
	

	m_nBoostClock = m_pfnGet_GPU_Boost_Clock();
	m_sName = m_pfnGet_GPU_name();
	m_nDeviceID = m_pfnGet_NVDeviceID(0);
	m_nGraphicsRangeMax = m_pfnGet_GPU_Overclock_rangeMax();
	m_nGraphicsRangeMin = m_pfnGet_GPU_Overclock_rangeMin();
	m_nMemoryRangeMax = m_pfnGet_Memory_range_max();
	m_nMemoryRangeMin = m_pfnGet_Memory_range_min();

	m_nStandardFrequency = m_nBoostClock - m_nGraphicsRangeMin;
	m_nMaxFrequency = m_nStandardFrequency + m_nGraphicsRangeMax;
	m_nLockClock = -1;
	//m_nOverClock = 0;
	ForcedRefreshGPU = 0;
	
	return TRUE;
}
void CGPUInfo::ReloadAPI(int forceinit)
{
	//if (m_nGPU_Temp ^ 0 && m_nGraphicsClock ^ 0 && m_nMemoryClock ^ 0)
	if (m_nGPU_Temp == 0 && m_nGraphicsClock == 0 && m_nMemoryClock == 0)
	{
		TRACE0("检测数据为0,重新加载NVGPU_DLL.dll。\n");
		LOG("检测数据为0,重新加载NVGPU_DLL.dll。\n");
		Init();
	}
	else {
		if (m_hGPUdll != NULL)
		{
			//TRACE0("无法加载" + dllpth + "\n");
			LOG("m_hGPUdll isNotNULL Free and then reload Init");
			nv_Api_init = 0;
			m_pfnCloseGPU_API();
			FreeLibrary(m_hGPUdll);
			m_hGPUdll = NULL;
			if (NvApi)
				NvApiFree();
			//FreeLibrary(nvapi);
			if (LogFile)
				fclose(LogFile);
			Init();
		}
		else{
			if (m_pfnInitGPU_API() || forceinit == 0)
			{
				TRACE0("InitGPU_API失败。\n");
				LOG("m_hGPUdll InitGPU_API失败。\n");
				m_pfnCloseGPU_API();
				FreeLibrary(m_hGPUdll);
				m_hGPUdll = NULL;
				if (NvApi)
					NvApiFree();
				//FreeLibrary(nvapi);

				if (LogFile)
					fclose(LogFile);

				Init();
				LOG("m_pfnInitGPU_API NULL, reload");
			}
			else {
				LOG("m_hGPUdll and GPU_API Not NULL, no reload");
			}
		}
	}
	//return m_Gpu;
}

BOOL CGPUInfo::Update()
{
	//if (m_hGPUdll == NULL)
	//	LOG("m_hGPUdll isNULL,Need Reload");
	//	return FALSE;
	if (nv_Api_init == 1)
	{
		if (!m_pfnCheck_GPU_VRAM_Clock())
			return FALSE;
		m_pfnSet_GPU_Number(0);
		m_nGPU_Temp = m_pfnGet_GPU_Temp();
		m_nGraphicsClock = m_pfnGet_GPU_Graphics_Clock();
		m_nMemoryClock = m_pfnGet_GPU_Memory_Clock();
		m_nGPU_Util = m_pfnGet_GPU_Util();
	}
	else
		{
			return FALSE;
		}
	return TRUE;
}
BOOL CGPUInfo::LockFrequency(int frequency)
{
	if (!m_hGPUdll)
		return FALSE;
	if (frequency == 0)
		frequency = m_nStandardFrequency;

	//if (frequency < 0 || frequency > m_nMaxFrequency)
	//if (frequency < m_nBaseClock || frequency > m_nMaxFrequency)
	if (frequency < 400 || frequency > m_nMaxFrequency)
		return FALSE;

	if (m_nLockClock == frequency)
		if (ForcedRefreshGPU == 0)
			return TRUE;
		else
			ForcedRefreshGPU = 0;
	m_nLockClock = frequency;
	int GpuOverclock = 0;
	int MemOverclock = 0;
	int GpuClock = 0;

	//int MemClock = 0;
	if (frequency >= 400 && frequency < m_nStandardFrequency)
	//if (frequency >= m_nBaseClock && frequency < m_nStandardFrequency)
	{
		//降频
		GpuClock = frequency;
	}

	//else if (frequency > m_nStandardFrequency)
	//else if (frequency)
	//{
	//	//超频
	//	GpuOverclock = frequency - m_nStandardFrequency;
	//	MemOverclock = GpuOverclock * m_nMemoryRangeMax / m_nGraphicsRangeMax;//按照比例进行显存超频
	//}
	

	//
	//int rv1 = (m_pfnSet_CoreOC(0, GpuOverclock) == 0);
	//if (!rv1)
	//	AfxMessageBox("Set_CoreOC失败");
	int rv1 = 1;
	int rv2 = 1;

	int rv4 = 1;
	//
	//int rv2 = (m_pfnSet_MEMOC(0, MemOverclock) == 0);
	//if (!rv2)
	//	AfxMessageBox("Set_MEMOC失败");
	////
	int rv3 = (m_pfnLock_Frequency(0, GpuClock) == 0x19);

	//if (!rv3)
	//{
	//	CString str;
	//	str.Format("Lock_Frequency失败: %d", GpuClock);
	//	AfxMessageBox(str);
	//	//AfxMessageBox("Lock_Frequency失败:" + GpuClock);
	//}

	////
	//int rv4 = 1;
	////int rv4 = (m_pfnLock_Frequency_MEM(0, MemClock) == 0x19);
	//if (!rv4)
	//	AfxMessageBox("Lock_Frequency_MEM失败");
	//
	return (rv1 && rv2 && rv3 && rv4);
}

BOOL CGPUInfo::OverClockFrequency(int frequency, int memOverClock, int limitUV,int overClock2)
{
	if (!m_hGPUdll)
	{
		LOG("OverClockFrequency m_hGPUdll isNULL");
		return FALSE;
	}

	int overClock = m_nStandardFrequency + frequency;
	if (frequency < 0 || overClock > m_nMaxFrequency)
		return FALSE;
	if (frequency == 0)
		frequency = m_nStandardFrequency;
	if (ForcedRefreshGPU == 0 && m_nOverLockClock == overClock)
	{
		return TRUE;
	}
	else
	{
		ForcedRefreshGPU = 0;
	}

	m_nOverLockClock = overClock;
	int GpuOverclock = 0;
	int MemOverclock = 0;
	int GpuClock = 0;
	//int OverClock = m_nOverClock;
	//int MemClock = 0;
	//if (overClock > 0 && overClock < m_nStandardFrequency)
	//{
	//	//降频
	//	GpuClock = frequency;
	//}
	//else if (overClock > m_nStandardFrequency)
	if (overClock > m_nStandardFrequency)
	{
		//超频
		GpuOverclock = frequency;
		//MemOverclock = GpuOverclock * m_nMemoryRangeMax / m_nGraphicsRangeMax;//按照比例进行显存超频
		//MemOverclock = int (memOverClock /4); //更新MemOverclock频率
		MemOverclock = memOverClock; //更新MemOverclock频率

	}
	

	//
	int rv1 = (m_pfnSet_CoreOC(0, GpuOverclock) == 0);
	if (!rv1)
	{
		CString str;
		str.Format("Set_CoreOC失败: %d", GpuOverclock);
		AfxMessageBox(str);
		//AfxMessageBox("Set_CoreOC失败:"+ GpuOverclock);
	}
	//
	//int rv2 = 1;

	int rv2 = (m_pfnSet_MEMOC(0, MemOverclock) == 0);
	if (!rv2)
	{
		CString str;
		str.Format("Set_MEMOC失败: %d", MemOverclock);
		AfxMessageBox(str);
		//AfxMessageBox("Set_MEMOC失败:"+ MemOverclock);
	}
	

	//int rv3 = (m_pfnLock_Frequency(0, GpuClock) == 0x19);
	//if (!rv3)
	//	AfxMessageBox("Lock_Frequency失败");
	////
	int rv3 = 1;
	int rv4 = 1;
	//int rv4 = (m_pfnLock_Frequency_MEM(0, MemClock) == 0x19);
	//if (!rv4)
	//	AfxMessageBox("Lock_Frequency_MEM失败");
	//

	//
	if (frequency > overClock2 && overClock2 > 0)
	{
		int ret = -1;
		Sleep(1000);
		int count1 = -1;
		unsigned int voltageUV[255] = { 0 };
		int frequencyDeltaKHz[255] = { 0 };
		ret = NvApiGetCurve(gpuBusId, (unsigned int*)&count1, voltageUV, frequencyDeltaKHz);
		LOG("frequencyDeltaKHz reload:%d", frequencyDeltaKHz[0] / 500);

		//NvApiSetCurve
		unsigned int voltageUV_OV[255] = { 0 };
		int frequencyDeltaKHz_OV[255] = { 0 };
		int count = sizeof(voltageUV) / sizeof(voltageUV[0]);
		int j = 0;
		unsigned int limitUv = limitUV * 1000;

		int frequency_DeltaKHz_Value = overClock2 * 500;
		//int frequency_DeltaKHz_Value = (frequency - 20) * 500;
		//if (overClock2 > 0 && frequency_DeltaKHz_Value > overClock2 * 500)
		//if (overClock2 > 0)
		//{
		//	frequency_DeltaKHz_Value = overClock2 * 500;
		//}

		for (int i = 0; i < count; ++i)
		{

			if (frequencyDeltaKHz[i] > 0)
			{
				//if (voltageUV[i] > limitUv)
				//{
				//	voltageUV_OV[j] = voltageUV[i];
				//	//frequencyDeltaKHz_OV[j] = frequencyDeltaKHz[i];
				//	frequencyDeltaKHz_OV[j] = frequency_DeltaKHz_Value;
				//	++j;
				//}  //仅更新limit曲线

				voltageUV_OV[i] = voltageUV[i];
				if (voltageUV[i] > limitUv)
				{
					//frequencyDeltaKHz_OV[j] = frequencyDeltaKHz[i];
					frequencyDeltaKHz_OV[i] = frequency_DeltaKHz_Value;
				}
				else
				{
					frequencyDeltaKHz_OV[i] = frequencyDeltaKHz[i];
				}
				++j;

			}
		}

		count = j;
		if (NvApiGpuHandles[gpuBusId] != 0)
			ret = NvApiSetCurve(gpuBusId, count, voltageUV_OV, frequencyDeltaKHz_OV);
		if (ret == 0)
		{
			//CString str;
			//str.Format("%dmv Curve OK:%d", limitUV,frequencyDeltaKHz_OV[0]/500);
			//AfxMessageBox(str);
			LOG("NvApi SetCurve OK");
		}
		else
		{
			LOG("NvApi SetCurve Fail", ret);
			//LOG(sizeof(frequencyDeltaKHz_OV));
			CString str;
			str.Format("%dmv SetCurveFail: %d", limitUV, frequencyDeltaKHz_OV[0] / 500);
			AfxMessageBox(str);
		}
	}

	return (rv1 && rv2 && rv3 && rv4);
}

CConfig::CConfig()
{
	LoadDefault();
}
void CConfig::LoadDefault()
{

		//add temp_limit
	//int upClockPercent;//占用率升频阈值
	//int downClockPercent;//占用率降频阈值
	//int downTemplimit;//温控降频阈值
	//int upTemplimit;//温控升频阈值
	CString dirPath = GetExePath();
	CString inipath = dirPath + "\\MyFanconfig.ini";
	//int battery_percent = GetBatteryLevel();
	//int battery_ACLine = GetBatteryACLineStatus();
	//int dm = GetDisplayFrequency();
	if (FileExists(inipath))
	{
		char returnValue[100];
		//char returnValue[1024] = { 0 }
		GetPrivateProfileString("LoadDefault", "upClockPercent", 0, returnValue, 100, inipath);
		upClockPercent = atoi(returnValue);
		GetPrivateProfileString("LoadDefault", "downClockPercent", 0, returnValue, 100, inipath);
		downClockPercent = atoi(returnValue);
		GetPrivateProfileString("LoadDefault", "downTemplimit", 0, returnValue, 100, inipath);
		downTemplimit = atoi(returnValue);
		GetPrivateProfileString("LoadDefault", "upTemplimit", 0, returnValue, 100, inipath);
		upTemplimit = atoi(returnValue);
		GetPrivateProfileString("LoadDefault", "upClocklimit", 0, returnValue, 100, inipath);
		upClocklimit = atoi(returnValue);
		GetPrivateProfileString("LoadDefault", "timelimit", 0, returnValue, 100, inipath);
		timelimit = atoi(returnValue);
		GetPrivateProfileString("LoadDefault", "UpdateInterval", "5", returnValue, 100, inipath);
		UpdateInterval = atoi(returnValue);
		GetPrivateProfileString("LoadDefault", "CurveUV_limit", 0, returnValue, 100, inipath);
		CurveUV_limit = atoi(returnValue);
		GetPrivateProfileString("LoadDefault", "OverClock2", 0, returnValue, 100, inipath);
		OverClock2 = atoi(returnValue);
	}
	else
	{
		upClockPercent = 94;//占用率升频阈值
		downClockPercent = 88;//占用率降频阈值
		downTemplimit = 80;//温控降频阈值
		upTemplimit = 79;//温控升频阈值
		upClocklimit = 1600; //升频上限
		timelimit = 3;    //统计时长
		CurveUV_limit = 690; //Curve保护
		OverClock2 = 145; //Curve保护145Mhz
		UpdateInterval = 5;
	}

	int i = 0;
	DutyList[0][i++] = upClockPercent;//90+  //占用率升频阈值
	DutyList[0][i++] = downClockPercent;//85+	//降频阈值
	DutyList[0][i++] = downTemplimit;//80+	//温控降频阈值
	DutyList[0][i++] = upTemplimit;//75+	//温控升频阈值
	DutyList[0][i++] = upClocklimit;//70+
	DutyList[0][i++] = timelimit;//65+
	DutyList[0][i++] = CurveUV_limit;//780mv
	DutyList[0][i++] = OverClock2;//145Mhz
	//DutyList[0][i++] = 0;//60+
	//DutyList[0][i++] = 0;//55+
	//DutyList[0][i++] = 0;//50+
	//DutyList[0][i++] = 0;//50-
	//for (int i = 0; i < 6; i++)
	//	DutyList[1][i] = DutyList[0][i];



	TransitionTemp = 0;
	//UpdateInterval = 2;
	Linear = FALSE;
	TakeOver = FALSE;
	ForceTemp = 50;
	LockGPUFrequency = FALSE;
	GPUFrequency = 0;
	GPUOverClock = 50;
	GPUOverMEMClock = 0;
	TakeOverDown = 0;
	TakeOverUp = 0;
	TakeOverLock = 0;
	GPU_LockClock = 0; //init Lock Clock
	
	// GPU动态频率控制参数默认值
	baseClockLimit = 600;
	lowClockLimit = 420;
	upClockRatio = 105;
	downClockRatio = 97;
	aggressiveUpRatio = 110;
	utilIdleThreshold = 55;
	tempCoolThreshold = 65;
	
	//m_sCurrentProfile = "";
	m_sCurrentProfile.Empty();


}


///////////////////////////////////////////////////////////////
// LoadConfig —— 新版（INI格式，安全，不再读取二进制）
///////////////////////////////////////////////////////////////
void CConfig::LoadConfig()
{
	CString dirPath = GetExePath();
	CString inipath = dirPath + "\\MyFanControl.cfg";

	// INI不存在 -> 直接走默认初始化
	if (!FileExists(inipath))
	{
		LoadDefault();
		SaveConfig(); // 顺便写出一份INI
		return;
	}

	char buf[128] = { 0 };

	auto ReadInt = [&](LPCSTR section, LPCSTR key, int def)->int
	{
		GetPrivateProfileStringA(section, key, "", buf, sizeof(buf), inipath);
		if (buf[0] == 0) return def;
		return atoi(buf);
	};

	auto ReadBool = [&](LPCSTR section, LPCSTR key, BOOL def)->BOOL
	{
		return ReadInt(section, key, def ? 1 : 0) ? TRUE : FALSE;
	};

	/////////////////////////
	// 基本参数
	/////////////////////////
	UpdateInterval = ReadInt("Config", "UpdateInterval", 5);
	TransitionTemp = ReadInt("Config", "TransitionTemp", 0);
	ForceTemp = ReadInt("Config", "ForceTemp", 50);

	Linear = ReadBool("Config", "Linear", FALSE);
	TakeOver = ReadBool("Config", "TakeOver", FALSE);

	/////////////////////////
	// GPU
	/////////////////////////
	LockGPUFrequency = ReadBool("GPU", "LockGPUFrequency", FALSE);
	GPUFrequency = ReadInt("GPU", "GPUFrequency", 0);
	// GPU_LockClock 用于存储初始锁定频率,从GPUFrequency初始化
	GPU_LockClock = GPUFrequency;
	GPUOverClock = ReadInt("GPU", "GPUOverClock", 50);
	GPUOverMEMClock = ReadInt("GPU", "GPUOverMEMClock", 0);

	/////////////////////////
	// 动态计数
	/////////////////////////
	TakeOverDown = ReadInt("Dynamic", "TakeOverDown", 0);
	TakeOverUp = ReadInt("Dynamic", "TakeOverUp", 0);
	TakeOverLock = ReadInt("Dynamic", "TakeOverLock", 0);

	/////////////////////////
	// 频率温控
	/////////////////////////
	upClockPercent = ReadInt("Limit", "upClockPercent", 94);
	downClockPercent = ReadInt("Limit", "downClockPercent", 88);
	downTemplimit = ReadInt("Limit", "downTemplimit", 80);
	upTemplimit = ReadInt("Limit", "upTemplimit", 79);
	upClocklimit = ReadInt("Limit", "upClocklimit", 1600);
	timelimit = ReadInt("Limit", "timelimit", 3);
	CurveUV_limit = ReadInt("Limit", "CurveUV_limit", 690);
	OverClock2 = ReadInt("Limit", "OverClock2", 145);

	/////////////////////////
	// GPU动态频率控制
	/////////////////////////
	baseClockLimit = ReadInt("FreqControl", "baseClockLimit", 600);
	lowClockLimit = ReadInt("FreqControl", "lowClockLimit", 420);
	upClockRatio = ReadInt("FreqControl", "upClockRatio", 105);
	downClockRatio = ReadInt("FreqControl", "downClockRatio", 97);
	aggressiveUpRatio = ReadInt("FreqControl", "aggressiveUpRatio", 110);
	utilIdleThreshold = ReadInt("FreqControl", "utilIdleThreshold", 55);
	tempCoolThreshold = ReadInt("FreqControl", "tempCoolThreshold", 65);

	/////////////////////////
	// DutyList
	/////////////////////////
	memset(DutyList, 0, sizeof(DutyList));

	int i = 0;
	DutyList[0][i++] = upClockPercent;
	DutyList[0][i++] = downClockPercent;
	DutyList[0][i++] = downTemplimit;
	DutyList[0][i++] = upTemplimit;
	DutyList[0][i++] = upClocklimit;
	DutyList[0][i++] = timelimit;
	DutyList[0][i++] = CurveUV_limit;
	DutyList[0][i++] = OverClock2;

	/////////////////////////
	// Profile
	/////////////////////////
	GetPrivateProfileStringA("Profile", "CurrentProfile", "",
		buf, sizeof(buf), inipath);
	m_sCurrentProfile = buf;
}


///////////////////////////////////////////////////////////////
// SaveConfig —— 新版（INI写盘）
///////////////////////////////////////////////////////////////
void CConfig::SaveConfig()
{
	CString dirPath = GetExePath();
	CString inipath = dirPath + "\\MyFanControl.cfg";

	auto WriteInt = [&](LPCSTR sec, LPCSTR key, int v)
	{
		char buf[32];
		_itoa_s(v, buf, 10);
		WritePrivateProfileStringA(sec, key, buf, inipath);
	};

	auto WriteBool = [&](LPCSTR sec, LPCSTR key, BOOL v)
	{
		WriteInt(sec, key, v ? 1 : 0);
	};

	/////////////////////////
	// 基本参数
	/////////////////////////
	WriteInt("Config", "UpdateInterval", UpdateInterval);
	WriteInt("Config", "TransitionTemp", TransitionTemp);
	WriteInt("Config", "ForceTemp", ForceTemp);

	WriteBool("Config", "Linear", Linear);
	WriteBool("Config", "TakeOver", TakeOver);

	/////////////////////////
	// GPU
	/////////////////////////
	WriteBool("GPU", "LockGPUFrequency", LockGPUFrequency);
	WriteInt("GPU", "GPUFrequency", GPUFrequency);
	// GPU_LockClock 不需要单独保存,它从GPUFrequency派生
	WriteInt("GPU", "GPUOverClock", GPUOverClock);
	WriteInt("GPU", "GPUOverMEMClock", GPUOverMEMClock);

	/////////////////////////
	// 动态计数
	/////////////////////////
	WriteInt("Dynamic", "TakeOverDown", TakeOverDown);
	WriteInt("Dynamic", "TakeOverUp", TakeOverUp);
	WriteInt("Dynamic", "TakeOverLock", TakeOverLock);

	/////////////////////////
	// 温控频率
	/////////////////////////
	WriteInt("Limit", "upClockPercent", upClockPercent);
	WriteInt("Limit", "downClockPercent", downClockPercent);
	WriteInt("Limit", "downTemplimit", downTemplimit);
	WriteInt("Limit", "upTemplimit", upTemplimit);
	WriteInt("Limit", "upClocklimit", upClocklimit);
	WriteInt("Limit", "timelimit", timelimit);
	WriteInt("Limit", "CurveUV_limit", CurveUV_limit);
	WriteInt("Limit", "OverClock2", OverClock2);

	/////////////////////////
	// GPU动态频率控制
	/////////////////////////
	WriteInt("FreqControl", "baseClockLimit", baseClockLimit);
	WriteInt("FreqControl", "lowClockLimit", lowClockLimit);
	WriteInt("FreqControl", "upClockRatio", upClockRatio);
	WriteInt("FreqControl", "downClockRatio", downClockRatio);
	WriteInt("FreqControl", "aggressiveUpRatio", aggressiveUpRatio);
	WriteInt("FreqControl", "utilIdleThreshold", utilIdleThreshold);
	WriteInt("FreqControl", "tempCoolThreshold", tempCoolThreshold);

	/////////////////////////
	// Profile
	/////////////////////////
	WritePrivateProfileString("Profile", "CurrentProfile",
		m_sCurrentProfile, inipath);
}


//void CConfig::LoadConfig()
//{
//	CString strPath = GetExePath() + "\\MyFanControl.cfg";
//	CFile file;
//	if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
//	{
//		SaveConfig();
//		if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
//		{
//			AfxMessageBox("无法载入配置文件");
//			return;
//		}
//	}
//	if (file.GetLength() != sizeof(*this))
//	{
//		file.Close();
//		SaveConfig();
//		if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
//		{
//			AfxMessageBox("重置后仍然无法载入配置文件");
//			return;
//		}
//		if (file.GetLength() != sizeof(*this))
//		{
//			AfxMessageBox("配置文件格式不正确");
//			file.Close();
//			return;
//		}
//	}
//	file.Read(this, sizeof(*this));
//	file.Close();
//}
//void CConfig::SaveConfig()
//{
//	FILE *fp = NULL;
//	CString strPath = GetExePath() + "\\MyFanControl.cfg";
//	fp = fopen(strPath, "wb");
//	if (fp == NULL)
//	{
//		AfxMessageBox("无法保存配置文件");
//		return;
//	}
//	fwrite(this, sizeof(*this), 1, fp);
//	fclose(fp);
//}

//////////////////////////////////////////////////

CCore::CCore()
{
	m_pfnInitIo = NULL;
	m_pfnSetFanDuty = NULL;
	m_pfnSetFANDutyAuto = NULL;
	m_pfnGetTempFanDuty = NULL;
	m_pfnGetFANCounter = NULL;
	m_pfnGetECVersion = NULL;
	m_pfnGetFANRPM[0] = NULL;
	m_pfnGetFANRPM[1] = NULL;

	//
	m_nInit = 0;
	m_nExit = 0;
	m_start_overclock = 0;
	m_hInstDLL = NULL;
	m_ApmPowerStatusChange = 0;

	for (int i = 0; i < 2; i++)
	{
		m_nCurTemp[i]=0;//当前温度
		m_nLastTemp[i]=0;//上一次温度
		m_nSetDuty[i]=0;//设置的负载
		m_nSetDutyLevel[i] = 0;//设置的转速挡位，最低速档为1，最高速档为10
		m_nCurDuty[i]=0;//当前负载
		m_nCurRPM[i]=0;//当前转速
	}
	m_bUpdateRPM=0;//是否更新转速，如果为0，只更新风扇温度和负载
	m_nLastUpdateTime = GetTime(0, -5);
	m_TakeOverTimeOut = 1;
	m_bForcedCooling = FALSE;
	m_bTakeOverStatus = FALSE;
	m_bForcedRefresh = FALSE;
	//m_nGPU_LockClock = 0;
	
}
CCore::~CCore()
{
	Uninit();
}

BOOL CCore::Init()
{


	if (m_hInstDLL)
	{
		return TRUE;
	}

	TRACE0("内核开始初始化。\n");
	m_nInit = -1;
	//
	//CString dllpth = GetExePath() + "\\ClevoEcInfo.dll";

	//m_hInstDLL = LoadLibrary(dllpth);
	//if (m_hInstDLL == NULL)
	//{
	//	AfxMessageBox("无法加载" + dllpth + "，请确保该文件在程序目录下，并且已安装NTPortDrv。");
	//	return FALSE;
	//}

	//m_pfnInitIo = (InitIo *)::GetProcAddress(m_hInstDLL, "InitIo");
	//m_pfnSetFanDuty = (::SetFanDuty *)::GetProcAddress(m_hInstDLL, "SetFanDuty");
	//m_pfnSetFANDutyAuto = (SetFANDutyAuto *)::GetProcAddress(m_hInstDLL, "SetFanDutyAuto");
	//m_pfnGetTempFanDuty = (GetTempFanDuty *)::GetProcAddress(m_hInstDLL, "GetTempFanDuty");
	//m_pfnGetFANCounter = (GetFANCounter *)::GetProcAddress(m_hInstDLL, "GetFanCount");

	//m_pfnGetECVersion = (GetECVersion *)::GetProcAddress(m_hInstDLL, "GetECVersion");
	//m_pfnGetFANRPM[0] = (GetFanRpm *)::GetProcAddress(m_hInstDLL, "GetCpuFanRpm");
	//m_pfnGetFANRPM[1] = (GetFanRpm *)::GetProcAddress(m_hInstDLL, "GetGpuFanRpm");
	////m_pfnGetFANRPM[2] = (GetFanRpm *)::GetProcAddress(m_hInstDLL, "GetGpu1FanRpm");
	////m_pfnGetFANRPM[3] = (GetFanRpm *)::GetProcAddress(m_hInstDLL, "GetX72FanRpm");

	//if (m_pfnInitIo == NULL)
	//{
	//	FreeLibrary(m_hInstDLL);
	//	m_hInstDLL = NULL;
	//	AfxMessageBox("错误的ClevoEcInfo.dll");
	//	return FALSE;
	//}

	//if (m_pfnInitIo() != 1)
	//{
	//	FreeLibrary(m_hInstDLL);
	//	m_hInstDLL = NULL;
	//	AfxMessageBox("接口初始化返回值错误！");
	//	return FALSE;
	//}

	/*
	int aa = 0, bb = 0;
	const char *str = m_pfnGetECVersion();
	int a = m_pfnGetCPUFANRPM();
	int b = m_pfnGetGPUFANRPM();
	ECData data1 = m_pfnGetTempFanDuty(1);
	ECData data2 = m_pfnGetTempFanDuty(2);
	m_pfnSetFanDuty(1, 128);
	m_pfnSetFANDutyAuto(1);
	m_pfnSetFanDuty(2, 128);
	m_pfnSetFANDutyAuto(2);
	*/
	//
	TRACE0("内核初始化成功。\n");
	m_nInit = 1;
	return TRUE;
}



void CCore::Uninit()
{
	ResetFan();
	//if (m_hInstDLL != NULL)
	//{
	//	FreeLibrary(m_hInstDLL);
	//	m_hInstDLL = NULL;
	//}
	m_nInit = 0;
}
void CCore::Run() 
{
	static int nNextChecktTime = 0;
	static BOOL bSetPriority = FALSE;
	m_config.LoadConfig();
	
	// 初始化时执行CmdShell检查
	RunCmdShell(FALSE, TRUE);
	
	m_nPowerStableCountdown = 0; // 初始化电源稳定倒计时
	
	if (m_config.TakeOver == 1)
	{
		m_TakeOverTimeOut = 0;
	}

	if (!m_nInit)
	{
		Init();
		//if (m_config.LockGPUFrequency)
		//{
		//	//m_nGPU_LockClock = m_config.GPUOverClock;
		//}
	}

	if (m_nInit == 1)
	{
		TRACE0("内核开始运行。\n");
		int curtime;
		while (!m_nExit)
		{
			curtime = GetTime();
			if (curtime >= nNextChecktTime || m_bForcedRefresh)
			{
				//MessageBox(NULL , "工作中...", "MyFunColtrol" , 0);
				// 持续监控显示模式和进程状态
				// RunCmdShell(FALSE);
				// m_ApmPowerStatusChange = 0; // 重置电源状态变更标志
				
				// 使用InterlockedExchange原子操作获取并重置状态，防止竞争条件
				int powerChange = InterlockedExchange((LONG*)&m_ApmPowerStatusChange, 0);
				
				// 如果检测到电源变更，开始倒计时（防抖，例如等待3秒）
				if (powerChange)
				{
					// 设置倒计时为4,因为会立即递减一次,实际等待3秒
					m_nPowerStableCountdown = 4;
					LOG("检测到电源变更，开始3秒稳定等待...");
				}
				
				// 倒计时逻辑
				if (m_nPowerStableCountdown > 0)
				{
					m_nPowerStableCountdown--;
					LOG("电源稳定倒计时: %d", m_nPowerStableCountdown);
					if (m_nPowerStableCountdown == 0)
					{
						// 倒计时结束，视为状态稳定，执行切换
						// 传入 TRUE 表示这是由电源变更触发的
						LOG("倒计时结束，准备执行RunCmdShell...");
						RunCmdShell(FALSE, TRUE);
					}
				}
				else
				{
					// 常规轮询 (内部限制每10秒一次)
					// 传入 FALSE 表示非电源变更触发
					RunCmdShell(FALSE, FALSE);
				}
				
				Work();
				m_nLastUpdateTime = curtime;//更新时间
				
				// 设定下一次检查时间
				if (m_nPowerStableCountdown > 0)
					nNextChecktTime = GetTime(NULL, 1); // 倒计时期间每秒检查
				else
					nNextChecktTime = GetTime(NULL, m_config.UpdateInterval); // 正常间隔
				m_bForcedRefresh = FALSE;
				if (!bSetPriority)
				{
					bSetPriority = TRUE;
					SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);//在首次更新成功后才设置高优先级
				}
			}
			Sleep(1000);
		}
		TRACE0("内核结束运行。\n");
	}
	m_nExit = 2;
}
void CCore::Work()
{
	if (m_GpuInfo.nv_Api_init == 1)
		m_GpuInfo.Update();
	else
		Sleep(500);

	//锁定GPU频率
	int limitClock;
	int limitTime = m_config.timelimit;
	int baseClockLimit = m_config.baseClockLimit;
	int lowClockLimit = m_config.lowClockLimit;
	float upClockRatio = (float)(m_config.upClockRatio / 100.0);
	float downClockRatio = (float)(m_config.downClockRatio / 100.0);
	int limit_overclock = 200;
	int resultLog = -1;
	int baseMemClock = 405;
	int nGPU_Util_limit = m_config.utilIdleThreshold;
	//m_core.m_config.Linear 线性控制
	int count_time = limitTime * m_config.UpdateInterval; //使用周期*时间统计

	if (m_config.TakeOver)
	{
		CalculateTargetFrequency();
	}
	
	//if ((m_config.GPUOverClock > 0 || m_config.GPUOverClock < limit_overclock) && (m_GpuInfo.m_nGPU_Util > 0 || m_start_overclock == 0))
	//if ((m_config.GPUOverClock >= 0 || m_config.GPUOverClock < limit_overclock) && (m_start_overclock == 0))

	if ((m_config.GPUOverClock >= 0 && m_config.GPUOverClock < limit_overclock))
	{
	
		if (m_start_overclock == 0)
		{
			//m_GpuInfo.m_nOverClock = m_config.GPUOverClock;
			//LOG(resultLog = m_config.GPUOverClock);
			Sleep(1000);
			if (NvApiGpuHandles[gpuBusId] != 0)
			{
				LOG("Check NvApiGpuHandles[%d] : %d ", busId, NvApiGpuHandles[gpuBusId]);
				unsigned int voltageUV_t[255] = { 0 };
				int frequencyDeltaKHz_t[255] = { 0 };
				int count_t = -1;
				int ret = -1;
				int overmemclock = baseMemClock + m_config.GPUOverMEMClock;
				BOOL oc_ret = 1;
				if (m_GpuInfo.nv_Api_init == 1)
					ret = NvApiGetCurve(gpuBusId, (unsigned int*)&count_t, voltageUV_t, frequencyDeltaKHz_t, 1);
				else
					ret = NvApiGetCurve(gpuBusId, (unsigned int*)&count_t, voltageUV_t, frequencyDeltaKHz_t);

				if (ret == 0)
				{

					if (frequencyDeltaKHz_t[0] > 0 && frequencyDeltaKHz_t[127] > 0)
					{
						int freq_over_clock = frequencyDeltaKHz_t[0] / 500;
						int freq_over_clock_limit = frequencyDeltaKHz_t[127] / 500;
						m_GpuInfo.nv_Api_init = 1;
						LOG(resultLog = freq_over_clock);
						LOG(resultLog = freq_over_clock_limit);
						if (freq_over_clock == freq_over_clock_limit || (m_GpuInfo.m_nMemoryClock > 5000 && m_GpuInfo.m_nMemoryClock != overmemclock && m_GpuInfo.m_nMemoryClock != overmemclock+1)  || (freq_over_clock_limit > m_config.OverClock2) || ((freq_over_clock > m_config.OverClock2) && (freq_over_clock_limit < m_config.OverClock2)))
						{
							m_GpuInfo.ForcedRefreshGPU = 1;
							LOG(resultLog = frequencyDeltaKHz_t[0] / 500);
							if (m_config.CurveUV_limit > 0)
								oc_ret = m_GpuInfo.OverClockFrequency(m_config.GPUOverClock, m_config.GPUOverMEMClock, m_config.CurveUV_limit, m_config.OverClock2);
							if (!oc_ret)
								m_start_overclock = 2;
							else
								m_GpuInfo.OverClockFrequency(m_config.GPUOverClock, m_config.GPUOverMEMClock, m_config.OverClock2);
							LOG(resultLog = m_config.GPUOverClock);
							m_TakeOverTimeOut = 1;
						}
						else
						{
							LOG("frequencyDeltaKH check same,no change");
						}

						//else
						//{
						//	//if (m_GpuInfo.nv_Api_init == 2)
						//	m_GpuInfo.nv_Api_init == 1;
						//}
					}
					else if (frequencyDeltaKHz_t[0] > 0)
					{
						if (m_GpuInfo.nv_Api_init == 0)
						{
							m_GpuInfo.nv_Api_init = 1;
						}
						int freq_over_clock = frequencyDeltaKHz_t[0] / 500;
						if (freq_over_clock != m_config.GPUOverClock  || (m_GpuInfo.m_nMemoryClock > 5000  && m_GpuInfo.m_nMemoryClock != overmemclock  && m_GpuInfo.m_nMemoryClock != overmemclock+1))
						{
							m_GpuInfo.ForcedRefreshGPU = 1;
							LOG(frequencyDeltaKHz_t[0] / 500);
							if (m_config.CurveUV_limit > 0)
								oc_ret = m_GpuInfo.OverClockFrequency(m_config.GPUOverClock, m_config.GPUOverMEMClock, m_config.CurveUV_limit, m_config.OverClock2);
							if (!oc_ret)
								m_start_overclock = 2;
							else
								oc_ret = m_GpuInfo.OverClockFrequency(m_config.GPUOverClock, m_config.GPUOverMEMClock, m_config.OverClock2);
							LOG(resultLog = freq_over_clock);
							LOG(resultLog = m_config.GPUOverClock);
							m_TakeOverTimeOut = 1;
						}
						else
						{
							if (m_start_overclock == 0)
							{
								//m_GpuInfo.m_nOverClock = m_config.GPUOverClock;
								if (m_TakeOverTimeOut == 0)
								{
									LOG(resultLog = m_TakeOverTimeOut);
									//m_TakeOverTimeOut = GetTime(NULL, 30);//下一个超时
								}
								//LOG(resultLog = freq_over_clock);
								//LOG(resultLog = m_config.GPUOverClock);
							}
							int overclockcurtime = GetTime();
							if (overclockcurtime > m_TakeOverTimeOut && m_start_overclock == 0) 
							{

								m_start_overclock = 1;
								//m_TakeOverTimeOut = 0;
								LOG(resultLog = freq_over_clock);
								LOG(resultLog = m_config.GPUOverClock);
								LOG(resultLog = m_start_overclock);
							}
						}
					}
					else 
					{

						LOG("NvApiGet frequencyDeltaKHz = 0,Pls check err,Then Chk m_start_overclock 0 To 1 ");
						LOG( resultLog = frequencyDeltaKHz_t[0] / 500);
						if (m_start_overclock == 0)
							m_start_overclock = 1;
							LOG(resultLog = m_start_overclock);

					}
				}
				else
				{
					LOG("NvApiGetCurve error ret:%d", ret);
				}
			}
			else
			{
				LOG("Check NvApiGpuHandles[%d] : Fail:%d", busId, NvApiGpuHandles[gpuBusId]);
			}
		}
	}
	else
	{
		LOG(resultLog = m_config.GPUOverClock);
	}


	//if (m_GpuInfo.m_nGraphicsClock >= baseClockLimit)
	if (m_GpuInfo.m_nGraphicsClock >= lowClockLimit - 5 || m_config.GPU_LockClock > m_GpuInfo.m_nGraphicsClock )
	{
		if (m_config.LockGPUFrequency)
		{
			//if (m_GpuInfo.m_nGraphicsClock != m_config.GPUFrequency)
			//{
			if (m_config.TakeOver)
				m_GpuInfo.LockFrequency(m_config.GPUFrequency);
			else
				m_GpuInfo.LockFrequency(m_config.GPU_LockClock);
			//}

		}
		else
			m_GpuInfo.LockFrequency(0);
	}
}


void CCore::Update()
{
	ECData data;
	int TempErr = 0;
	for (int i = 0; i < 2; i++)
	{
		data = m_pfnGetTempFanDuty(i+1);
		if (abs(data.Remote - this->m_nCurTemp[i]) > 30)
		{
			//AfxMessageBox("获取温度有误");
			//温度获取可能有误，重试一次
			if (TempErr++ == 0)
			{
				Sleep(1000);
				i--;
				continue;//重试一次
			}
		}
		this->m_nLastTemp[i] = this->m_nCurTemp[i];
		this->m_nCurTemp[i] = data.Remote;
		this->m_nCurDuty[i] = int(data.FanDuty * 100 / 255.0 + 0.5);

		if (m_bUpdateRPM)//获取风扇转速
		{
			int val = m_pfnGetFANRPM[i]();
			if (val == 0)
				this->m_nCurRPM[i] = 0;
			if (val > 300 && val < 5000)
				this->m_nCurRPM[i] = 2100000 / val;
		}
		else
		{
			this->m_nCurRPM[i] = -1;
		}
		TempErr = 0;
	}
	if (m_bUpdateRPM)
		m_GpuInfo.Update();
}
void CCore::Control()
{
	if (m_config.Linear)
		CalcLinearDuty();
	else
		CalcStdDuty();
	//设定转速
	SetFanDuty();

}
void CCore::CalcLinearDuty()
{
	static int nLastTemp[2] = { 0, 0 };//每次用于计算转速的温度

	//int duty,dl;
	//int j;
	//for (int i = 0; i < 2; i++)
	//{
	//	nLastTemp[i] = max(nLastTemp[i], m_nCurTemp[i]);//温度上升时立刻以当前温度计算转速
	//	nLastTemp[i] = min(nLastTemp[i], m_nCurTemp[i] + m_config.TransitionTemp);//温度下降时以当前温度+过渡温度来计算转速

	//	j = nLastTemp[i];//计算转速使用的温度

	//	if (j < 45)
	//	{
	//		duty = m_config.DutyList[i][9];
	//		dl = 0;
	//	}
	//	else if (j >= 90)
	//	{
	//		duty = m_config.DutyList[i][0];
	//		dl = 10;
	//	}
	//	else
	//	{
	//		int idx = 0;
	//		if (j < 50)
	//			idx = 8;
	//		else if (j < 55)
	//			idx = 7;
	//		else if (j < 60)
	//			idx = 6;
	//		else if (j < 65)
	//			idx = 5;
	//		else if (j < 70)
	//			idx = 4;
	//		else if (j < 75)
	//			idx = 3;
	//		else if (j < 80)
	//			idx = 2;
	//		else if (j < 85)
	//			idx = 1;
	//		else// if (j < 90)
	//			idx = 0;

	//		int temp_l = TEMP_LIST[idx + 1];
	//		int temp_h = TEMP_LIST[idx];
	//		int duty_l = m_config.DutyList[i][idx + 1];
	//		int duty_h = m_config.DutyList[i][idx];
	//		duty = int((duty_h - duty_l) / double(temp_h - temp_l) * (j - temp_l) + 0.5) + duty_l;
	//		dl = 9 - idx;
	//	}
	//	m_nSetDuty[i] = duty;
	//	m_nSetDutyLevel[i] = dl;
	//}
}
void CCore::CalcStdDuty()
{
	int dl;
	int last_dl;
	int j,k;
	for (int i = 0; i < 2; i++)
	{
		j = m_nCurTemp[i];
		last_dl = m_nSetDutyLevel[i];//上一次的负载等级
		for (k = 0; k < 10; k++)
		{
			dl = 10 - k;
			if (j >= TEMP_LIST[k])
			{
				break;
			}
			else if (j < TEMP_LIST[k] - m_config.TransitionTemp)
			{
				continue;
			}
			else
			{
				//根据上一次的负载挡位决定
				if (last_dl >= dl)
				{
					break;
				}
				continue;
			}
		}
		k = min(9, k);
		m_nSetDuty[i] = m_config.DutyList[i][k];
		m_nSetDutyLevel[i] = dl;
	}
}
void CCore::ResetFan()
{
	if (m_bTakeOverStatus)
	{
		m_nSetDuty[0] = 0;
		m_nSetDutyLevel[0] = 0;
		m_nSetDuty[1] = 0;
		m_nSetDutyLevel[1] = 0;
		m_pfnSetFANDutyAuto(1);
		m_pfnSetFANDutyAuto(2);
		m_pfnSetFANDutyAuto(3);
		m_bTakeOverStatus = FALSE;
	}
}

void CCore::ResetGPUFrequancy()
{
	m_GpuInfo.ForcedRefreshGPU = 1;
	//检测曲线变化
	//m_config.GPU_LockClock = m_config.GPUFrequency;
	m_start_overclock = 0;

}

void CCore::ResetSleepStatus()
{
	int resultLog = -1;
	LOG("ResetSleepStatus");
	m_start_overclock = 0;
	LOG(resultLog = m_start_overclock);
	m_TakeOverTimeOut = 0;
	Sleep(10000);
	
}

void CCore::SetFanDuty()
{
	int duty;
	for (int i = 0; i < 2; i++)
	{
		if (m_nCurDuty[i] == m_nSetDuty[i])
			continue;
		duty = int(m_nSetDuty[i] * 255.0 / 100 + 0.5);
		m_pfnSetFanDuty(i + 1, duty);
		if (i == 1)
			m_pfnSetFanDuty(i + 2, duty);//如果存在第3个风扇
	}
	m_bTakeOverStatus = TRUE;
}

// RunCmdShell - 执行自定义命令脚本和进程检查
// 功能:
// 1. 根据电池状态和显示频率执行显示模式切换脚本
// 2. 检查并启动指定进程
// 参数: bForce - 强制执行
//       bPowerStatusChange - 电源状态是否刚发生改变
// BOOL CCore::RunCmdShell(BOOL bForce, BOOL bPowerStatusChange)
// {
// 	CString dirPath = GetExePath();
// 	CString inipath = dirPath + "\\MyFanconfig.ini";
	
// 	// 检查配置文件是否存在
// 	if (!FileExists(inipath))
// 	{
// 		return FALSE;
// 	}
	
// 	// 读取CmdShell配置
// 	char returnValue[100];
// 	CString cmdpath, runcmd, runcmdpath;
// 	int runcmdshow = SW_SHOWMINIMIZED;
	
// 	GetPrivateProfileString("cmdshell", "filepath", 0, returnValue, 100, inipath);
// 	cmdpath = returnValue;
// 	GetPrivateProfileString("cmdshell", "runcmd", 0, returnValue, 100, inipath);
// 	runcmd = returnValue;
// 	runcmdpath = runcmd + " " + cmdpath;
// 	GetPrivateProfileString("cmdshell", "showcmd", 0, returnValue, 100, inipath);
// 	runcmdshow = atoi(returnValue);
	
// 	// 读取进程检查配置
// 	CString processname, processpath, processcmd, processcmdpath;
// 	int procshowcmd = SW_SHOWMINIMIZED;
	
// 	GetPrivateProfileString("processcheck", "processname", 0, returnValue, 100, inipath);
// 	processname = returnValue;
// 	GetPrivateProfileString("processcheck", "processpath", 0, returnValue, 100, inipath);
// 	processpath = returnValue;
// 	GetPrivateProfileString("processcheck", "processcmd", 0, returnValue, 100, inipath);
// 	processcmd = returnValue;
// 	processcmdpath = processcmd + " " + processpath;
// 	GetPrivateProfileString("processcheck", "procshowcmd", 0, returnValue, 100, inipath);
// 	procshowcmd = atoi(returnValue);
	
// 	// 获取当前电池和显示状态
// 	int battery_ACLine = GetBatteryACLineStatus();
// 	int dmFrequency = GetDisplayFrequency();
	
// 	// 防止频繁执行的简单的冷却计时器
// 	static int lastExecTime = 0;
// 	int curTime = GetTime();
	
// 	// 状态记忆：记录上次检测到的AC和Freq状态
// 	static int lastKnownACLine = -1;
// 	static int lastKnownFreq = -1;
	
// 	// 执行显示模式切换命令
// 	// 逻辑:
// 	// 1. 强制执行
// 	// 2. 电池模式(AC=0) 且 屏幕频率不是60Hz -> 切换(预期切到60Hz)
// 	// 3. 电源模式(AC=1) 且 屏幕频率是60Hz -> 切换(预期切到高刷)
// 	bool bNeedSwitch = false;
	
// 	if (battery_ACLine == 0 && dmFrequency != 60)
// 		bNeedSwitch = true;
// 	else if (battery_ACLine == 1 && dmFrequency == 60)
// 		bNeedSwitch = true;
	
// 	// 状态变化检测：只有当AC或Freq状态发生实际变化时，才认为需要重新尝试
// 	bool bStateChanged = (battery_ACLine != lastKnownACLine) || (dmFrequency != lastKnownFreq);
	
// 	// 调试日志：输出当前状态信息
// 	if (bPowerStatusChange || bStateChanged)
// 	{
// 		LOG("RunCmdShell: 状态检查 - AC=%d(上次=%d), Freq=%d(上次=%d)", 
// 			battery_ACLine, lastKnownACLine, dmFrequency, lastKnownFreq);
// 		LOG("RunCmdShell: 状态标志 - NeedSwitch=%d, StateChanged=%d, PowerChange=%d", 
// 			bNeedSwitch, bStateChanged, bPowerStatusChange);
// 	}
		
// 	if (FileExists(cmdpath))
// 	{
// 		int intervalSeconds = 0;
// 		GetTimeInterval(curTime, lastExecTime, &intervalSeconds);
// 		intervalSeconds = abs(intervalSeconds); // 获取秒数差

// 		// 执行条件：
// 		// 1. 强制执行
// 		// 2. 需要切换 且 电源刚变更(需间隔>3秒)
// 		// 3. 需要切换 且 状态发生变化 且 轮询检查(需间隔>10秒)
// 		// 移除了无状态变化时的自动重试，避免脚本失败时的无限循环
// 		BOOL bTimeCondition = FALSE;
// 		if (bPowerStatusChange)
// 		{
// 			if (intervalSeconds > 3) bTimeCondition = TRUE;
// 		}
// 		else if (bStateChanged)
// 		{
// 			// 只有状态变化时才允许轮询重试
// 			if (intervalSeconds > 10) bTimeCondition = TRUE;
// 		}

// 		if (bForce || (bNeedSwitch && bTimeCondition))
// 		{
// 			// 只有在非强制执行时才记录日志(避免初始化时的刷屏? 其实初始化时也只是一次)
// 			// 为了调试，我们总是记录
// 			LOG("RunCmdShell: 触发切换. AC=%d, Freq=%d, Force=%d, PowerChange=%d, Inteval=%ds", 
// 				battery_ACLine, dmFrequency, bForce, bPowerStatusChange, intervalSeconds);
			
// 			int result = WinExec(runcmdpath, runcmdshow);
// 			if (result > 31)
// 				LOG("RunCmdShell: 脚本执行成功 (返回值=%d)", result);
// 			else
// 				LOG("RunCmdShell: 脚本执行失败 (错误码=%d)", result);
			
// 			lastExecTime = curTime;
// 			// 更新状态记忆，防止在同一状态下重复执行
// 			lastKnownACLine = battery_ACLine;
// 			lastKnownFreq = dmFrequency;
// 			return TRUE; // 成功执行
// 		}
// 		else if (bNeedSwitch || bStateChanged)
// 		{
// 			// 需要切换但不满足时间条件，输出原因
// 			LOG("RunCmdShell: 跳过执行 - NeedSwitch=%d, TimeCondition=%d, Interval=%ds, PowerChange=%d, StateChanged=%d", 
// 				bNeedSwitch, bTimeCondition, intervalSeconds, bPowerStatusChange, bStateChanged);
// 		}
// 	}
	
// 	// 检查并启动进程
// 	if (!processname.IsEmpty())
// 	{
// 		// 降低检查频率? 不，进程检查轻量级
// 		// 修复: 检查 processpath 而不是 processcmdpath (后者是完整命令行，不是文件路径)
// 		if (FindProcessIDByName(processname.GetString()) == 0 && FileExists(processpath))
// 		{
// 			// 防止连续启动尝试，也加上冷却时间? 
// 			// ProcessIDByName返回0说明没运行，应该启动。
// 			// 如果启动失败，我们不希望死循环。
// 			static int lastProcExecTime = 0;
// 			if (abs(curTime - lastProcExecTime) > 10) 
// 			{
// 				LOG("RunCmdShell: 检测到进程[%s]未运行, 正在启动...", processname.GetString());
// 				LOG("RunCmdShell: 执行命令: %s", processcmdpath.GetString());
// 				int result = WinExec(processcmdpath, procshowcmd);
// 				if (result > 31)
// 					LOG("RunCmdShell: 进程启动成功 (返回值=%d)", result);
// 				else
// 					LOG("RunCmdShell: 进程启动失败 (错误码=%d)", result);
// 				lastProcExecTime = curTime;
// 			}
// 		}
// 		else if (FindProcessIDByName(processname.GetString()) == 0 && !FileExists(processpath))
// 		{
// 			// 进程不存在且文件路径也不存在，记录警告
// 			static int lastWarningTime = 0;
// 			if (abs(curTime - lastWarningTime) > 60) // 每60秒警告一次
// 			{
// 				LOG("RunCmdShell: 警告 - 进程[%s]未运行，但文件路径不存在: %s", 
// 					processname.GetString(), processpath.GetString());
// 				lastWarningTime = curTime;
// 			}
// 		}
// 	}
	
// 	return FALSE; // 未执行切换
// }

BOOL CCore::RunCmdShell(BOOL bForce, BOOL bPowerStatusChange)
{
    CString dirPath = GetExePath();
    CString inipath = dirPath + "\\MyFanconfig.ini";

    if (!FileExists(inipath))
        return FALSE;

    // ---------------- 读取 CmdShell 配置 ----------------
    char returnValue[100] = {0};
    CString cmdpath, runcmd, runcmdpath;
    int runcmdshow = SW_SHOWMINIMIZED;

    GetPrivateProfileString("cmdshell", "filepath", "", returnValue, 100, inipath);
    cmdpath = returnValue;
    GetPrivateProfileString("cmdshell", "runcmd", "", returnValue, 100, inipath);
    runcmd = returnValue;
    runcmdpath = runcmd + " " + cmdpath;
    GetPrivateProfileString("cmdshell", "showcmd", "2", returnValue, 100, inipath);
    runcmdshow = atoi(returnValue);
	
	// 读取进程检查配置
	CString processname, processpath, processcmd, processcmdpath;
	int procshowcmd = SW_SHOWMINIMIZED;
	
	GetPrivateProfileString("processcheck", "processname", 0, returnValue, 100, inipath);
	processname = returnValue;
	GetPrivateProfileString("processcheck", "processpath", 0, returnValue, 100, inipath);
	processpath = returnValue;
	GetPrivateProfileString("processcheck", "processcmd", 0, returnValue, 100, inipath);
	processcmd = returnValue;
	processcmdpath = processcmd + " " + processpath;
	GetPrivateProfileString("processcheck", "procshowcmd", 0, returnValue, 100, inipath);
	procshowcmd = atoi(returnValue);
	
    // ---------------- 当前系统状态 ----------------
    int battery_ACLine = GetBatteryACLineStatus();   // 0=电池 1=AC
    int dmFrequency   = GetDisplayFrequency();       // Hz

    static int  lastExecTime     = 0;
    static int  lastKnownACLine  = -1;
    static int  lastKnownFreq    = -1;
    static BOOL s_waitingEffect  = FALSE;

    int curTime = GetTime();

    // ---------------- 是否需要切换 ----------------
    bool bNeedSwitch = false;
    if (battery_ACLine == 0 && dmFrequency != 60)
        bNeedSwitch = true;
    else if (battery_ACLine == 1 && dmFrequency == 60)
        bNeedSwitch = true;

    bool bStateChanged =
        (battery_ACLine != lastKnownACLine) ||
        (dmFrequency   != lastKnownFreq);

    // ---------------- 等待脚本效果生效 ----------------
    // 脚本刚执行完，但系统状态还没变 —— 这是正常的
    if (s_waitingEffect)
    {
        if (bStateChanged)
        {
            LOG("RunCmdShell: 状态已生效 AC=%d Freq=%d",
                battery_ACLine, dmFrequency);

            lastKnownACLine = battery_ACLine;
            lastKnownFreq   = dmFrequency;
            s_waitingEffect = FALSE;
        }
        else
        {
            // 状态未生效前，彻底退出，不刷 skip 日志
            return FALSE;
        }
    }

    // ---------------- 调试日志 ----------------
    if (bPowerStatusChange || bStateChanged)
    {
        LOG("RunCmdShell: 状态检查 AC=%d(上次=%d) Freq=%d(上次=%d)",
            battery_ACLine, lastKnownACLine,
            dmFrequency,   lastKnownFreq);
        LOG("RunCmdShell: 标志 NeedSwitch=%d StateChanged=%d PowerChange=%d",
            bNeedSwitch, bStateChanged, bPowerStatusChange);
    }

    // ---------------- 执行 CmdShell ----------------
    if (FileExists(cmdpath))
    {
        int intervalSeconds = abs(curTime - lastExecTime);
        BOOL bTimeCondition = FALSE;

        if (bPowerStatusChange)
        {
            if (intervalSeconds > 3)
                bTimeCondition = TRUE;
        }
        else if (bStateChanged)
        {
            if (intervalSeconds > 10)
                bTimeCondition = TRUE;
        }

        // if (bForce || (bNeedSwitch && bTimeCondition))
        // {
        //     LOG("RunCmdShell: 触发执行 AC=%d Freq=%d Force=%d Interval=%ds",
        //         battery_ACLine, dmFrequency, bForce, intervalSeconds);

        //     int result = WinExec(runcmdpath, runcmdshow);
        //     if (result > 31)
        //         LOG("RunCmdShell: 脚本执行成功 (返回值=%d)", result);
        //     else
        //         LOG("RunCmdShell: 脚本执行失败 (错误码=%d)", result);

        //     lastExecTime    = curTime;
        //     s_waitingEffect = TRUE;   // ⭐关键：等待真实状态变化
        //     return TRUE;
        // }
		BOOL bShouldRun = FALSE;

		// 1. 强制
		if (bForce)
		{
			bShouldRun = TRUE;
		}
		// 2. 电源状态发生变化 → 必执行一次
		else if (bPowerStatusChange && bTimeCondition)
		{
			bShouldRun = TRUE;
		}
		// 3. 非电源变化，但状态确实不匹配 → 允许补救
		else if (bNeedSwitch && bTimeCondition)
		{
			bShouldRun = TRUE;
		}

		if (bShouldRun)
		{
			LOG("RunCmdShell: 触发执行 AC=%d Freq=%d Force=%d PowerChange=%d Interval=%ds",
				battery_ACLine, dmFrequency, bForce, bPowerStatusChange, intervalSeconds);

			int result = WinExec(runcmdpath, runcmdshow);
			if (result > 31)
				LOG("RunCmdShell: 脚本执行成功 (返回值=%d)", result);
			else
				LOG("RunCmdShell: 脚本执行失败 (错误码=%d)", result);

			lastExecTime    = curTime;
			s_waitingEffect = TRUE;
			return TRUE;
		}
	}
	// 检查并启动进程
	if (!processname.IsEmpty())
	{
		// 降低检查频率? 不，进程检查轻量级
		// 修复: 检查 processpath 而不是 processcmdpath (后者是完整命令行，不是文件路径)
		if (FindProcessIDByName(processname.GetString()) == 0 && FileExists(processpath))
		{
			// 防止连续启动尝试，也加上冷却时间? 
			// ProcessIDByName返回0说明没运行，应该启动。
			// 如果启动失败，我们不希望死循环。
			static int lastProcExecTime = 0;
			if (abs(curTime - lastProcExecTime) > 10) 
			{
				LOG("RunCmdShell: 检测到进程[%s]未运行, 正在启动...", processname.GetString());
				LOG("RunCmdShell: 执行命令: %s", processcmdpath.GetString());
				int result = WinExec(processcmdpath, procshowcmd);
				if (result > 31)
					LOG("RunCmdShell: 进程启动成功 (返回值=%d)", result);
				else
					LOG("RunCmdShell: 进程启动失败 (错误码=%d)", result);
				lastProcExecTime = curTime;
			}
		}
		else if (FindProcessIDByName(processname.GetString()) == 0 && !FileExists(processpath))
		{
			// 进程不存在且文件路径也不存在，记录警告
			static int lastWarningTime = 0;
			if (abs(curTime - lastWarningTime) > 60) // 每60秒警告一次
			{
				LOG("RunCmdShell: 警告 - 进程[%s]未运行，但文件路径不存在: %s", 
					processname.GetString(), processpath.GetString());
				lastWarningTime = curTime;
			}
		}
	}

    return FALSE;
}


// Profile Management Implementation

CString CConfig::GetProfilePath(CString profileName)
{
	CString dirPath = GetExePath();
	CString profilesDir = dirPath + "\\profiles";
	if (!FileExists(profilesDir))
	{
		CreateDirectory(profilesDir, NULL);
	}
	return profilesDir + "\\profile_" + profileName + ".ini";
}

void CConfig::SaveProfile(CString profileName)
{
	CString path = GetProfilePath(profileName);
	CString str;
	
	// Save DutyList
	for(int i=0; i<2; i++) {
		for(int j=0; j<10; j++) {
			str.Format("%d", DutyList[i][j]);
			CString key;
			key.Format("DutyList_%d_%d", i, j);
			WritePrivateProfileString("Config", key, str, path);
		}
	}

	// Save other fields
	#define SAVE_INT(x) str.Format("%d", x); WritePrivateProfileString("Config", #x, str, path)
	#define SAVE_BOOL(x) str.Format("%d", x); WritePrivateProfileString("Config", #x, str, path)

	SAVE_INT(TransitionTemp);
	SAVE_INT(UpdateInterval);
	SAVE_BOOL(Linear);
	SAVE_BOOL(TakeOver);
	SAVE_INT(ForceTemp);
	SAVE_BOOL(LockGPUFrequency);
	SAVE_INT(GPUFrequency);
	SAVE_INT(GPUOverClock);
	SAVE_INT(GPUOverMEMClock);
	SAVE_INT(TakeOverDown);
	SAVE_INT(TakeOverUp);
	SAVE_INT(TakeOverLock);
	SAVE_INT(upClockPercent);
	SAVE_INT(downClockPercent);
	SAVE_INT(downTemplimit);
	SAVE_INT(upTemplimit);
	SAVE_INT(upClocklimit);
	SAVE_INT(timelimit);
	SAVE_INT(GPU_LockClock);
	SAVE_INT(CurveUV_limit);
	SAVE_INT(OverClock2);
	
	m_sCurrentProfile = profileName;
}

void CConfig::LoadProfile(CString profileName)
{
	CString path = GetProfilePath(profileName);
	if(!FileExists(path)) return;

	char buf[256];
	
	// Load DutyList
	for(int i=0; i<2; i++) {
		for(int j=0; j<10; j++) {
			CString key;
			key.Format("DutyList_%d_%d", i, j);
			GetPrivateProfileString("Config", key, "0", buf, 256, path);
			DutyList[i][j] = atoi(buf);
		}
	}

	#define LOAD_INT(x) GetPrivateProfileString("Config", #x, "0", buf, 256, path); x = atoi(buf)
	#define LOAD_BOOL(x) GetPrivateProfileString("Config", #x, "0", buf, 256, path); x = atoi(buf)

	LOAD_INT(TransitionTemp);
	LOAD_INT(UpdateInterval);
	LOAD_BOOL(Linear);
	LOAD_BOOL(TakeOver);
	LOAD_INT(ForceTemp);
	LOAD_BOOL(LockGPUFrequency);
	LOAD_INT(GPUFrequency);
	LOAD_INT(GPUOverClock);
	LOAD_INT(GPUOverMEMClock);
	LOAD_INT(TakeOverDown);
	LOAD_INT(TakeOverUp);
	LOAD_INT(TakeOverLock);
	LOAD_INT(upClockPercent);
	LOAD_INT(downClockPercent);
	LOAD_INT(downTemplimit);
	LOAD_INT(upTemplimit);
	LOAD_INT(upClocklimit);
	LOAD_INT(timelimit);
	LOAD_INT(GPU_LockClock);
	LOAD_INT(CurveUV_limit);
	LOAD_INT(OverClock2);

	m_sCurrentProfile = profileName;
	SaveConfig();
}

void CConfig::DeleteProfile(CString profileName)
{
	CString path = GetProfilePath(profileName);
	DeleteFile(path);
	if(m_sCurrentProfile == profileName)
		//m_sCurrentProfile = "";
		m_sCurrentProfile.Empty();
}

void CConfig::GetProfileList(CStringArray& profiles)
{
	profiles.RemoveAll();
	CString dirPath = GetExePath();
	CString profilesDir = dirPath + "\\profiles";
	CString searchPath = profilesDir + "\\profile_*.ini";
	
	CFileFind finder;
	BOOL bWorking = finder.FindFile(searchPath);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		CString filename = finder.GetFileName();
		// filename is profile_<name>.ini
		// remove profile_ prefix and .ini suffix
		if(filename.Left(8) == "profile_" && filename.Right(4) == ".ini")
		{
			CString name = filename.Mid(8, filename.GetLength() - 12);
			profiles.Add(name);
		}
	}
}
