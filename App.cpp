/*
 * Copyright 2011-2015. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "App.h"
#include "MainWindow.h"

#include <Alert.h>
#include <Catalog.h>
#include <Font.h>
#include <TextView.h>

#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"

extern const char *kApplicationSignature = "application/x-vnd.humdinger-ubertuber";


App::App()
	:
	BApplication("application/x-vnd.UberTuber"),
	fSavedRefsReceived(NULL),
	fURLReceived(NULL)	
{
	fMainWindow = new MainWindow();
	fMainWindow->Show();
}


App::~App()
{
	delete fSavedRefsReceived;
	delete fURLReceived;
}


void
App::ArgvReceived(int32 argc, char** argv)
{
	if (strcmp(argv[1], "--help") == 0
		|| strcmp(argv[1], "-h") == 0) {
		printf(B_TRANSLATE("UberTuber can be launched with either a URL or a "
			"Web+ Bookmark file.\n"));
		PostMessage(B_QUIT_REQUESTED);
		return;
	}

	BMessage refsReceived(B_REFS_RECEIVED);
	BEntry entry(argv[1], true);
	entry_ref ref;
	if (entry.GetRef(&ref) == B_OK)
		refsReceived.AddRef("refs", &ref);

	if (refsReceived.HasRef("refs"))
		PostMessage(&refsReceived);
	else
		fURLReceived = new BString(argv[1]);
}


void
App::RefsReceived(BMessage* message)
{
	if (!message->HasRef("refs") && message->HasRef("dir_ref")) {
		entry_ref dirRef;
		if (message->FindRef("dir_ref", &dirRef) == B_OK)
			message->AddRef("refs", &dirRef);
	}

	if (fMainWindow == NULL) {
		// ReadyToRun() has not been called yet, this happens when someone
		// launches us with a B_REFS_RECEIVED message.
		delete fSavedRefsReceived;
		fSavedRefsReceived = new BMessage(*message);
	} else
		fMainWindow->PostMessage(message);
}


void
App::MessageReceived(BMessage* message)
{
	switch (message->what)
	{
		case statBUFFER:
		case statDOWNLOAD:
		case statERROR:
		case statFINISH_GET:
		case statFINISH_PLAY:
		case statFINISH_SAVE:
		case statPLAYING:
		{
			fMainWindow->PostMessage(DetachCurrentMessage());
			break;
		}
		default:
		{
			BApplication::MessageReceived(message);
			break;
		}
	}
}


void
App::ReadyToRun()
{	
	if (fSavedRefsReceived) {
		// RefsReceived() was called earlier than ReadyToRun()
		fMainWindow->PostMessage(fSavedRefsReceived);
		delete fSavedRefsReceived;
		fSavedRefsReceived = NULL;
	}
	if (fURLReceived){
		fMainWindow->LockLooper();
		fMainWindow->SetURL(fURLReceived);
		fMainWindow->UnlockLooper();
	}
}


void
App::AboutRequested()
{
	BAlert* alert = new BAlert("about", B_TRANSLATE("UberTuber v0.9.11\n"
		"\twritten by Humdinger\n"
		"\tbased on ideas of Leszek's YAVTD\n"
		"\tCopyright 2011-2015\n\n"
		"UberTuber uses a script to download YouTube videos.\n"
		"Clips can be saved or played back directly. UberTuber monitors "
		"the system clipboard for newly copied URLs and has an 'Auto-play' "
		"option that'll start playback as soon as a supported URL is detected.\n"),
		B_TRANSLATE("Thank you"));
	BTextView* view = alert->TextView();
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
	App* app = new App();
	app->Run();
	delete app;
	return 0;
}
