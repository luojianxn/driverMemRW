// testMem2.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "windows.h"

int _tmain(int argc, _TCHAR* argv[])
{
	PVOID addr = malloc(4);
	memcpy(addr, "abc", 4);
	printf("%x", addr);
	system("PAUSE");
	return 0;
}

