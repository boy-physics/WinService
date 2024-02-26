#ifndef THREADWORKER_H
#define THREADWORKER_H

#include <windows.h>
#include <stdio.h>
#include "Service.h"

// Function declarations for the thread
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#endif // ! THREADWORKER_H