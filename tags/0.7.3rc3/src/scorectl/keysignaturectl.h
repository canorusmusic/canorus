/*!
        Copyright (c) 2006-2010, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _KEYSIGNATURE_CTL_H_
#define _KEYSIGNATURE_CTL_H_

// Includes
#include <QObject>

// Forward declarations
class CAMainWin;

// Keysignature control is a class for implementing UI controls
// for placing key signatures into a score document
// It is created via the Canorus mainwindow (currently)
class CAKeySignatureCtl : public QObject
{
	Q_OBJECT

public:
	CAKeySignatureCtl( CAMainWin *poMainWin, const QString &oHash );
	~CAKeySignatureCtl();
	void setupActions();
	
protected:
	CAMainWin    *_poMainWin;

public slots:
	void on_uiInsertKeySig_toggled(bool);
	// Key Signature
	void on_uiKeySig_activated( int );

private:
	const QString _oHash;
};

#endif // _KEYSIGNATURE_CTL_H

