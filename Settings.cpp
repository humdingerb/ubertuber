/*
 * Copyright 2011. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */


#include <Application.h>
#include <FindDirectory.h>
#include <File.h>
#include <Path.h>
#include <Font.h>
#include <String.h>
#include <stdio.h>
#include <Message.h>

#include "Settings.h"

#include <stdio.h>
#include <stdlib.h>


Settings::Settings()
{
	BPath path;
	BMessage msg;

	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK) {
		path.Append(kSettingsFile);
		BFile file(path.Path(), B_READ_ONLY);

		if (file.InitCheck() != B_OK)
			SetDefaults();
		else if (msg.Unflatten(&file) != B_OK)
			SetDefaults();
		else {
			BString dir;
			msg.FindBool("autoplay", &fStateAuto);
			msg.FindBool("clear", &fStateClear);
			msg.FindBool("history", &fStateHistory);
			msg.FindString("savefolder", &dir);
			msg.FindRect("windowlocation", &fPosition);

			BPath path(dir.String());
			fLastDir = path;
			
			originalStateAuto = fStateAuto;
			originalStateClear = fStateClear;
			originalStateHistory = fStateHistory;
			originalLastDir = fLastDir;
			originalPosition = fPosition;
		}
	}
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK) {
		path.Append(kMonitorFile);
		
		FILE* file;
		char* line = NULL;
		size_t len = 0;
		ssize_t read;
		
		file = fopen(path.Path(), "r");
		if (file == NULL) {
			printf("Monitor file doesn't exist -> create it.\n");

			BString* command = new BString(
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
			"exit");
			command->ReplaceAll("%FILE%", path.Path());
			system(command->String());
				
			file = fopen(path.Path(), "r");
		}
		while ((read = getline(&line, &len, file)) != -1) {
			if ((line[0] == '#') || (read == 1))	// Comment or empty
				continue;
			
			/* strip trailing newline */
			for (int i = 0; (unsigned) i < strlen(line); i++) {
				if (line[i] == '\n' || line[i] == '\r' )
					line[i] = '\0';
			}
			fValidAddressList.AddItem(new BString(line));
		}
    }
}


Settings::~Settings()
{
	if (originalStateAuto == fStateAuto &&
		originalStateClear == fStateClear &&
		originalStateHistory == fStateHistory &&
		originalLastDir == fLastDir &&
		originalPosition == fPosition)
			return;

	BPath path;
	BMessage msg;

	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) < B_OK)
		return;

	path.Append(kSettingsFile);
	BFile file(path.Path(), B_WRITE_ONLY|B_CREATE_FILE);

	if (file.InitCheck() == B_OK) {
		
		BString dir;
		dir = fLastDir.Path();
		
		msg.AddBool("autoplay", fStateAuto);
		msg.AddBool("clear", fStateClear);
		msg.AddBool("history", fStateHistory);
		msg.AddString("savefolder", dir);
		msg.AddRect("windowlocation", fPosition);
		msg.Flatten(&file);
	}
}


bool
Settings::ValidURL(BString url)
{
	bool valid = false;

	for (int32 i = 0; i < fValidAddressList.CountItems(); i++) {
		BString* address = fValidAddressList.ItemAt(i);
		if (url.IFindFirst(address->String()) != B_ERROR) {
			valid = true;
			break;
		}
	}
	return valid;
}


void
Settings::SetDefaults()
{
	fPosition.Set(-1, -1, -1, -1);

	BPath path;
	find_directory(B_USER_DIRECTORY, &path);

	fLastDir = path;
	fStateAuto = false;
	fStateClear = true;
	fStateHistory = true;
}


void
Settings::SetStateAuto(bool state)
{
	fStateAuto = state;
}


void
Settings::SetStateClear(bool state)
{
	fStateClear = state;
}


void
Settings::SetStateHistory(bool state)
{
	fStateHistory = state;
}


void
Settings::SetLastDir(BPath destPath)
{
	fLastDir = destPath;
}


void
Settings::SetWindowPosition(BRect where)
{
	fPosition = where;
}
