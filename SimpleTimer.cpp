#include "SimpleThread.h"
#include "SimpleTimer.h"

DWORD SimpleTimer::Run()
{
	LARGE_INTEGER dueTime;
	if((timer = CreateWaitableTimer(NULL, FALSE, NULL)) == NULL)
		return -1;
	dueTime.QuadPart=0;
	SetWaitableTimer(timer, &dueTime, ticktime, NULL, NULL, FALSE);
	HANDLE handles[] = { timer, stopReq };

	while(!bKillThread) { //Do Things in this part using TickFunc
		TickFunc();
		WaitForMultipleObjects(2, handles, FALSE, INFINITE);
	}
	CloseHandle(timer);
	return 0;
}