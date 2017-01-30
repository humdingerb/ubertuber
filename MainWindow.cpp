/*
 * Copyright 2011-2015. All rights reserved.
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
#include <Catalog.h>
#include <Clipboard.h>
#include <ControlLook.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <NodeMonitor.h>
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

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"

int32
_call_script(void* arg)
{
	BString* command = static_cast<BString *>(arg);
	system(command->String());
	return 0;
}


class DirectoryFilter : public BRefFilter {
public:
	DirectoryFilter() {};
	virtual bool Filter(const entry_ref* ref,
		BNode* node, struct stat_beos* st, const char* filetype)
	{
		// ToDo: Fix this properly in Tracker
		// If you create a folder, then rename it, node is NULL.
		// The BeBook says: "Note that the function is never sent an
		// abstract entry, so the node, st, and filetype arguments will
		// always be valid."
		return node == NULL ? false : node->IsDirectory();
	}
};


// #pragma mark -


MainWindow::MainWindow()
	:
	BWindow(BRect(), B_TRANSLATE_SYSTEM_NAME("UberTuber"), B_TITLED_WINDOW,
		B_NOT_V_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS |
		B_QUIT_ON_WINDOW_CLOSE | B_AUTO_UPDATE_SIZE_LIMITS),
	fSaveFilePanel(NULL),
	fClipPath(NULL),
	fAbortedFlag(false),
	fGetFlag(false),
	fGotClipFlag(false),
	fPlayedFlag(false),
	fPlayingFlag(false),
	fSavedFlag(false),
	fSaveIt(false)
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
	WatchMonitoredSitesList();
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

	menu = new BMenu(B_TRANSLATE("File"));
	menu->AddItem(fPlayMenu = new BMenuItem(B_TRANSLATE("Play"), new BMessage(msgPLAY), 'P'));
	fPlayMenu->SetEnabled(false);
	menu->AddItem(fSaveMenu = new BMenuItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS),
		new BMessage(msgSAVE), 'S'));
	fSaveMenu->SetEnabled(false);

	menu->AddItem(fAbortMenu = new BMenuItem(B_TRANSLATE("Abort download"),
		new BMessage(msgABORT)));
	fAbortMenu->SetEnabled(false);

	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("About UberTuber"),
		new BMessage(B_ABOUT_REQUESTED)));
	item->SetTarget(be_app);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED),
		'Q'));
	fMenuBar->AddItem(menu);

	menu = new BMenu(B_TRANSLATE("URL"));
	menu->AddItem(item = fClearURLMenu = new BMenuItem(B_TRANSLATE("Clear URL field"),
		new BMessage(msgCLEARURL), 'D'));
	fClearURLMenu->SetEnabled(false);
	menu->AddItem(item = fOpenURLMenu = new BMenuItem(B_TRANSLATE("Open URL in browser"),
		new BMessage(msgOPENURL), 'O'));
	fOpenURLMenu->SetEnabled(false);
	fMenuBar->AddItem(menu);

	menu = new BMenu(B_TRANSLATE("Settings"));
	menu->AddItem(item = fAutoMenu = new BMenuItem(B_TRANSLATE("Auto-play"),
		new BMessage(msgAUTO)));
	item->SetMarked(fSettings.StateAuto());
	menu->AddItem(item = fCleanMenu = new BMenuItem(
		B_TRANSLATE("Remove temporary files on quit"), new BMessage(msgCLEAN)));
	item->SetMarked(fSettings.StateClear());
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem(
		B_TRANSLATE("Edit monitored URLs" B_UTF8_ELLIPSIS), new BMessage(msgEDIT)));
	fMenuBar->AddItem(menu);
}


void
MainWindow::_BuildLayout()
{
	fURLBox = new BTextControl("urlbox", B_TRANSLATE("URL:"), " ", NULL);
	fURLBox->SetModificationMessage(new BMessage(msgURL));

	fTitleView = new BStringView("title", " ");
	fTitleView->SetFontSize(be_plain_font->Size() - 2);
	fTitleView->SetHighColor(tint_color(ui_color(B_CONTROL_TEXT_COLOR), 0.7));

	fStatusView = new BStringView("status", " ");
	fStatusView->SetFontSize(be_plain_font->Size() - 2);
	fStatusView->SetHighColor(tint_color(ui_color(B_CONTROL_TEXT_COLOR), 0.7));

	fAbortButton = new BButton("abortbutton", B_TRANSLATE("Abort download"),
		new BMessage(msgABORT));
	fAbortButton->SetEnabled(false);

	fPlayButton = new BButton("playbutton", B_TRANSLATE("Play"), new BMessage(msgPLAY));
	fPlayButton->SetEnabled(false);

	fSaveButton = new BButton("savebutton", B_TRANSLATE("Save as" B_UTF8_ELLIPSIS),
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


// #pragma mark -


status_t
MainWindow::WatchMonitoredSitesList()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return false;

	path.Append(kMonitorFile);
	BEntry* entry = new BEntry(path.Path());

	node_ref nref;
	entry->GetNodeRef(&nref);
	return (watch_node(&nref, B_WATCH_STAT, this));
}


void
MainWindow::MessageReceived(BMessage* msg)
{
	if (msg->WasDropped()) {
		entry_ref ref;
		int32 i = 0;
		if (msg->FindRef("refs", i++, &ref) == B_OK) {
			URLofFile(ref);
		}
		return;
	}

	entry_ref inRef;
	BEntry inEntry;

	switch (msg->what)
	{
		case B_NODE_MONITOR:
		{
			int32 opcode;
			if (msg->FindInt32("opcode", &opcode) == B_OK) {
				switch (opcode) {
					case B_STAT_CHANGED:
					{
						printf("Monitored sites list has changed.\n");
						fSettings.SetChangedMonitoredList();
						break;
					}
				}
			}
			break;
		}
		case B_CLIPBOARD_CHANGED:
		{
			BString clipboardString(GetClipboard());
			BString urlString(fURLBox->Text());

			if (!urlString.ICompare(clipboardString)) {
				printf("Same contents: break!\n");
				break;
			}

			if (fSettings.ValidURL(clipboardString) == false) {
				printf("Not valid: break!\n");
				break;
			}

			fURLBox->SetText(clipboardString.String());

			if (!fGetFlag) {
				fPlayButton->SetEnabled(true);
				fSaveButton->SetEnabled(true);
				fPlayMenu->SetEnabled(true);
				fSaveMenu->SetEnabled(true);

				ResetFlags();

				if (fSettings.StateAuto())
					PostMessage(msgPLAY);
			}
			SetStatus(B_TRANSLATE("Auto-inserted URL"));
			break;
		}

		case msgSAVE:
		{
			// Execute Save Panel
			if (fSaveFilePanel == NULL) {
				BButton* selectThisDir;

				BMessage folderSelect(FOLDER_SELECT_MESSAGE);
				BEntry entry(fSettings.LastDir().Path(), true);
				entry_ref destRef;
				if (entry.Exists() && entry.IsDirectory())
					entry.GetRef(&destRef);

				fSaveFilePanel = new BFilePanel(B_OPEN_PANEL, NULL, &destRef,
					B_DIRECTORY_NODE, true, &folderSelect, NULL, false, true);
				fSaveFilePanel->SetButtonLabel(B_DEFAULT_BUTTON, "Select");
				fSaveFilePanel->SetTarget(this);

				fSaveFilePanel->Window()->Lock();
				fSaveFilePanel->Window()->SetTitle(B_TRANSLATE("Choose destination folder"));
				BRect buttonRect
					= fSaveFilePanel->Window()->ChildAt(0)->FindView(
						"cancel button")->Frame();
				buttonRect.right  = buttonRect.left - 20;
				buttonRect.left = buttonRect.right - 130;
				selectThisDir = new BButton(buttonRect, NULL, B_TRANSLATE("Select this folder"),
					new BMessage(SELECT_THIS_DIR_MESSAGE),
					B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT);
				selectThisDir->SetTarget(this);
				fSaveFilePanel->Window()->ChildAt(0)->AddChild(selectThisDir);
				fSaveFilePanel->Window()->Unlock();

				fSaveFilePanel->SetRefFilter(new DirectoryFilter);
			}
			fURL = new BString(fURLBox->Text());
			fSaveFilePanel->Show();
			break;
		}
		case FOLDER_SELECT_MESSAGE:
		{
			// "SELECT" Button at Save Panel Pushed
			fSaveFilePanel->GetNextSelectedRef(&inRef);
			inEntry.SetTo(&inRef, true);

			BPath path(&inEntry);

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

			fSaveFilePanel->Rewind();
			break;
		}
		case SELECT_THIS_DIR_MESSAGE:
		{
			// "THIS DIR" Button at Save Panel Pushed
			fSaveFilePanel->GetPanelDirectory(&inRef);
			fSaveFilePanel->Hide();
			inEntry.SetTo(&inRef, true);

			BPath path(&inEntry);

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
		case msgAUTO:
		{
			fSettings.SetStateAuto(!fSettings.StateAuto());
			fAutoMenu->SetMarked(fSettings.StateAuto());
			break;
		}
		case msgCLEAN:
		{
			fSettings.SetStateClear(!fSettings.StateClear());
			fCleanMenu->SetMarked(fSettings.StateClear());
			break;
		}
		case msgEDIT:
		{
			BPath path;
			if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK) {
				path.Append(kMonitorFile);
				BString* command = new BString(
				"if [ ! -e %FILE% ] ; then "
				"touch %FILE% ; "
				"printf \""
				"# UberTuber's list of monitored websites\n"
				"#\n"
				"# UberTuber monitors the clipboard and auto-inserts the URL of a known\n"
				"# website into its URL text field. The 'known' websites are defined in this\n"
				"# text file. See http://rg3.github.io/youtube-dl/supportedsites.html for a\n"
				"# list of reportedly working sites.\n"
				"#\n"
				"# Add, remove or edit URLs to your needs. One URL per line.\n"
				"\n"
				"blip.tv\n"
				"escapistmagazine.com/videos/view\n"
				"dailymotion.com\n"
				"depositfiles.com\n"
				"facebook.com/video\n"
				"metacafe.com/watch\n"
				"photobucket.com\n"
				"video.google.com\n"
				"video.yahoo.com\n"
				"vimeo.com\n"
				"youtu.be\n"
				"youtube.com/embed\n"
				"youtube.com/watch\n"
				"youtube-nocookie.com\n\" > %FILE% ; "
				"settype -t text/plain %FILE% ; "
				"fi ; "
				"open %FILE% ; "
				"exit");
				command->ReplaceAll("%FILE%", path.Path());
				system(command->String());
			}
			break;
		}
		case msgABORT:
		{
			KillThread("ps -o Id Team | grep python | grep youtube-dl | awk '{ print $1; }' ; exit");
			KillThread("ps -o Id Team | grep hey | grep UberTuber | awk '{ print $1; }' ; exit");

			SetStatus(B_TRANSLATE("Aborted"));
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
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			int32 i = 0;
			if (msg->FindRef("refs", i++, &ref) == B_OK) {
	//			printf("File dropped: %s\n -------- \n", ref.name);
				URLofFile(ref);
			}
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
				fClearURLMenu->SetEnabled(state);
				fOpenURLMenu->SetEnabled(state);

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
			if (fClipTitle.FindFirst("ERROR: YouTube said:") == 0) {
				PostMessage(msgABORT);
			}
			break;
		}
		case msgCLEARURL:
		{
			fURLBox->SetText("");
			break;
		}
		case msgOPENURL:
		{
			BString command("open %URL%");
			command.ReplaceAll("%URL%", fURLBox->Text());
			system(command);
			break;
		}

		// React to messages from 'hey', forwarded from App
		case statBUFFER:
		{
			SetStatus(B_TRANSLATE("Buffering" B_UTF8_ELLIPSIS));
			break;
		}
		case statDOWNLOAD:
		{
			SetStatus(B_TRANSLATE("Downloading" B_UTF8_ELLIPSIS));
			break;
		}
		case statERROR:
		{
			if (!fAbortedFlag)
				SetStatus(B_TRANSLATE("Download failed"));
			else {
				SetStatus(B_TRANSLATE("Aborted"));
				fAbortMenu->SetEnabled(false);
				fAbortButton->SetEnabled(false);
			}
			fURLBox->SetEnabled(true);
			ResetFlags();
			break;
		}
		case statFINISH_GET:
		{
			if (!fPlayingFlag && !fAbortedFlag)
				SetStatus(B_TRANSLATE("Download finished"));
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
				SetStatus(B_TRANSLATE("Playback finished"));
			if (fGetFlag && fSaveIt)
				SetStatus(B_TRANSLATE("Downloading" B_UTF8_ELLIPSIS));
			if (fGetFlag && !fSaveIt) {
				SetStatus(B_TRANSLATE("Aborted"));
				fAbortedFlag = true;
				KillThread("ps -o Id Team | grep python | grep youtube-dl | awk '{ print $1; }' ; exit");
				KillThread("ps -o Id Team | grep hey | grep UberTuber | awk '{ print $1; }' ; exit");
			}
			if (fAbortedFlag) {
				SetStatus(B_TRANSLATE("Aborted"));
				fGotClipFlag = false;
			}
			if (fGotClipFlag)
				fPlayedFlag = true;

			fPlayingFlag = false;
			fPlayButton->SetEnabled(true);
			fPlayMenu->SetEnabled(true);
			break;
		}
		case statFINISH_SAVE:
		{
			SetStatus(B_TRANSLATE("Saving complete"));
			fSaveButton->SetEnabled(true);
			fSaveMenu->SetEnabled(true);
			fSavedFlag = true;
			break;
		}
		case statPLAYING:
		{
			fPlayingFlag = true;
			fAbortButton->SetEnabled(true);
			fAbortMenu->SetEnabled(true);
			SetStatus(B_TRANSLATE("Playing" B_UTF8_ELLIPSIS));
			break;
		}
		// React to messages from the WorkerThreads
		case msgGETTITLE:
		{
			_GetTitleOutput(msg);
			break;
		}
		case msgGETCLIP:
		{
			_GetClipOutput(msg);
			break;
		}
		case msgPLAYCLIP:
		{
			_PlayClipOutput(msg);
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
MainWindow::_GetTitleOutput(BMessage* message)
{
	BString data;

	if (message->FindString("line", &data) == B_OK) {
		printf("GetTitle - Title: %s\n", data.String());
		SetStatus(data);
	}
}


void
MainWindow::_GetClipOutput(BMessage* message)
{
	BString data;

	if (message->FindString("line", &data) == B_OK) {
		if (data.FindFirst("ERROR: ") == 0) {
			printf("GetClip - Failure: %s\n", data.String());
			PostMessage(msgABORT);
		} else if (data.FindFirst("[download] Destination: ") == 0)
			fClipPath = data.ReplaceFirst("[download] Destination: ", "");
	}
	int32 code = -1;
	if (message->FindInt32("thread_exit", &code) == B_OK) {
		printf("GetClip - Download finished\n");
		PostMessage(statFINISH_GET);
	}
}


void
MainWindow::_PlayClipOutput(BMessage* message)
{
	BString data;

	if (message->FindString("line", &data) == B_OK) {
		if (data.FindFirst("ERROR: ") == 0) {
			printf("GetClip - Failure: %s\n", data.String());
			PostMessage(msgABORT);
		}
	}
	int32 code = -1;
	if (message->FindInt32("thread_exit", &code) == B_OK) {
		printf("GetClip - Download finished\n");
		PostMessage(statFINISH_GET);
	}
}


bool
MainWindow::QuitRequested()
{
	if (fGetFlag) {
		BAlert* alert = new BAlert(B_TRANSLATE("Abort download?"), B_TRANSLATE("A clip is still being "
			"downloaded.\nDo you want to abort or continue the download?"),
			B_TRANSLATE("Abort download"), B_TRANSLATE("Continue download"), NULL, B_WIDTH_FROM_LABEL,
			B_OFFSET_SPACING, B_WARNING_ALERT);
		alert->SetShortcut(1, B_ESCAPE);

		switch (alert->Go())
		{
			case 0:	// abort
			{
				KillThread("ps -o Id Team | grep python | grep youtube-dl | awk '{ print $1; }' ; exit");
				KillThread("ps -o Id Team | grep hey | grep UberTuber | awk '{ print $1; }' ; exit");
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
//	fWorkerThread->PostMessage(B_QUIT_REQUESTED);
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
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

void
MainWindow::URLofFile(entry_ref &ref)
{
	const ssize_t bufferSize = 256;
	char buffer[bufferSize];
	BNode node(&ref);
	ssize_t readBytes;

	readBytes = node.ReadAttr("META:url", B_STRING_TYPE, 0, buffer, bufferSize);
	if (readBytes < 1)
		SetStatus(B_TRANSLATE("No URL found"));
	else {
		fURLBox->SetText(buffer);
		if (!fGetFlag) {
			fPlayButton->SetEnabled(true);
			fSaveButton->SetEnabled(true);
			fPlayMenu->SetEnabled(true);
			fSaveMenu->SetEnabled(true);

			ResetFlags();

			if (fSettings.StateAuto())
				PostMessage(msgPLAY);
		}
	}
	return;
}


// #pragma mark -


void
MainWindow::KillThread(char* command)
{
	char output[6];
	FILE* pget_threadID(NULL);

	pget_threadID = popen(command,"r");
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
MainWindow::SetStatus(BString text)
{
	fStatusView->SetText(text.String());
	return;
}


void
MainWindow::SetURL(BString* url)
{
	fURLBox->SetText(url->String());
	return;
}


// #pragma mark -


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
	fGetTitleThread = new WorkerThread(NULL,
		new BInvoker(new BMessage(msgGETTITLE), this));

	fGetTitleThread->AddArgument("youtube-dl")
		->AddArgument("--get-title")
		->AddArgument(fURL->String())
		->AddArgument("2>&1 | tail -n 1")	// also get output from error out
		->Run();

	return;
}


bool
MainWindow::GetClip()
{
	if (fSavedFlag || fPlayedFlag)
		return true;

	fGetClipThread = new WorkerThread(NULL,
		new BInvoker(new BMessage(msgGETCLIP), this));

	fGetClipThread->AddArgument("youtube-dl")
		->AddArgument("youtube-dl")
		->AddArgument("--continue")
		->AddArgument("--restrict-filenames")
		->AddArgument("--no-part")
		->AddArgument("--no-cache-dir")
		->AddArgument("--format best")
		->AddArgument("--output '/tmp/ubertuber/%(title)s.%(ext)s'")
		->AddArgument(fURL->String())
		->Run();

	fGetFlag = true;

	fAbortButton->SetEnabled(fSaveIt);
	fAbortMenu->SetEnabled(fSaveIt);

	fURLBox->SetEnabled(false);
	return true;
}


void
MainWindow::PlayClip()
{
	fPlayThread = new WorkerThread(NULL,
		new BInvoker(new BMessage(msgPLAYCLIP), this));

	entry_ref ref;
	be_roster->FindApp("video/mpeg4", &ref);
	BString prefApp = BPath(&ref).Path();

	fPlayThread->AddArgument("settype -t video/mpeg4 &&")
		->AddArgument(prefApp.String())
		->AddArgument(fClipPath.String())
		->AddArgument("&& echo playbackfinished")	// needed?
		->Run();

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

