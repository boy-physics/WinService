#include "Service.h"

// Purpose:
//   Entry point for the process
//
// Parameters:
//   None
//
// Return value:
//   None, defaults to 0 (zero)

// Main function
int _tmain(int argc, TCHAR *argv[])
{
    // If command-line parameter is "install", install the service
    // else if command-line parameter is "remove", remove the service
    // else, the service is probably being started by the SCM

    if (lstrcmpi(argv[1], TEXT("install")) == 0)
    {
        SvcInstall();
        return 0;
    }
    else if (lstrcmpi(argv[1], TEXT("remove")) == 0)
    {
        // TO DO: Add code to remove the service
        return 0;
    }

    // TO DO: Add any additional services for the process to this table
    SERVICE_TABLE_ENTRY DispatchTable[] =
        {
            {SvcName, (LPSERVICE_MAIN_FUNCTION)SvcMain},
            {NULL, NULL}};

    // This call returns when the service has stopped
    // the process should simply terminate when the call returns
    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        SvcReportEvent(TEXT("StartServiceCtrlDispatcher"));
    }

    return 0;
}