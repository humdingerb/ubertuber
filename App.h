/*
 * Copyright 2011. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef APP_H
#define APP_H

#include "MainWindow.h"

#include <Application.h>


class App : public BApplication {
public:
						App();
		virtual			~App();
		virtual	void	ArgvReceived(int32 argc, char** argv);
		virtual	void	RefsReceived(BMessage* message);

		virtual	void	ReadyToRun();
		void			AboutRequested();
		void			MessageReceived(BMessage* message);

private:
		MainWindow*		fMainWindow;
		BMessage*		fSavedRefsReceived;
};

#endif /* APP_H */
