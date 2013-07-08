/***************************************************************************
								qtconsole.h  -  description
									-------------------
	begin                : dom nov 17 13:34:51 CET 2002
	copyright            : (C) 2002 by Giancarlo
	email                : 
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QTCONSLE_H
#define QTCONSLE_H

// include files for QT
#include <qapp.h>
#include <qmainwindow.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qstatusbar.h>
#include <qwhatsthis.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qmsgbox.h>
#include <qfiledialog.h>
#include <qprinter.h>
#include <qpainter.h>

#include <qapplication.h>
#include <qfont.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>
// application specific includes
#include "qtconsoleview.h"
#include "qtconsoledoc.h"

#define MAX_BUF_CHARS 50
#define VERSION "1.0"

/**
* This Class is the base class for your application. It sets up the main
* window and providing a menubar, toolbar
* and statusbar. For the main view, an instance of class QTconsoleView is
* created which creates your view.
*/
class QTconsoleApp : public QMainWindow
{
Q_OBJECT

public:
	/** construtor */
	QTconsoleApp();
	/** destructor */
	~QTconsoleApp();
	/** initializes all QActions of the application */
	void initActions();
	/** initMenuBar creates the menu_bar and inserts the menuitems */
	void initMenuBar();
	/** this creates the toolbars. Change the toobar look and add new toolbars in this
	* function */
	void initToolBar();
	/** setup the statusbar */
	void initStatusBar();
	/** setup the document*/
	void initDoc();
	/** setup the mainview*/
	void initView();

	/** overloaded for Message box on last window exit */
	bool queryExit();
	
	QTconsoleDoc *getDoc() { return doc; }
	QKeyEvent *nextKey( );

public slots:

	/** generate a new document in the actual view */
	void slotFileNew();
	/** open a document */
	void slotFileOpen();
	/** save a document */
	void slotFileSave();
	/** save a document under a different filename*/
	void slotFileSaveAs();
	/** close the actual file */
	void slotFileClose();
	/** print the actual file */
	void slotFilePrint();
	/** exits the application */
	void slotFileQuit();
	/** put the marked text/object into the clipboard and remove
	* it from the document */
	void slotEditCut();
	/** put the marked text/object into the clipboard*/
	void slotEditCopy();
	/** paste the clipboard into the document*/
	void slotEditPaste();
	/** toggle the toolbar*/
	void slotViewToolBar(bool toggle);
	/** toggle the statusbar*/
	void slotViewStatusBar(bool toggle);

	/** shows an about dlg*/
	void slotHelpAbout();
/** Called when view has something important to notify
*/
void viewUpdated();


private:

	/** view is the main widget which represents your working area. The View
	* class should handle all events of the view widget.  It is kept empty so
	* you can create your view according to your application's needs by
	* changing the view class.
	*/
	QTconsoleView *view;
	/** doc represents your actual document and is created only once. It keeps
	* information such as filename and does the serialization of your files.
	*/
	QTconsoleDoc *doc;

	/** file_menu contains all items of the menubar entry "File" */
	QPopupMenu *fileMenu;
	/** edit_menu contains all items of the menubar entry "Edit" */
	QPopupMenu *editMenu;
	/** view_menu contains all items of the menubar entry "View" */
	QPopupMenu *viewMenu;
	/** view_menu contains all items of the menubar entry "Help" */
	QPopupMenu *helpMenu;
	/** the main toolbar */
	QToolBar *fileToolbar;
	/** actions for the application initialized in initActions() and used to en/disable them
		* according to your needs during the program */
	QAction *fileNew;
	QAction *fileOpen;
	QAction *fileSave;
	QAction *fileSaveAs;
	QAction *fileClose;
	QAction *filePrint;
	QAction *fileQuit;

	QAction *editCut;
	QAction *editCopy;
	QAction *editPaste;

	QAction *viewToolBar;
	/**  */
	int _cols;
	/**  */
	int _rows;

	QAction *viewStatusBar;

	QAction *helpAboutApp;

protected:
	void keyPressEvent( QKeyEvent *e );
};

#define GTQC_MAX_KEYCODES 50
extern int gtqt_keycodes[];
extern int gtqt_keycount;

#endif

