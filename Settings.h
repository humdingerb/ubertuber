/*
 * Copyright 2011-2012. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */


#ifndef SETTINGS_H
#define SETTINGS_H

#include <ObjectList.h>
#include <Path.h>
#include <Rect.h>
#include <String.h>

static const char kSettingsFile[] = "UberTuber_settings";
static const char kMonitorFile[] = "UberTuber_monitored_sites";

class Settings {
public:
					Settings();
					~Settings();

		BPath		LastDir() const { return fLastDir; }
		bool		StateAuto() const { return fStateAuto; }
		bool		StateClear() const { return fStateClear; }
		BRect		WindowPosition() const { return fPosition; }

		void		SetChangedMonitoredList();
		void		SetDefaults();
		void		SetLastDir(BPath);
		void		SetStateAuto(bool);
		void		SetStateClear(bool);
		void		SetWindowPosition(BRect);

		bool		ValidURL(BString);
		void		ReadMonitoredSitesList();

private:
		BPath		fLastDir;
		BRect		fPosition;
		bool		fStateAuto;
		bool		fStateClear;

		BPath		originalLastDir;
		BRect		originalPosition;
		bool		originalStateAuto;
		bool		originalStateClear;
		
		BObjectList<BString>	fValidAddressList;
		bool		fChangedMonitoredList;
};

#endif	/* SETTINGS_H */
