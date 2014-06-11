#include <Windows.h>
#pragma once

class SimpleThread{
protected:
	HANDLE thread;
	HANDLE timer;
	HANDLE stopReq;
	bool bKillThread;

	virtual DWORD Run() {return 0;}
	static DWORD WINAPI ThreadProc(LPVOID pParam){ return static_cast<SimpleThread*>(pParam)->Run(); }
	
public:	
	inline bool isAlive() {return (thread!=0);}
	SimpleThread() {
		thread = 0;
		bKillThread = true;
		stopReq = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	~SimpleThread() {
		StopThread();
		CloseHandle(stopReq);
	}
	
	void StartThread() {
		if(thread==0) {
			bKillThread = false;
			thread = CreateThread(NULL, 0, ThreadProc, (LPVOID)this, 0, NULL);
		}
	}
		
	void StopThread () {
		if(thread!=0) {
			bKillThread = true;
			DWORD retv;
			MSG msg;
			HANDLE handles[] = {thread};
			SetEvent(stopReq);
			//process messages while waiting for the thread to complete, otherwise the thread will be locked forever
			while((retv = MsgWaitForMultipleObjects(1, handles, FALSE, INFINITE, QS_ALLINPUT)) != WAIT_FAILED) {
				if(retv == WAIT_OBJECT_0) break;
				else if(retv == WAIT_OBJECT_0+1)
					while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
			}
			ResetEvent(stopReq);
			if(thread!=0) {
				CloseHandle(thread);
				thread = 0;
			}
		}
	}
};


