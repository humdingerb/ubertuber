/*
 * Copyright 2011-2012. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */


#ifndef SETTINGS_H
#define SETTINGS_H

#include <Path.h>
#include <Rect.h>


class Settings {
public:
					Settings();
					~Settings();

		BPath		LastDir() const { return fLastDir; }
		bool		StateAuto() const { return fStateAuto; }
		bool		StateClear() const { return fStateClear; }
		bool		StateHistory() const { return fStateHistory; }
		BRect		WindowPosition() const { return fPosition; }

		void		SetDefaults();
		void		SetLastDir(BPath);
		void		SetStateAuto(bool);
		void		SetStateClear(bool);
		void		SetStateHistory(bool);
		void		SetWindowPosition(BRect);

private:
		BPath		fLastDir;
		BRect		fPosition;
		bool		fStateAuto;
		bool		fStateClear;
		bool		fStateHistory;

		BPath		originalLastDir;
		BRect		originalPosition;
		bool		originalStateAuto;
		bool		originalStateClear;
		bool		originalStateHistory;
};

#endif	/* SETTINGS_H */