/*
 * Copyright 2014. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 * based on ideas of YAVTD for Haiku
 * Version 1.0 by Leszek Lesner (C) 2011
 */

#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <Looper.h>
#include <Messenger.h>
#include <String.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

enum {
	msgGETCLIP	= 'gclp',
	msgGETTITLE	= 'gtit',
	msgTITLE	= 'titl'
};


class WorkerThread : public BLooper {
public:
						WorkerThread(const BMessenger& owner);
		void			MessageReceived(BMessage* message);
		
		BString			_GetTitle(BString url);
		bool			_GetClip(BString url);
private:
		BMessenger		fOwner;
};

#endif
