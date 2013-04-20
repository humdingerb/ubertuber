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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Settings.h"

#include <Button.h>
#include <FilePanel.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>

enum {
	statBUFFER			= 'buff',
	statDOWNLOAD		= 'down',
	statUPDATING		= 'upda',
	statFINISH_GET		= 'gfin',
	statFINISH_PLAY		= 'pfin',
	statFINISH_SAVE		= 'sfin',
	statFINISH_UPDATE	= 'ufin',
	statERROR			= 'erro',
	statPLAYING			= 'play',
	statINSTPYTH		= 'ipyt',
	statINSTRDY			= 'irdy',

	msgABORT	= 'abor',
	msgAUTO		= 'auto',
	msgCLEAR	= 'clea',
	msgCLEARHIST = 'clhi',
	msgHISTORY	= 'hist',
	msgPLAY		= 'ypla',
	msgSAVE		= 'esav',
	msgUPDATE	= 'updt',
	msgURL		= 'urly'
};


class MainWindow : public BWindow {
public:
						MainWindow();
		void			MessageReceived(BMessage* msg);
		bool			QuitRequested();
		virtual void	FrameResized(float width, float height);

private:
		void			_BuildMenu();
		void			_BuildLayout();
		void			_GetDirectories();
		void			_CheckForPython();

		bool			GetClip();
		void			GetTitle();
		BString			GetClipboard();
		void			KillThread();
		void			ResetFlags();
		void			SetStatus(char* text);
		void			TruncateTitle();
		bool			ValidURL(BString url);

		void			PlayClip();
		void			SaveClip();
		void			UpdateScript();

private:
		BButton*		fAbortButton;
		BButton*		fPlayButton;
		BButton*		fSaveButton;

		BFilePanel*		fSavePanel;

		BMenuBar*		fMenuBar;
		BMenuItem*		fAbortMenu;
		BMenuItem*		fAutoMenu;
		BMenuItem*		fClearMenu;
		BMenuItem*		fClearHistory;
		BMenuItem*		fHistoryMenu;
		BMenuItem*		fPlayMenu;
		BMenuItem*		fSaveMenu;
		BMenuItem*		fUpdateMenu;

		BString*		fAppDir;
		BString			fClipTitle;
		BString*		fTitle;
		BString*		fSaveDir;
		BString*		fTempDir;
		BString*		fURL;

		BStringView*	fStatusView;
		BStringView*	fTitleView;
		BTextControl*	fURLBox;

		bool			fAbortedFlag;
		bool			fGetFlag;
		bool			fGotClipFlag;
		bool			fPlayedFlag;
		bool			fPlayingFlag;
		bool			fSavedFlag;
		bool			fSaveIt;

		Settings		fSettings;
		thread_id		fGetThread;
		thread_id		fPlayThread;
		thread_id		fSaveThread;
		thread_id		fUpdateThread;
};

#endif /* MAINWINDOW_H */
