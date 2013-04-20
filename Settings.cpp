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

static const char kSettingsFile[] = "UberTuber_settings";


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
