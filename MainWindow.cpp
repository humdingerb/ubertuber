/*
 * Copyright 2011-2014. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 * based on ideas of YAVTD for Haiku
 * Version 1.0 by Leszek Lesner (C) 2011
 */


#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Clipboard.h>
#include <ControlLook.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <Roster.h>
#include <Screen.h>
#include <Size.h>
#include <String.h>
#include <SupportDefs.h>
#include <View.h>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>


BString validAddress[] = {
	"youtube.com/watch", "youtube.com/embed", "youtu.be", "youtube-nocookie.com",
	"metacafe.com/watch", "dailymotion.com", "video.google.com", "photobucket.com", 
	"video.yahoo.com", "depositfiles.com", "facebook.com/video", "vimeo.com",
	"blip.tv", "escapistmagazine.com/videos/view",
	0
};


int32
_call_script(void* arg)
{
	BString* command = static_cast<BString *>(arg);
	system(command->String());
	return 0;
}


MainWindow::MainWindow()
	:
	BWindow(BRect(), "UberTuber", B_TITLED_WINDOW,
		B_NOT_V_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS |
		B_QUIT_ON_WINDOW_CLOSE | B_AUTO_UPDATE_SIZE_LIMITS),
	fAbortedFlag(false),
	fGetFlag(false),
	fGotClipFlag(false),
	fPlayedFlag(false),
	fPlayingFlag(false),
	fSavedFlag(false),
	fSaveIt(false),
	fWorkerThread(new WorkerThread(this))
{
	_BuildMenu();
	_BuildLayout();

	fURLBox->MakeFocus(true);
	fPlayButton->MakeDefault(true);

	if (fSettings.WindowPosition() == BRect(-1, -1, -1, -1)) {
		CenterOnScreen();
		ResizeTo(400, 50);
	} else {
		BRect frame = fSettings.WindowPosition();
		MoveTo(frame.LeftTop());
		ResizeTo(frame.right - frame.left, frame.bottom - frame.top);

		// make sure window is on screen
		BScreen screen(this);
		if (!screen.Frame().InsetByCopy(10, 10).Intersects(Frame()))
			CenterOnScreen();
	}

	BEntry entry(fSettings.LastDir().Path(), true);
	entry_ref destRef;
	if (entry.Exists() && entry.IsDirectory())
		entry.GetRef(&destRef);

	fSavePanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), &destRef,
		B_DIRECTORY_NODE);

	if (fSavePanel->Window()->Lock()) {
		fSavePanel->Window()->SetTitle("Choose destination folder");
		fSavePanel->SetButtonLabel(B_DEFAULT_BUTTON, "Save");
		fSavePanel->Window()->Unlock();
	}

	be_clipboard->StartWatching(this);
	PostMessage(B_CLIPBOARD_CHANGED);

	fURLBox->SetText("");
}


void
MainWindow::_BuildMenu()
{
	BMenu* menu;
	BMenuItem* item;

	fMenuBar = new BMenuBar(BRect(), "menubar");

	menu = new BMenu("File");
	menu->AddItem(fPlayMenu = new BMenuItem("Play", new BMessage(msgPLAY), 'P'));
	fPlayMenu->SetEnabled(false);
	menu->AddItem(fSaveMenu = new BMenuItem("Save as" B_UTF8_ELLIPSIS,
		new BMessage(msgSAVE), 'S'));
	fSaveMenu->SetEnabled(false);

	menu->AddItem(fAbortMenu = new BMenuItem("Abort download",
		new BMessage(msgABORT)));
	fAbortMenu->SetEnabled(false);

	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("About UberTuber",
		new BMessage(B_ABOUT_REQUESTED)));
	item->SetTarget(be_app);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED),
		'Q'));
	fMenuBar->AddItem(menu);

	menu = new BMenu("Settings");
	menu->AddItem(item = fAutoMenu = new BMenuItem("Auto-play",
		new BMessage(msgAUTO)));
	item->SetMarked(fSettings.StateAuto());
	menu->AddItem(item = fClearMenu = new BMenuItem(
		"Remove temporary files on quit", new BMessage(msgCLEAR)));
	item->SetMarked(fSettings.StateClear());
	fMenuBar->AddItem(menu);

//	The future History menu:
//	menu = new BMenu("History");
//	menu->AddItem(item = fHistoryMenu = new BMenuItem("Activate history",
//		new BMessage(msgHISTORY)));
//	item->SetMarked(fSettings.StateHistory());
//	menu->AddItem(item = new BMenuItem("Clear History",
//		new BMessage(msgCLEARHIST)));
//	menu->AddSeparatorItem();
//
//	fMenuBar->AddItem(menu);
}


void
MainWindow::_BuildLayout()
{
	fURLBox = new BTextControl("urlbox", "URL:", " ", NULL);
	fURLBox->SetModificationMessage(new BMessage(msgURL));

	fTitleView = new BStringView("title", " ");
	fTitleView->SetFontSize(be_plain_font->Size() - 2);
	fTitleView->SetHighColor(tint_color(ui_color(B_CONTROL_TEXT_COLOR), 0.7));

	fStatusView = new BStringView("status", " ");
	fStatusView->SetFontSize(be_plain_font->Size() - 2);
	fStatusView->SetHighColor(tint_color(ui_color(B_CONTROL_TEXT_COLOR), 0.7));

	fAbortButton = new BButton("abortbutton", "Abort download",
		new BMessage(msgABORT));
	fAbortButton->SetEnabled(false);

	fPlayButton = new BButton("playbutton", "Play", new BMessage(msgPLAY));
	fPlayButton->SetEnabled(false);

	fSaveButton = new BButton("savebutton", "Save as" B_UTF8_ELLIPSIS,
		new BMessage(msgSAVE));
	fSaveButton->SetEnabled(false);

	BFont font;
	float spacing = be_control_look->DefaultItemSpacing();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMenuBar)
		.AddGroup(B_HORIZONTAL, spacing)
			.Add(fURLBox)
			.SetInsets(spacing, spacing, spacing, 0)
		.End()
		.AddGroup(B_HORIZONTAL, 0)
			.AddStrut(font.StringWidth("URL:") + be_control_look->DefaultLabelSpacing())
			.Add(fTitleView)
			.AddStrut(spacing)
			.AddGlue()
			.Add(fStatusView)
			.SetInsets(spacing, 0, spacing, 0)
		.End()
		.AddGroup(B_HORIZONTAL, 0)
			.AddStrut(font.StringWidth("URL:") + be_control_look->DefaultLabelSpacing())
			.Add(fAbortButton)
			.AddStrut(spacing * 4)
			.AddGlue()
			.Add(fSaveButton)
			.AddStrut(spacing)
			.Add(fPlayButton)
			.SetInsets(spacing, 0, spacing, spacing / 2)
		.End();
}


void
MainWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case B_CLIPBOARD_CHANGED:
		{
			BString clipboardString(GetClipboard());
			BString urlString(fURLBox->Text());

			if (!urlString.ICompare(clipboardString)) {
				printf("Same contents: break!\n");
				break;
			}

			if (ValidURL(clipboardString) == false) {
				printf("Not valid: break!\n");
				break;
			}

			fURLBox->SetText(clipboardString.String());
			SetStatus("Auto-inserted clipboard URL");

			if (!fGetFlag) {
				fPlayButton->SetEnabled(true);
				fSaveButton->SetEnabled(true);
				fPlayMenu->SetEnabled(true);
				fSaveMenu->SetEnabled(true);

				ResetFlags();

				if (fSettings.StateAuto())
					PostMessage(msgPLAY);
			}
			break;
		}
		case msgAUTO:
		{
			fSettings.SetStateAuto(!fSettings.StateAuto());
			fAutoMenu->SetMarked(fSettings.StateAuto());
			break;
		}
		case msgCLEAR:
		{
			fSettings.SetStateClear(!fSettings.StateClear());
			fClearMenu->SetMarked(fSettings.StateClear());
			break;
		}
		case msgCLEARHIST:
		{
			printf("Clear history!\n");
			break;
		}
		case msgHISTORY:
		{
			fSettings.SetStateHistory(!fSettings.StateHistory());
			fHistoryMenu->SetMarked(fSettings.StateHistory());
			break;

		}
		case msgABORT:
		{
			KillThread();

			SetStatus("Aborted");
			printf("Download aborted\n");

			fAbortMenu->SetEnabled(false);
			fAbortButton->SetEnabled(false);

			fPlayButton->SetEnabled(true);
			fSaveButton->SetEnabled(true);
			fPlayMenu->SetEnabled(true);
			fSaveMenu->SetEnabled(true);
			fURLBox->SetEnabled(true);
			ResetFlags();
			fAbortedFlag = true;
			break;
		}
		case msgPLAY:
		{
			fURL = new BString(fURLBox->Text());
			
			if (!fGetFlag && !fGotClipFlag)
				GetClip();

			PlayClip();
			fPlayButton->SetEnabled(false);
			fPlayMenu->SetEnabled(false);
			break;
		}
		case msgSAVE:
		{
			fURL = new BString(fURLBox->Text());
			fSavePanel->Show();
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref fileref;
			msg->FindRef("refs", 0, &fileref);
			BPath path(new BEntry(&fileref));

			fSaveDir = new BString(path.Path());
			printf("Save path: %s\n", path.Path());

	 	 	if (fGotClipFlag)
	 	 		SaveClip();
	 	 	else if (!fGetFlag && !fGotClipFlag) {
	 	 		fSaveIt = true;
	 	 	 	GetClip();
	 	 	} else
	 	 		fSaveIt = true;

			fSaveButton->SetEnabled(false);
			fSaveMenu->SetEnabled(false);
			fSettings.SetLastDir(path);
			break;
		}
		case msgURL:
		{
			if (!fGetFlag) {
				int state = (fURLBox->TextView()->TextLength() > 0) ? true : false;
				fPlayButton->SetEnabled(state);
				fSaveButton->SetEnabled(state);
				fPlayMenu->SetEnabled(state);
				fSaveMenu->SetEnabled(state);

				SetStatus("");
				fTitleView->SetText("");
				printf("URL changed:\n");

				ResetFlags();
			}
			break;
		}
		case msgTITLE:
		{
			if (msg->FindString("title", &fClipTitle) == B_OK)
				TruncateTitle();
			break;
		}

		// React to messages from 'hey', forwarded from App
		case statBUFFER:
		{
			SetStatus("Buffering...");
			break;
		}
		case statDOWNLOAD:
		{
			SetStatus("Downloading...");
			break;
		}
		case statERROR:
		{
			if (!fAbortedFlag)
				SetStatus("Download failed");
			else {
				SetStatus("Aborted");
				fAbortMenu->SetEnabled(false);
				fAbortButton->SetEnabled(false);
			}
			fURLBox->SetEnabled(true);
			ResetFlags();
			break;
		}
		case statFINISH_GET:
		{
			if (!fPlayingFlag)
				SetStatus("Download finished");
			fAbortButton->SetEnabled(false);
			fAbortMenu->SetEnabled(false);
			fGetFlag = false;
			fGotClipFlag = true;
			if (fSaveIt)
				SaveClip();
			fURLBox->SetEnabled(true);
			break;
		}
		case statFINISH_PLAY:
		{
			if (!fGetFlag)
				SetStatus("Playback finished");
			if (fGetFlag && fSaveIt)
				SetStatus("Downloading...");
			if (fGetFlag && !fSaveIt) {
				SetStatus("Aborted");
				fAbortedFlag = true;
				KillThread();
			}
			if (fAbortedFlag)
				SetStatus("Aborted");	
			if (fGotClipFlag)
				fPlayedFlag = true;

			fPlayingFlag = false;
			fPlayButton->SetEnabled(true);
			fPlayMenu->SetEnabled(true);		
			break;
		}
		case statFINISH_SAVE:
		{
			SetStatus("Saving complete");
			fSaveButton->SetEnabled(true);
			fSaveMenu->SetEnabled(true);
			fSavedFlag = true;
			break;
		}
		case statPLAYING:
		{
			fPlayingFlag = true;
			SetStatus("Playing...");
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}


void
MainWindow::FrameResized(float width, float height)
{
		TruncateTitle();
}


void
MainWindow::TruncateTitle()
{
	float widthURL(fURLBox->Bounds().Width());
	BString* title = new BString(fClipTitle);
	fTitleView->TruncateString(title, B_TRUNCATE_END, widthURL - 120.0);
	fTitleView->SetText(title->String());
	
	return;
}


bool
MainWindow::QuitRequested()
{
	if (fGetFlag) {
		BAlert* alert = new BAlert("Abort download?", "A clip is still being"
			"downloaded.\nDo you want to abort or continue the download?",
			"Abort download", "Continue download", NULL, B_WIDTH_FROM_LABEL,
			B_OFFSET_SPACING, B_WARNING_ALERT);
		alert->SetShortcut(1, B_ESCAPE);

		switch (alert->Go())
		{
			case 0:	// abort
			{
				KillThread();
				break;
			}
			case 1:	// continue
			{
				return false;
			}
			default:
				return false;
		}
	}
	if (fSettings.StateClear()) {

		system("rm /tmp/ubertuber/*");
		printf("Deleting temporary files\n");

	}
	fSettings.SetWindowPosition(ConvertToScreen(Bounds()));
	fWorkerThread->PostMessage(B_QUIT_REQUESTED);
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::KillThread()
{
	BString command("ps -o Id Team | grep python | grep youtube-dl | awk '{ print $1; }' ; exit");

	char output[6];
	FILE* pget_threadID;

	pget_threadID = popen(command.String(),"r");
	fread(output, 1, sizeof(output), pget_threadID);
	fclose(pget_threadID);

	/* strip trailing newline */
	for (int i = 0; (unsigned) i < strlen(output); i++) {
		if (output[i] == '\n' || output[i] == '\r')
			output[i] = '\0';
	}
	
	printf("Kill, Kill, Kill!\nThreadID: %s\n", output);

	int ID(atoi(output));
	kill_thread((thread_id) ID);	
	
	return;
}


bool
MainWindow::ValidURL(BString url)
{
	bool valid = false;
	int i = 0;
	
	while (validAddress[i] != 0) {
		if (url.IFindFirst(validAddress[i]) != B_ERROR) {
			valid = true;
			break;
		}
		i++;
	}
	return valid;
}


void
MainWindow::ResetFlags()
{
	fAbortedFlag = false;
	fGetFlag = false;
	fGotClipFlag = false;
	fPlayedFlag = false;
	fPlayingFlag = false;
	fSavedFlag = false;
	fSaveIt = false;
	return;
}


void
MainWindow::SetStatus(char* text)
{
	fStatusView->SetText(text);
	return;
}


BString
MainWindow::GetClipboard()
{
	const char* text;
	ssize_t textLen;
	BMessage* clipboard;
	if (be_clipboard->Lock()) {
		if ((clipboard = be_clipboard->Data()))
			clipboard->FindData("text/plain", B_MIME_TYPE,
				(const void **)&text, &textLen);
		be_clipboard->Unlock();
	}
	BString clipboardString(text, textLen);
	return clipboardString;
}


void
MainWindow::GetTitle()
{
	BMessage msg(msgGETTITLE);
	msg.AddString("url", fURL->String());
	fWorkerThread->Looper()->PostMessage(&msg, fWorkerThread);

	return;
}


bool
MainWindow::GetClip()
{
	if (fSavedFlag || fPlayedFlag)
		return true;

	GetTitle();
	
	BMessage msg(msgGETCLIP);
	msg.AddString("url", fURL->String());
	fWorkerThread->Looper()->PostMessage(&msg, fWorkerThread);

	fGetFlag = true;
	fAbortButton->SetEnabled(true);
	fAbortMenu->SetEnabled(true);
	fAbortButton->SetEnabled(true);

	fURLBox->SetEnabled(false);
	return true;
}


void
MainWindow::PlayClip()
{
	BString* command = new BString(
	"hey application/x-vnd.UberTuber buff ; "
	"cd %DIR% ; "
	"FILE=$(youtube-dl --restrict-filenames --get-filename %URL% 2>&1 | tail -n 1) ; "
	"until [ -e \"$FILE.part\" ] || [ -e \"$FILE\" ] ; do " // wait until file exists
	"sleep 1 ; "
	"done ; "
	"settype -t video/mpeg4 \"$FILE.part\" ; "	// Force MPEG4 for MediaPlayer
	"settype -t video/mpeg4 \"$FILE\" ; "
	"hey application/x-vnd.UberTuber play ; "
	"if [ -e \"$FILE.part\" ] ; then "
	"open \"$FILE.part\" ; "
	"elif [ -e \"$FILE\" ] ; then "
	"open \"$FILE\" ; "
	"else "
	"hey application/x-vnd.UberTuber erro ; "
	"exit 1 ; "
	"fi ; "
	"sleep 1 ; "
	"TITLETHREAD1=$(echo $FILE | cut -c 1-45) ; "
	"TITLETHREAD2=$(echo $FILE.part | cut -c 1-45) ; "
	"waitfor -e \"w>$TITLETHREAD1\" ; "
	"waitfor -e \"w>$TITLETHREAD2\" ; "
	"if [ -e \"$FILE.part\" ] ; then "	
	"mimeset -F \"$FILE.part\" ; "				// Reset mimetype
	"else "
	"mimeset -F \"$FILE\" ; "
	"fi ; "
	"hey application/x-vnd.UberTuber pfin ; "
	"exit");

	command->ReplaceAll("%URL%", fURL->String());
	
	if (fSavedFlag)
		command->ReplaceAll("%DIR%", fSaveDir->String());
	else
		command->ReplaceAll("%DIR%", "/tmp/ubertuber");

	if (fSavedFlag || fPlayedFlag || fGotClipFlag)
		command->RemoveAll("sleep 2 ; ");

	thread_id playthread = spawn_thread(_call_script, "Clip player",
		B_LOW_PRIORITY, command);

	if (playthread < B_OK)
		return;
	resume_thread(playthread);

	return;
}


void
MainWindow::SaveClip()
{
	BString* command = new BString(
		"cd /tmp/ubertuber ; "
		"FILE=$(youtube-dl --restrict-filenames --get-filename %URL% 2>&1 | tail -n 1) ; "
		"addattr -t string META:url %URL% \"$FILE\" ; "
		"mv \"$FILE\" \"%DEST%\" ; "
		"hey application/x-vnd.UberTuber sfin ; "
		"exit");

	command->ReplaceAll("%DEST%", fSaveDir->String());
	command->ReplaceAll("%URL%", fURL->String());

	thread_id savethread = spawn_thread(_call_script, "Clip saver",
		B_LOW_PRIORITY, command);

	if (savethread < B_OK)
		return;

	resume_thread(savethread);
	
	return;
}

