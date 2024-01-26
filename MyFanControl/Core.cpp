#include "stdafx.h"
#include "Core.h"

#include <Windows.h>
#include <direct.h>
#include <iostream>



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
	//ʱ����������6λ��ʱ�䣬�翪��ʱ��91500���õ�a-b����ת��Ϊ6λ��ʱ�䣬ָ��p��������Ƶ�ʱ���
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


CGPUInfo::CGPUInfo()
{
	if (Init())
	{
		Update();

		TRACE0("�ɹ�����NVGPU_DLL.dll��\n");

	}
	//

}

bool NvApi = true, Adl = false;
bool LogFileEnable = true;
unsigned int gpuBusId = 0;
void* NvApiGpuHandles[128] = { 0 };

//unsigned int AdlGpuIndexes[128] = { 0 };

//void* AdlContext = 0;

#define LOG(expression) Log(#expression, strrchr(__FILE__, '\\') + 1, __LINE__, (intptr_t) (expression))


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
int NvApiGetCurve(unsigned int gpuBusId, unsigned int* count, unsigned int* voltageUV, int* frequencyDeltaKHz,int checkCount=128)
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
		LockFrequency();//��ԭGPUƵ������
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

	nv_Api_init = 0; //��ʼ��check 128
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



	TRACE0("��ʼ����NVGPU_DLL.dll��\n");
	m_hGPUdll = NULL;
	CString dllpth = GetExePath() + "\\NVGPU_DLL.dll";
	m_hGPUdll = LoadLibrary(dllpth);
	if (m_hGPUdll == NULL)
	{
		TRACE0("�޷�����" + dllpth + "\n");
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
		TRACE0("InitGPU_API��ʼ��ʧ�ܡ�\n");
		LOG("m_hGPUdll InitGPU_API��ʼ��ʧ�ܡ�\n");
		FreeLibrary(m_hGPUdll);
		//LOG("m_hGPUdll InitGPU_API��ʼ��ʧ��");
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
		TRACE0("�������Ϊ0,���¼���NVGPU_DLL.dll��\n");
		LOG("�������Ϊ0,���¼���NVGPU_DLL.dll��\n");
		Init();
	}
	else {
		if (m_hGPUdll != NULL)
		{
			//TRACE0("�޷�����" + dllpth + "\n");
			LOG("m_hGPUdll isNotNULL Free and then reload Init");
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
				TRACE0("InitGPU_APIʧ�ܡ�\n");
				LOG("m_hGPUdll InitGPU_APIʧ�ܡ�\n");
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
	if (!m_pfnCheck_GPU_VRAM_Clock())
		return FALSE;
	m_pfnSet_GPU_Number(0);
	m_nGPU_Temp = m_pfnGet_GPU_Temp();
	m_nGraphicsClock = m_pfnGet_GPU_Graphics_Clock();
	m_nMemoryClock = m_pfnGet_GPU_Memory_Clock();
	m_nGPU_Util = m_pfnGet_GPU_Util();
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
		//��Ƶ
		GpuClock = frequency;
	}

	//else if (frequency > m_nStandardFrequency)
	//else if (frequency)
	//{
	//	//��Ƶ
	//	GpuOverclock = frequency - m_nStandardFrequency;
	//	MemOverclock = GpuOverclock * m_nMemoryRangeMax / m_nGraphicsRangeMax;//���ձ��������Դ泬Ƶ
	//}
	

	//
	//int rv1 = (m_pfnSet_CoreOC(0, GpuOverclock) == 0);
	//if (!rv1)
	//	AfxMessageBox("Set_CoreOCʧ��");
	int rv1 = 1;
	int rv2 = 1;

	int rv4 = 1;
	//
	//int rv2 = (m_pfnSet_MEMOC(0, MemOverclock) == 0);
	//if (!rv2)
	//	AfxMessageBox("Set_MEMOCʧ��");
	////
	int rv3 = (m_pfnLock_Frequency(0, GpuClock) == 0x19);

	//if (!rv3)
	//{
	//	CString str;
	//	str.Format("Lock_Frequencyʧ��: %d", GpuClock);
	//	AfxMessageBox(str);
	//	//AfxMessageBox("Lock_Frequencyʧ��:" + GpuClock);
	//}

	////
	//int rv4 = 1;
	////int rv4 = (m_pfnLock_Frequency_MEM(0, MemClock) == 0x19);
	//if (!rv4)
	//	AfxMessageBox("Lock_Frequency_MEMʧ��");
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
	//	//��Ƶ
	//	GpuClock = frequency;
	//}
	//else if (overClock > m_nStandardFrequency)
	if (overClock > m_nStandardFrequency)
	{
		//��Ƶ
		GpuOverclock = frequency;
		//MemOverclock = GpuOverclock * m_nMemoryRangeMax / m_nGraphicsRangeMax;//���ձ��������Դ泬Ƶ
		//MemOverclock = int (memOverClock /4); //����MemOverclockƵ��
		MemOverclock = memOverClock; //����MemOverclockƵ��

	}
	

	//
	int rv1 = (m_pfnSet_CoreOC(0, GpuOverclock) == 0);
	if (!rv1)
	{
		CString str;
		str.Format("Set_CoreOCʧ��: %d", GpuOverclock);
		AfxMessageBox(str);
		//AfxMessageBox("Set_CoreOCʧ��:"+ GpuOverclock);
	}
	//
	//int rv2 = 1;

	int rv2 = (m_pfnSet_MEMOC(0, MemOverclock) == 0);
	if (!rv2)
	{
		CString str;
		str.Format("Set_MEMOCʧ��: %d", MemOverclock);
		AfxMessageBox(str);
		//AfxMessageBox("Set_MEMOCʧ��:"+ MemOverclock);
	}
	

	//int rv3 = (m_pfnLock_Frequency(0, GpuClock) == 0x19);
	//if (!rv3)
	//	AfxMessageBox("Lock_Frequencyʧ��");
	////
	int rv3 = 1;
	int rv4 = 1;
	//int rv4 = (m_pfnLock_Frequency_MEM(0, MemClock) == 0x19);
	//if (!rv4)
	//	AfxMessageBox("Lock_Frequency_MEMʧ��");
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
		int limitUv = limitUV * 1000;

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
				//}  //������limit����

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
	//int upClockPercent;//ռ������Ƶ��ֵ
	//int downClockPercent;//ռ���ʽ�Ƶ��ֵ
	//int downTemplimit;//�¿ؽ�Ƶ��ֵ
	//int upTemplimit;//�¿���Ƶ��ֵ
	upClockPercent = 93;//ռ������Ƶ��ֵ
	downClockPercent = 80;//ռ���ʽ�Ƶ��ֵ
	downTemplimit = 82;//�¿ؽ�Ƶ��ֵ
	upTemplimit = 79;//�¿���Ƶ��ֵ
	upClocklimit = 1600; //��Ƶ����
	timelimit = 3;    //ͳ��ʱ��
	CurveUV_limit = 780; //Curve����
	OverClock2 = 145; //Curve����145Mhz

	int i = 0;
	DutyList[0][i++] = upClockPercent;//90+  //ռ������Ƶ��ֵ
	DutyList[0][i++] = downClockPercent;//85+	//��Ƶ��ֵ
	DutyList[0][i++] = downTemplimit;//80+	//�¿ؽ�Ƶ��ֵ
	DutyList[0][i++] = upTemplimit;//75+	//�¿���Ƶ��ֵ
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
	UpdateInterval = 2;
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

}
void CConfig::LoadConfig()
{
	CString strPath = GetExePath() + "\\MyFanControl.cfg";
	CFile file;
	if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
	{
		SaveConfig();
		if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
		{
			AfxMessageBox("�޷����������ļ�");
			return;
		}
	}
	if (file.GetLength() != sizeof(*this))
	{
		file.Close();
		SaveConfig();
		if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
		{
			AfxMessageBox("���ú���Ȼ�޷����������ļ�");
			return;
		}
		if (file.GetLength() != sizeof(*this))
		{
			AfxMessageBox("�����ļ���ʽ����ȷ");
			file.Close();
			return;
		}
	}
	file.Read(this, sizeof(*this));
	file.Close();
}
void CConfig::SaveConfig()
{
	FILE *fp = NULL;
	CString strPath = GetExePath() + "\\MyFanControl.cfg";
	fp = fopen(strPath, "wb");
	if (fp == NULL)
	{
		AfxMessageBox("�޷����������ļ�");
		return;
	}
	fwrite(this, sizeof(*this), 1, fp);
	fclose(fp);
}

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
		m_nCurTemp[i]=0;//��ǰ�¶�
		m_nLastTemp[i]=0;//��һ���¶�
		m_nSetDuty[i]=0;//���õĸ���
		m_nSetDutyLevel[i] = 0;//���õ�ת�ٵ�λ������ٵ�Ϊ1������ٵ�Ϊ10
		m_nCurDuty[i]=0;//��ǰ����
		m_nCurRPM[i]=0;//��ǰת��
	}
	m_bUpdateRPM=0;//�Ƿ����ת�٣����Ϊ0��ֻ���·����¶Ⱥ͸���
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

	TRACE0("�ں˿�ʼ��ʼ����\n");
	m_nInit = -1;
	//
	CString dllpth = GetExePath() + "\\ClevoEcInfo.dll";

	m_hInstDLL = LoadLibrary(dllpth);
	if (m_hInstDLL == NULL)
	{
		AfxMessageBox("�޷�����" + dllpth + "����ȷ�����ļ��ڳ���Ŀ¼�£������Ѱ�װNTPortDrv��");
		return FALSE;
	}

	m_pfnInitIo = (InitIo *)::GetProcAddress(m_hInstDLL, "InitIo");
	m_pfnSetFanDuty = (::SetFanDuty *)::GetProcAddress(m_hInstDLL, "SetFanDuty");
	m_pfnSetFANDutyAuto = (SetFANDutyAuto *)::GetProcAddress(m_hInstDLL, "SetFanDutyAuto");
	m_pfnGetTempFanDuty = (GetTempFanDuty *)::GetProcAddress(m_hInstDLL, "GetTempFanDuty");
	m_pfnGetFANCounter = (GetFANCounter *)::GetProcAddress(m_hInstDLL, "GetFanCount");

	m_pfnGetECVersion = (GetECVersion *)::GetProcAddress(m_hInstDLL, "GetECVersion");
	m_pfnGetFANRPM[0] = (GetFanRpm *)::GetProcAddress(m_hInstDLL, "GetCpuFanRpm");
	m_pfnGetFANRPM[1] = (GetFanRpm *)::GetProcAddress(m_hInstDLL, "GetGpuFanRpm");
	//m_pfnGetFANRPM[2] = (GetFanRpm *)::GetProcAddress(m_hInstDLL, "GetGpu1FanRpm");
	//m_pfnGetFANRPM[3] = (GetFanRpm *)::GetProcAddress(m_hInstDLL, "GetX72FanRpm");

	if (m_pfnInitIo == NULL)
	{
		FreeLibrary(m_hInstDLL);
		m_hInstDLL = NULL;
		AfxMessageBox("�����ClevoEcInfo.dll");
		return FALSE;
	}

	if (m_pfnInitIo() != 1)
	{
		FreeLibrary(m_hInstDLL);
		m_hInstDLL = NULL;
		AfxMessageBox("�ӿڳ�ʼ������ֵ����");
		return FALSE;
	}

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
	TRACE0("�ں˳�ʼ���ɹ���\n");
	m_nInit = 1;
	return TRUE;
}

//bool FileExists(CString fileName)
//{
//	DWORD       fileAttr;
//	fileAttr = GetFileAttributes(fileName);
//	if (0xFFFFFFFF == fileAttr && GetLastError() == ERROR_FILE_NOT_FOUND)
//		return false;
//	return true;
//}

void CCore::Uninit()
{
	ResetFan();
	if (m_hInstDLL != NULL)
	{
		FreeLibrary(m_hInstDLL);
		m_hInstDLL = NULL;
	}
	m_nInit = 0;
}
void CCore::Run() 
{
	static int nNextChecktTime = 0;
	static BOOL bSetPriority = FALSE;
	m_config.LoadConfig();
	//m_nInit = 2;
	//Sleep(3000);

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
		TRACE0("�ں˿�ʼ���С�\n");
		int curtime;
		while (!m_nExit)
		{
			curtime = GetTime();
			if (curtime >= nNextChecktTime || m_bForcedRefresh)
			{
				//MessageBox(NULL , "������...", "MyFunColtrol" , 0);
				if (m_ApmPowerStatusChange == 1)
				{
					CString cmdpath = "C:\\JohnsonProgram\\SetDisplayMode\\core\\SetDisplayMode.py";
					CString runcmdpath = "cmd /c python C:\\JohnsonProgram\\SetDisplayMode\\core\\SetDisplayMode.py";
					//bool ret = FileExists(cmdpath);
					DWORD fileAttr = GetFileAttributes(cmdpath);
					if (0xFFFFFFFF == fileAttr && GetLastError() == ERROR_FILE_NOT_FOUND)
					{
						LOG("runApmPowerStatusChange is not exists cmdpath");
					}
					else {
						//int result = system("cmd /k python C:\\JohnsonProgram\\SetDisplayMode\\core\\SetDisplayMode.py");
						int result = WinExec(runcmdpath, SW_SHOWMINIMIZED);
						// don't show cmd
						//int result = WinExec((runcmdpath),1);
						int resultLog = -1;
						LOG("runApmPowerStatusChange");
						LOG(resultLog = result);
					}
					m_ApmPowerStatusChange = 0;
				}

				Work();
				m_nLastUpdateTime = curtime;//����ʱ��
				nNextChecktTime = GetTime(NULL, m_config.UpdateInterval);//��һ������ʱ��
				m_bForcedRefresh = FALSE;
				if (!bSetPriority)
				{
					bSetPriority = TRUE;
					SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);//���״θ��³ɹ�������ø����ȼ�
				}
			}
			Sleep(1000);
		}
		TRACE0("�ں˽������С�\n");
	}
	m_nExit = 2;
}
void CCore::Work()
{
	//Update();
	//if (m_bForcedCooling)//ǿ����ȴ
	//{
	//	if (m_nCurTemp[0] >= m_config.ForceTemp || m_nCurTemp[1] >= m_config.ForceTemp)
	//	{
	//		if (m_nSetDuty[0] < 95 || m_nSetDuty[1] < 95)
	//		{
	//			m_nSetDuty[0] = 95;
	//			m_nSetDutyLevel[0] = 10;
	//			m_nSetDuty[1] = 95;
	//			m_nSetDutyLevel[1] = 10;
	//			SetFanDuty();
	//		}
	//		return;
	//	}
	//	else
	//		m_bForcedCooling = FALSE;
	//}
	//if (m_config.TakeOver)
	//{
	//	Control();
	//}
	//else
	//	ResetFan();
	m_GpuInfo.Update();
	//����GPUƵ��
	int limitClock = 0;
	int limitTime = m_config.timelimit;
	//int baseClockLimit = m_GpuInfo.m_nBaseClock;
	int baseClockLimit = 600;
	int lowClockLimit = 420;  //Lock 800 m_nGraphicsClock ==795
	float upClockRatio = 1.05;
	float downClockRatio = 0.97;
	int limit_overclock = 350;
	int resultLog = -1;
	//m_core.m_config.Linear ���Կ���
	int count_time = limitTime * m_config.UpdateInterval; //ʹ������*ʱ��ͳ��
	if (m_config.TakeOver)
	{
		if (m_TakeOverTimeOut == 0)
			m_TakeOverTimeOut = GetTime(NULL, count_time * 2);//��һ����ʱ
		//if (m_GpuInfo.m_nGPU_Temp < m_config.downTemplimit && m_GpuInfo.m_nGPU_Util < m_config.downClockPercent && m_GpuInfo.m_nGraphicsClock > m_GpuInfo.m_nBaseClock)
		if (m_GpuInfo.m_nGPU_Util > 35 && m_GpuInfo.m_nGPU_Util < m_config.downClockPercent && m_GpuInfo.m_nGraphicsClock > baseClockLimit)
		{
			//ռ����<88
			//m_GpuInfo.m_nGPU_UtilCount += m_GpuInfo.m_nGPU_Util;
			m_config.TakeOverDown += 1;
			int takeOvercurtime = GetTime();
			if (m_config.TakeOverDown >= limitTime || (takeOvercurtime > m_TakeOverTimeOut && m_config.TakeOverUp == 0 && m_config.TakeOverDown >= int(limitTime/2)) )
			{
				//limitClock = int(m_GpuInfo.m_nGraphicsClock * int(m_GpuInfo.m_nGPU_UtilCount /(m_config.TakeOverDown - 1)) / 100); //��ƵΪռ��������
				limitClock = int(m_GpuInfo.m_nGraphicsClock * downClockRatio);  //��Ƶ0.95

				if (limitClock >= baseClockLimit)
				{
					m_config.LockGPUFrequency = 1;
					m_config.GPUFrequency = limitClock;
				}
				m_config.TakeOverDown = 0;
				m_config.TakeOverUp = 0;
				m_config.TakeOverLock = 0;
				m_TakeOverTimeOut = 0 ;
			}
		}
		else
		{
			//if (m_GpuInfo.m_nGPU_Temp < m_config.upTemplimit && m_config.LockGPUFrequency > 0 && m_GpuInfo.m_nGPU_Util > m_config.upClockPercent && m_GpuInfo.m_nGraphicsClock > m_GpuInfo.m_nBaseClock)
			if (m_GpuInfo.m_nGPU_Temp >= m_config.downTemplimit && m_GpuInfo.m_nGraphicsClock > baseClockLimit)   //�жϵ�ǰ�¶�С�ڼ����¶�85�� > 75
			{
				//else
				//{
				//	m_config.LockGPUFrequency = 1;  //�¶ȹ��ߺ�Ƶ����
				//	limitClock = int(m_GpuInfo.m_nGraphicsClock * 0.95);
				//}
				limitClock = int(m_GpuInfo.m_nGraphicsClock * downClockRatio);
				limitClock = ((limitClock + 5) / 10) * 10;
				if (limitClock < baseClockLimit)
				{
					limitClock = baseClockLimit;
				}
				m_config.LockGPUFrequency = 1;  //�¶ȹ��ߺ�Ƶ����
				m_config.GPUFrequency = limitClock;

				m_config.TakeOverUp = 0;
				m_config.TakeOverDown = 0;
				m_config.TakeOverLock = 0;
				m_TakeOverTimeOut = 0;
			}
			else
			{
				if (m_config.LockGPUFrequency ^ 0)
					//���������
				{
					//if (m_GpuInfo.m_nGPU_Util > m_config.upClockPercent && m_GpuInfo.m_nGraphicsClock >= baseClockLimit)
					//	//ռ���ʳ�������97����Ƶand Ƶ�ʸ���1080
					if (m_GpuInfo.m_nGPU_Util > m_config.upClockPercent && m_GpuInfo.m_nGraphicsClock >= lowClockLimit)
						//ռ���ʳ�������97����Ƶand  Ƶ�ʸ��� lowClockLimit 600
					{
						m_config.TakeOverUp += 1;
						//int count_time = m_config.TakeOverUp * m_config.UpdateInterval;//ʹ������*ʱ��ͳ��


						//limitClock = m_GpuInfo.m_nGraphicsClock //��̬��Ƶ
						int takeOvercurtime = GetTime();
						if (m_config.TakeOverUp >= limitTime || (takeOvercurtime > m_TakeOverTimeOut &&  m_config.TakeOverDown==0 && m_config.TakeOverUp >= int(limitTime / 2)) )
						{
							if (m_GpuInfo.m_nGPU_Util >= m_config.downClockPercent && m_GpuInfo.m_nGPU_Temp <= m_config.upTemplimit - 10)  //ռ���ʴ���70 �е�ǰ�¶�С����Ƶ�¶�-10�� 
							{
								limitClock = int(m_GpuInfo.m_nGraphicsClock * 1.10);
								//limitClock = 0;
								//limitClock = m_config.GPU_LockClock;				//default LockClock bug 630 set 0
							}

							else if (m_GpuInfo.m_nGPU_Temp <= m_config.upTemplimit)  //�жϵ�ǰ�¶�С����Ƶ�¶�75�� 
							{
								limitClock = int(m_GpuInfo.m_nGraphicsClock * upClockRatio);
							}
							else if (m_GpuInfo.m_nGPU_Temp > m_config.upTemplimit && m_GpuInfo.m_nGPU_Temp <= m_config.downTemplimit)  //�жϵ�ǰ�¶ȴ�����Ƶ�¶�75�� С��82ά�ֲ���
							{
								limitClock = m_GpuInfo.m_nGraphicsClock;
							}
							else
							{
								limitClock = int(m_GpuInfo.m_nGraphicsClock * downClockRatio);  //��Ƶ���¶ȸ���82��Ƶ
								if (limitClock < baseClockLimit)
								{
									limitClock = baseClockLimit;
								}

							}
							//if (limitClock < m_GpuInfo.m_nStandardFrequency || limitClock < 1600)
							limitClock = ((limitClock + 5) / 10) * 10;

							//if (limitClock >= baseClockLimit && limitClock < m_config.upClocklimit)
							if (limitClock == 0 || (limitClock >= lowClockLimit && limitClock < m_config.upClocklimit))

							{
								m_config.GPUFrequency = limitClock;
							}
							else
							{
								if (limitClock >= m_config.upClocklimit && m_GpuInfo.m_nGPU_Temp < m_config.upTemplimit)  //limitClock > 1600 ,�¶�С��75�ſ�����
								{
									//m_config.LockGPUFrequency = 0;
									//bug when limitClock == upClocklimit will to LockClock
									//m_config.GPUFrequency = m_config.GPU_LockClock;  //�ָ�init LockClock BUG
									m_config.GPUFrequency = m_config.upClocklimit;   //set GPU to upClocklimit
								}
							}
							m_config.TakeOverUp = 0;
							m_config.TakeOverDown = 0;
							m_config.TakeOverLock = 0;
							m_TakeOverTimeOut = 0;
						}
					}
					else
					{
						if (m_GpuInfo.m_nGPU_Util < 35 && m_GpuInfo.m_nGPU_Temp < m_config.upTemplimit)
							//������� Util <35 and �¶�С������ �ſ����� 
						{
							m_config.TakeOverLock += 1;
							if (m_config.TakeOverLock >= limitTime)
								//����Times
							{
								//m_config.LockGPUFrequency = 0;  //���ÿ��е��º�ſ�����
								//m_config.GPUFrequency = 0;
								m_config.GPUFrequency = m_config.GPU_LockClock;  //�ָ�init LockClock
								m_config.TakeOverUp = 0;
								m_config.TakeOverDown = 0;
								m_config.TakeOverLock = 0;

							}
						}


					}
				}
			}
		}
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
						if (freq_over_clock == freq_over_clock_limit || (freq_over_clock_limit > m_config.OverClock2) || ((freq_over_clock > m_config.OverClock2) && (freq_over_clock_limit < m_config.OverClock2)))
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
						int freq_over_clock = frequencyDeltaKHz_t[0] / 500;
						if (freq_over_clock != m_config.GPUOverClock)
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

						}
						else
						{
							if (m_start_overclock == 0)
							{
								//m_GpuInfo.m_nOverClock = m_config.GPUOverClock;
								if (m_TakeOverTimeOut == 0)
								{
									LOG(resultLog = m_TakeOverTimeOut);
									m_TakeOverTimeOut = GetTime(NULL, 30);//��һ����ʱ
								}
								//LOG(resultLog = freq_over_clock);
								//LOG(resultLog = m_config.GPUOverClock);
							}
							int overclockcurtime = GetTime();
							if (overclockcurtime > m_TakeOverTimeOut && m_start_overclock == 0) 
							{

								m_start_overclock = 1;
								m_TakeOverTimeOut = 0;
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
			if (m_config.TakeOver)
				m_GpuInfo.LockFrequency(m_config.GPUFrequency);
			else
				m_GpuInfo.LockFrequency(m_config.GPU_LockClock);
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
			//AfxMessageBox("��ȡ�¶�����");
			//�¶Ȼ�ȡ������������һ��
			if (TempErr++ == 0)
			{
				Sleep(1000);
				i--;
				continue;//����һ��
			}
		}
		this->m_nLastTemp[i] = this->m_nCurTemp[i];
		this->m_nCurTemp[i] = data.Remote;
		this->m_nCurDuty[i] = int(data.FanDuty * 100 / 255.0 + 0.5);

		if (m_bUpdateRPM)//��ȡ����ת��
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
	//�趨ת��
	SetFanDuty();

}
void CCore::CalcLinearDuty()
{
	static int nLastTemp[2] = { 0, 0 };//ÿ�����ڼ���ת�ٵ��¶�

	//int duty,dl;
	//int j;
	//for (int i = 0; i < 2; i++)
	//{
	//	nLastTemp[i] = max(nLastTemp[i], m_nCurTemp[i]);//�¶�����ʱ�����Ե�ǰ�¶ȼ���ת��
	//	nLastTemp[i] = min(nLastTemp[i], m_nCurTemp[i] + m_config.TransitionTemp);//�¶��½�ʱ�Ե�ǰ�¶�+�����¶�������ת��

	//	j = nLastTemp[i];//����ת��ʹ�õ��¶�

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
		last_dl = m_nSetDutyLevel[i];//��һ�εĸ��صȼ�
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
				//������һ�εĸ��ص�λ����
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
	//������߱仯
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
			m_pfnSetFanDuty(i + 2, duty);//������ڵ�3������
	}
	m_bTakeOverStatus = TRUE;
}