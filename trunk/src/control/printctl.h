/*!
        Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _PRINT_CTL_H_
#define _PRINT_CTL_H_

// Includes
#include <QObject>
#include <QVariant>
#include <QVector>
#include <QStringList>
#include <QFile>

// Forward declarations
class CATypesetCtl;
class CADocument;
class CAMainWin;

class CAPrintCtl : public QObject
{
	Q_OBJECT

public:
	CAPrintCtl( CAMainWin *poMainWin );
	~CAPrintCtl();

public slots:
	void on_uiPrint_triggered();
 
protected slots:
	void outputTypsetterOutput( const QByteArray &roOutput );
	void printPDF( int iExitCode );
	
protected:
	CATypesetCtl *_poTypesetCtl;
	CAMainWin    *_poMainWin;
	QString           _oOutputPDFName;
};

#endif // _PRINT_CTL_H

