/*
 * Copyright 2010, BurnItNow Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _WorkerThread_H_
#define _WorkerThread_H_


#include "ObjectList.h"

#include <Invoker.h>
#include <Locker.h>
#include <String.h>


class WorkerThread : public BLocker {
public:
					WorkerThread(BObjectList<BString>* argList = NULL,
						BInvoker* invoker = NULL);
	virtual 		~WorkerThread();

	BObjectList<BString>* Arguments();
	void 			SetArguments(BObjectList<BString>* argList);
	WorkerThread*	AddArgument(const char* argument);

	BInvoker* 		Invoker();
	void 			SetInvoker(BInvoker* invoker);

	status_t 		Run();
	status_t 		Stop();
	status_t 		Wait();
	bool 			IsRunning();

private:
	static int32 	_Thread(void* data);
	static void 	_ThreadExit(void* data);

	BObjectList<BString>* fArgumentList;
	BInvoker* 		fInvoker;
	thread_id 		fThread;
};


#endif	// _WorkerThread_H_
