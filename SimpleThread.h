/********************************************************************************
 Copyright (C) 2014 Append Huang <Append@gmail.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/

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
		onDebugMsg(L"Before StartThread(): Thread %d",(int)thread);
		if(thread==0) {
			onDebugMsg(L"Begin StartThread(): Thread %d",(int)thread);
			bKillThread = false;
			thread = CreateThread(NULL, 0, ThreadProc, (LPVOID)this, 0, NULL);
		}
	}
		
	void StopThread () {
		bKillThread = true;
		onDebugMsg(L"Before StopThread(): Thread %d",(int)thread);
		if(thread!=0) {
			onDebugMsg(L"Begin StopThread(): Thread %d",(int)thread);
			DWORD retv;

			retv=WaitForSingleObject(thread,30000);
			switch(retv){
				case WAIT_TIMEOUT://Failed waiting//Extremely Case
				case WAIT_ABANDONED:
					if(retv==WAIT_TIMEOUT) onDebugMsg(L"Thread Closing: TIMEOUT, FORCE TERMINATING THE THREAD");
					if(retv==WAIT_ABANDONED) onDebugMsg(L"Thread Closing: ABANDONED, FORCE TERMINATING THE THREAD");
					onDebugMsg(L"Thread Closing: Might Lead to Some UNSTABLITY");
					DWORD exitcode;
					GetExitCodeThread(thread,&exitcode);
					TerminateThread(thread,exitcode);
					break;
				case WAIT_FAILED:
					if(thread==0) onDebugMsg(L"Thread Closing: Already Closed, thread=%d",thread);
					else onDebugMsg(L"Thread Closing: FAILED, thread=%d",thread);
					break;
				case WAIT_OBJECT_0:
					onDebugMsg(L"Thread Closing: Finished");
			}
			onDebugMsg(L"Thread Closed: RETV = 0x%08X",retv);
			if(thread!=0) {
				CloseHandle(thread);
				thread = 0;
			}
		}
	}
};


