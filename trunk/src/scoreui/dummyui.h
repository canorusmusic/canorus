/*!
        Copyright (c) 2006-2010, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _DUMMY_UI_H_
#define _DUMMY_UI_H_

// Includes
#include <QObject>

// Forward declarations
class CAMainWin;
class CADummyUIObj;
class CADummyCtl;

// Dummy ui is an example class for creating UI parts of the mainwindow
// Such ui objects are created via the Canorus mainwindow (currently)
class CADummyUI : public QObject
{
	Q_OBJECT

public:
	CADummyUI( CAMainWin *poMainWin );
	~CADummyUI();

protected:
	updateDummyUIObjs();

	CAMainWin    *_poMainWin;
	CADummyCtl   *_poDummyCtl;
	CADummyUIObj *_poDummyUIObj;
};

#endif // _DUMMY_UI_H

