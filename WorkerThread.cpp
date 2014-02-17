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

#include "WorkerThread.h"


WorkerThread::WorkerThread(const BMessenger& owner)
	:
	BLooper("title_getter"),
	fOwner(owner)
{
	Run();
}


void
WorkerThread::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case msgGETTITLE:
		{
			BString url;
			if (message->FindString("url", &url) == B_OK) {
				BMessage msg(msgTITLE);
				msg.AddString("title", _GetTitle(url));
				fOwner.SendMessage(&msg);
			}
			break;
		}
		case msgGETCLIP:
		{
			BString url;
			if (message->FindString("url", &url) == B_OK)
				_GetClip(url);
			break;
		}
		default:
			BLooper::MessageReceived(message);
	}
}


BString
WorkerThread::_GetTitle(BString url)
{
	BString command("youtube-dl --get-title %URL% 2>&1 | tail -n 1"); // also get output from error out
	command.ReplaceAll("%URL%", url.String());

	char title[1000];
	FILE* pget_title;

	pget_title = popen(command.String(),"r");
	fread(title, 1, sizeof(title), pget_title);
	fclose(pget_title);

	/* strip trailing newline */
	for (int i = 0; (unsigned) i < strlen(title); i++) {
		if (title[i] == '\n' || title[i] == '\r' )
			title[i] = '\0';
	}
	return title;
}


bool
WorkerThread::_GetClip(BString url)
{
	BString* command = new BString(
	"mkdir -p /tmp/ubertuber ; "
	"cd /tmp/ubertuber ; "
	"hey application/x-vnd.UberTuber down ; "
	"youtube-dl --max-quality=22 --continue --restrict-filenames --no-cache-dir %URL% ; "
	"while [ -n \"$(%TEST%)\" ] ; do " // wait for script to finish/aborted
	"sleep 2 ; "
	"done ; "
	"FILE=$(youtube-dl --restrict-filenames --get-filename %URL% 2>&1 | tail -n 1) ; "
	"addattr -t string META:url %URL% \"$FILE\" ; "
	"if [ -e \"$FILE\" ] ; then "
	"hey application/x-vnd.UberTuber gfin ; "
	"else hey application/x-vnd.UberTuber erro ; "
	"fi ; "
	"exit");

	BString threadtest("ps | grep python | grep youtube-dl");

	command->ReplaceAll("%TEST%", threadtest.String());
	command->ReplaceAll("%URL%", url.String());

	system(command->String());
	return true;
}
