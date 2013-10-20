/*
 * Copyright 2011-2013. All rights reserved.
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
	fSaveIt(false)
{
	_GetDirectories();
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
	menu->AddSeparatorItem();
	menu->AddItem(fUpdateMenu = new BMenuItem("Update download script",
		new BMessage(msgUPDATE)));
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
			if (fGetThread) {
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
			}
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
		case msgUPDATE:
		{
			UpdateScript();
			SetStatus("");
			fTitleView->SetText("");
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
			fUpdateMenu->SetEnabled(true);

			break;
		}
		case statFINISH_GET:
		{
			if (!fPlayingFlag)
				SetStatus("Download finished");
			fAbortButton->SetEnabled(false);
			fAbortMenu->SetEnabled(false);
			fUpdateMenu->SetEnabled(true);
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
			fUpdateMenu->SetEnabled(true);
			
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
		case statFINISH_UPDATE:
		{
			SetStatus("Update finished");
			fUpdateMenu->SetEnabled(true);
			break;
		}
		case statPLAYING:
		{
			fPlayingFlag = true;
			SetStatus("Playing...");
			break;
		}
		case statUPDATING:
		{
			SetStatus("Updating script...");
			break;
		}
		case statINSTRDY:
		{
			SetStatus("Installation finished");
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
		BString	command("rm %TMP%/*");

		command.ReplaceAll("%TMP%", fTempDir->String());

		printf("Deleting temporary files\n");
		system(command.String());
	}
	fSettings.SetWindowPosition(ConvertToScreen(Bounds()));

	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::KillThread()
{
	BString threadname("/bin/python %APP%/youtube-dl "
		"--max-quality=22 --continue --restrict-filenames %URL%");
	threadname.ReplaceAll("%APP%", fAppDir->String());
	threadname.Truncate(63);

	BString command("ps | grep \"%THREAD%\" | awk '{ print $3; }' ; exit");
	command.ReplaceAll("%THREAD%", threadname.String());

	char output[6];
	FILE* pget_threadID;

	pget_threadID = popen(command.String(),"r");
	fread(output, 1, sizeof(output), pget_threadID);
	fclose(pget_threadID);

	if (output[0] == '-') {	// If path to UberTuber is too short, column 3
							// of ps output is "--max". Get column 4 instead.
		BString command("ps | grep \"%THREAD%\" | awk '{ print $4; }' ; exit");
		command.ReplaceAll("%THREAD%", threadname.String());
	
		pget_threadID = popen(command.String(),"r");
		fread(output, 1, sizeof(output), pget_threadID);
		fclose(pget_threadID);
	}

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


void
MainWindow::_GetDirectories()
{
	BPath path;
	find_directory(B_SYSTEM_TEMP_DIRECTORY, &path);
	fTempDir = new BString(path.Path());
	fTempDir->Append("/ubertuber");

	app_info info;
	be_app->GetAppInfo(&info);
	path = BPath(&info.ref);
	path.GetParent(&path);
	fAppDir = new BString(path.Path());
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
	BString command("python %APP%/youtube-dl --get-title %URL%");
	command.ReplaceAll("%APP%", fAppDir->String());
	command.ReplaceAll("%URL%", fURL->String());
	command.Append(" 2>&1 | tail -n 1");  // also get output from error out

printf("Cliptitle:\n");

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
	fClipTitle = title;			// saving original title
	TruncateTitle();

	return;
}


void
MainWindow::GetFilename()
{
	BString command("python %APP%/youtube-dl --restrict-filenames --get-filename %URL%");
	command.ReplaceAll("%APP%", fAppDir->String());
	command.ReplaceAll("%URL%", fURL->String());
	command.Append(" 2>&1 | tail -n 1");  // also get output from error out

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
	fFilename = new BString(title, strlen(title));

	return;
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
MainWindow::GetClip()
{
	if (fSavedFlag || fPlayedFlag)
		return true;

	GetTitle();
	GetFilename();

	if (fFilename->FindFirst("ERROR: ") != B_ERROR) {
		SetStatus("Not a valid URL");
		printf("GetClip: Not a valid URL");
		return false;
	}
	printf("GetClip: %s\n", fFilename->String());

	BString* command = new BString(
	"hey application/x-vnd.UberTuber down ; "
	"mkdir -p %DIR% ; "
	"cd %DIR% ; "
	"python %APP%/youtube-dl --max-quality=22 --continue --restrict-filenames %URL% ; "
	"while [ -n \"$(%TEST%)\" ] ; do " // wait for script to finish/aborted
	"sleep 2 ; "
	"done ; "
	"addattr -t string META:url %URL% \"%FILE%\" ; "
	"if [ -e \"%FILE%\" ] ; then "
	"hey application/x-vnd.UberTuber gfin ; "
	"else hey application/x-vnd.UberTuber erro ; "
	"fi ; "
	"exit");

	BString threadname("/bin/python %APP%/youtube-dl "
		"--max-quality=35 --continue --restrict-filenames %URL%");
	threadname.ReplaceAll("%APP%", fAppDir->String());
	threadname.Truncate(63);
	BString threadtest("ps | grep \"%THREAD%\"");
	threadtest.ReplaceAll("%THREAD%", threadname.String());

	command->ReplaceAll("%TEST%", threadtest.String());
	command->ReplaceAll("%DIR%", fTempDir->String());
	command->ReplaceAll("%APP%", fAppDir->String());
	command->ReplaceAll("%URL%", fURL->String());
	command->ReplaceAll("%FILE%", fFilename->String());

	fGetThread = spawn_thread(_call_script, "Clip downloader",
		B_LOW_PRIORITY, command);

	if (fGetThread < B_OK)
		return false;

	resume_thread(fGetThread);

	fGetFlag = true;
	fAbortButton->SetEnabled(true);
	fAbortMenu->SetEnabled(true);
	fAbortButton->SetEnabled(true);

	fUpdateMenu->SetEnabled(false);
	fURLBox->SetEnabled(false);
	return true;
}


void
MainWindow::PlayClip()
{
	printf("->PlayClip\n");

	BString* command = new BString(
	"sleep 2 ; "
	"hey application/x-vnd.UberTuber buff ; "
	"cd %DIR% ; "
	"sleep 4 ; "
	"settype -t video/mpeg4 \"%FILE%.part\" ; "	// Force MPEG4 for MediaPlayer
	"settype -t video/mpeg4 \"%FILE%\" ; "
	"hey application/x-vnd.UberTuber play ; "
	"if [ -e \"%FILE%.part\" ] ; then "
	"open \"%FILE%.part\" ; "
	"elif [ -e \"%FILE%\" ] ; then "
	"open \"%FILE%\" ; "
	"else "
	"hey application/x-vnd.UberTuber erro ; "
	"exit 1 ; "
	"fi ; "
	"sleep 1 ; "
	"waitfor -e \"w>%TITLETHREAD1%\" ; "
	"waitfor -e \"w>%TITLETHREAD2%\" ; "
	"mimeset -F \"%FILE%.part\" ; "				// Reset mimetype
	"mimeset -F \"%FILE%\" ; "
	"hey application/x-vnd.UberTuber pfin ; "
	"exit");

	if (fSavedFlag)
		command->ReplaceAll("%DIR%", fSaveDir->String());
	else
		command->ReplaceAll("%DIR%", fTempDir->String());

	if (fSavedFlag || fPlayedFlag || fGotClipFlag)
		command->RemoveAll("sleep 2 ; ");

	command->ReplaceAll("%FILE%", fFilename->String());

	// Truncate MediaPlayer thread names
	BString title = fFilename->String();
	command->ReplaceAll("%TITLETHREAD1%", title.Truncate(45));
	title = fFilename->String();
	title.Append(".part");
	command->ReplaceAll("%TITLETHREAD2%", title.Truncate(45));

	fPlayThread = spawn_thread(_call_script, "Clip player",
		B_LOW_PRIORITY, command);

	if (fPlayThread < B_OK)
		return;

	resume_thread(fPlayThread);

	return;
}


void
MainWindow::SaveClip()
{
	BString* command = new BString(
		"cd %DIR% ; "
		"addattr -t string META:url %URL% \"%FILE%\" ; "
		"mv \"%FILE%\" \"%DEST%\" ; "
		"hey application/x-vnd.UberTuber sfin ; "
		"exit");

	command->ReplaceAll("%DIR%", fTempDir->String());
	command->ReplaceAll("%DEST%", fSaveDir->String());
	command->ReplaceAll("%FILE%", fFilename->String());
	command->ReplaceAll("%URL%", fURL->String());

	fSaveThread = spawn_thread(_call_script, "Clip saver",
		B_LOW_PRIORITY, command);

	if (fSaveThread < B_OK)
		return;

	resume_thread(fSaveThread);
	
	return;
}


void
MainWindow::UpdateScript()
{
	printf("Updating download script...\n");

	BString* command = new BString(
		"hey application/x-vnd.UberTuber upda ; "
		"python %APP%/youtube-dl --update; "
		"hey application/x-vnd.UberTuber ufin ; "
		"exit");

	command->ReplaceAll("%APP%", fAppDir->String());

	fUpdateThread = spawn_thread(_call_script, "Skript updater",
		B_LOW_PRIORITY, command);

	if (fUpdateThread < B_OK)
		return;

	resume_thread(fUpdateThread);
	return;
}
