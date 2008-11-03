/*!
        Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _PREVIEW_CTL_H_
#define _PREVIEW_CTL_H_

// Includes
#include <QObject>
#include <QVariant>
#include <QVector>
#include <QStringList>
#include <QFile>

// Forward declarations
class CAMainWin;
class CAPDFExport;

class CAPreviewCtl : public QObject
{
	Q_OBJECT

public:
	CAPreviewCtl( CAMainWin *poMainWin );
	~CAPreviewCtl();

public slots:
	void on_uiPrintPreview_triggered();
 
protected slots:
	void showPDF( int iExitCode );
	
protected:
	CAMainWin    *_poMainWin;
	CAPDFExport *_poPDFExport;
	QString           _oOutputPDFName;
};

#endif // _PREVIEW_CTL_H

