/*
 * Copyright 2011-2013. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "App.h"
#include "MainWindow.h"

#include <Alert.h>
#include <Font.h>
#include <TextView.h>


extern const char *kApplicationSignature = "application/x-vnd.UberTuber";


App::App()
	:	BApplication("application/x-vnd.UberTuber")
{
	fMainWindow = new MainWindow();
	fMainWindow->Show();
}


void
App::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case statBUFFER:
		case statDOWNLOAD:
		case statERROR:
		case statFINISH_GET:
		case statFINISH_PLAY:
		case statFINISH_SAVE:
		case statPLAYING:
		{
			msg->PrintToStream();
			fMainWindow->PostMessage(DetachCurrentMessage());
			break;
		}
		default:
		{
			BApplication::MessageReceived(msg);
			break;
		}
	}
}


void
App::AboutRequested()
{
	BAlert *alert = new BAlert("about", "UberTuber   v0.9.8\n"
		"\twritten by Humdinger\n"
		"\tbased on ideas of Leszek's YAVTD\n"
		"\tCopyright 2011-2013\n\n"
		"UberTuber uses a script to download YouTube videos.\n"
		"Clips can be saved or played back directly. UberTuber monitors "
		"the system clipboard for newly copied URLs and has an 'Auto-play' "
		"option that'll start playback as soon as a YouTube URL is detected.\n",
		"Thank you");
	BTextView *view = alert->TextView();
	BFont font;

	view->SetStylable(true);
	view->GetFont(&font);
	font.SetSize(font.Size()+4);
	font.SetFace(B_BOLD_FACE);
	view->SetFontAndColor(0, 9, &font);
	alert->Go();
}

int
main()
{
	App *app = new App();
	app->Run();
	delete app;
	return 0;
}
