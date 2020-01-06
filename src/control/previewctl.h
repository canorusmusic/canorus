/*!
        Copyright (c) 2006-2019, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef PREVIEW_CTL_H
#define PREVIEW_CTL_H

// Includes
#include <QFile>
#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QVector>

// Forward declarations
class CAMainWin;
class CAPDFExport;

class CAPreviewCtl : public QObject {
    Q_OBJECT

public:
    CAPreviewCtl(CAMainWin* poMainWin);
    ~CAPreviewCtl();

public slots:
    void on_uiPrintPreview_triggered();

protected slots:
    void showPDF(int iExitCode);

protected:
    CAMainWin* _poMainWin;
    CAPDFExport* _poPDFExport;
    QString _oOutputPDFName;
};

#endif // PREVIEW_CTL_H
