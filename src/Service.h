#ifndef SERVICE_H
#define SERVICE_H

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>

// Create the global variables
SERVICE_STATUS gSvcStatus;
SERVICE_STATUS_HANDLE gSvcStatusHandle;
extern HANDLE ghSvcStopEvent;
extern HANDLE ghSvcThread;

#define SvcName _T("My Sample Service")

// Function prototypes
VOID SvcInstall(void);
VOID WINAPI SvcCtrlhandler(DWORD);
VOID WINAPI SvcMain(DWORD argc, LPTSTR *argv);

VOID WINAPI ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR *);
VOID SvcReportEvent(LPTSTR);
DWORD ServiceWorkerThread(LPVOID lpParam);

#endif // ! SERVICE_H
