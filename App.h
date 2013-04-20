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
		void			AboutRequested();
		void			MessageReceived(BMessage* msg);

private:
		MainWindow*		fMainWindow;
};

#endif /* APP_H */
