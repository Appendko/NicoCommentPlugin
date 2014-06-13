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