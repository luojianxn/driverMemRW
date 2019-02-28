// testMem1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
#define DEVICE_LINK_NAME    L"\\\\.\\BufferedIODevcieLinkName"
#define CTL_SYS \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x830, METHOD_BUFFERED, FILE_ANY_ACCESS)

//测试驱动读写
int _tmain(int argc, _TCHAR* argv[])
{
	
	HANDLE DeviceHandle = CreateFile(DEVICE_LINK_NAME,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (DeviceHandle == INVALID_HANDLE_VALUE)
	{   
		printf("CreateFile error");
		system("PAUSE ");
		return 0;
	}
	char BufferData = NULL;
	DWORD ReturnLength = 0;
	
	BOOL IsOk = DeviceIoControl(DeviceHandle, CTL_SYS,"Ring3->Ring0",strlen("Ring3->Ring0") + 1,(LPVOID)BufferData,0,&ReturnLength,NULL);
	if (IsOk == FALSE)
	{
		int LastError = GetLastError();

		if (LastError == ERROR_NO_SYSTEM_RESOURCES)
		{
			char BufferData[MAX_PATH] = { 0 };
			IsOk = DeviceIoControl(DeviceHandle, CTL_SYS,
				"Ring3->Ring0",
				strlen("Ring3->Ring0") + 1,
				(LPVOID)BufferData,
				MAX_PATH,
				&ReturnLength,
				NULL);

			if (IsOk == TRUE)
			{
				printf("%s\r\n", BufferData);
			}
		}
	}
	if (DeviceHandle != NULL)
	{
		CloseHandle(DeviceHandle);
		DeviceHandle = NULL;
	}
	printf("Input AnyKey To Exit\r\n");
	system("PAUSE ");
	return 0;
}

