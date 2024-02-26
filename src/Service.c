#include "Service.h"

HANDLE ghSvcStopEvent = NULL;
HANDLE ghSvcThread = NULL;

// Purpose:
//   Installs the service in the SCM database
//
// Parameters:
//   None
//
// Return value:
//   None

VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];

    if (!GetModuleFileName(NULL, szPath, MAX_PATH))
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the SCM database
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service
    schService = CreateService(
        schSCManager,                     // SCM database
        SvcName,                          // name of service
        SvcName,                          // service name to display
        SERVICE_ALL_ACCESS | GENERIC_ALL, // desired access
        SERVICE_WIN32_OWN_PROCESS,        // service type
        SERVICE_AUTO_START,               // start type
        SERVICE_ERROR_NORMAL,             // error control type
        szPath,                           // path to service's binary
        NULL,                             // no load ordering group
        NULL,                             // no tag identifier
        NULL,                             // no dependencies
        NULL,                             // LocalSystem account
        NULL);                            // no password

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else
    {
        printf("Service installed successfully\n");
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

// Purpose:
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//		the service and subsequent strings are passed by the process
//		that called the StartService function to start the service.
//
// Return value:
//   None.

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    // Register the handler function for the service
    gSvcStatusHandle = RegisterServiceCtrlHandler(SvcName, SvcCtrlhandler);

    if (!gSvcStatusHandle)
    {
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    // These SERVICE_STATUS members remain as set here
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    // Report initial status to the SCM
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work
    SvcInit(dwArgc, lpszArgv);
}

// Purpose:
//   The service control handler function is called when the
//   SCM sends a control code to the service.
//
// Parameters:
//   dwCtrl - control code
//
// Return value:
//   None

VOID WINAPI SvcCtrlhandler(DWORD dwCtrl)
{
    // Handle the requested control code
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop
        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

        return;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }
}

// Purpose:
//   Sets the current service status and reports it to the SCM.
//  If the dwCheckPoint and dwWaitHint members of the SERVICE_STATUS
//  structure are not zero, the dwServiceStatus member of the
//  SERVICE_STATUS structure is set to SERVICE_START_PENDING and
//  the function returns immediately.  This signals the SCM to not
//  wait for the service to start before reporting to the user.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, in milliseconds
//
// Return value:
//   None

VOID WINAPI ReportSvcStatus(DWORD dwCurrentState,
                            DWORD dwWin32ExitCode,
                            DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure
    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else
        gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

// Purpose:
//   Performs actual initialization of the service
//
// Parameters:
//   dwArgc - Number of arguments in lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//		the service and subsequent strings are passed by the process
//		that called the StartService function to start the service.
//
// Return value:
//   None

VOID SvcInit(DWORD dwArgc, LPTSTR *lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.
    ghSvcStopEvent = CreateEvent(
        NULL,  // default security attributes
        TRUE,  // manual reset event
        FALSE, // not signaled
        NULL); // no name

    if (ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    // Report running status when initialization is complete
    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    ghSvcThread = CreateThread(
        NULL,                                        // default security attributes
        0,                                           // default stack size
        (LPTHREAD_START_ROUTINE)ServiceWorkerThread, // thread function
        NULL,                                        // no thread parameter
        0,                                           // default creation flags
        NULL);                                       // receive thread identifier

    if (ghSvcThread == NULL)
    {
        // Handle thread creation error
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    // Clean up the system resources
    WaitForSingleObject(ghSvcStopEvent, INFINITE);
    CloseHandle(ghSvcThread);
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

// Purpose:
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
//
// Return value:
//   None

VOID SvcReportEvent(LPTSTR szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SvcName);

    if (NULL != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SvcName;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    0,                   // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}