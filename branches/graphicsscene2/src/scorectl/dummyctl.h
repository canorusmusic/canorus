/*!
        Copyright (c) 2006-2010, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _DUMMY_CTL_H_
#define _DUMMY_CTL_H_

// Includes
#include <QObject>

// Forward declarations
class CAMainWin;
class CADummy;

// Dummy control is an example class for implementing UI controls
// Such controls are created via the Canorus mainwindow (currently)
class CADummyCtl : public QObject
{
	Q_OBJECT

public:
	CADummyCtl( CAMainWin *poMainWin );
	~CADummyCtl();

public slots:
	void on_uiDummy_triggered();
 
protected slots:
	void myToggle( int iOn );
	
protected:
	CAMainWin    *_poMainWin;
	CADummy      *_poDummy;
};

#endif // _DUMMY_CTL_H

