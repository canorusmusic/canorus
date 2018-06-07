/*!
        Copyright (c) 2006-2010, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _KEYSIGNATURE_UI_H_
#define _KEYSIGNATURE_UI_H_

// Includes
#include <QObject>
#include <QComboBox>

// Forward declarations
class CAMainWin;
class QToolBar;
class CAKeySignatureCtl;

// Dummy ui is an example class for creating UI parts of the mainwindow
// Such ui objects are created via the Canorus mainwindow (currently)
// To find the widget children the parent must be a widget too!
class CAKeySignatureUI : public QWidget
{
	Q_OBJECT

public:
	CAKeySignatureUI( CAMainWin *poMainWin, const QString &oHash );
	~CAKeySignatureUI();
	void updateKeySigToolBar();
	inline CAKeySignatureCtl &ctl() { return *_poKeySignatureCtl; }

	static void populateComboBox( QComboBox *c );
	static void populateComboBoxDirection( QComboBox *c );

protected:
	CAMainWin    	  *_poMainWin;
	CAKeySignatureCtl *_poKeySignatureCtl;
	QToolBar *uiKeySigToolBar;
	// CAKeySigPSP  *uiKeySigPSP;	  // Key signature perspective. \todo Reimplement it.
	QComboBox *uiKeySig;
	// QComboBox    *uiKeySigGender;
};

#endif // _KEYSIGNATURE_UI_H

