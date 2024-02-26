#include "Threadworker.h"

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    // TO_DO: Declare and initialize variables.

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(ghSvcStopEvent, 0) != WAIT_OBJECT_0)
    {
        // TO_DO: Perform main service function here
        FILE *file = fopen("C:\\Users\\M1\\Desktop\\output.txt", "a");
        if (file != NULL)
        {
            fprintf(file, "hello world\n");
            fclose(file);
        }

        //  Simulate some work by sleeping
        Sleep(3000);
    }

    SvcReportEvent(TEXT("ServiceWorkerThread exitting"));
    return ERROR_SUCCESS;
}
