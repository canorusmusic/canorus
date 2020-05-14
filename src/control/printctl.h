/*!
        Copyright (c) 2006-2019, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _PRINT_CTL_H_
#define _PRINT_CTL_H_

// Includes
#include <QFile>
#include <QObject>
#include <QPrinterInfo>
#include <QStringList>
#include <QVariant>
#include <QVector>

#include <memory>

// Forward declarations
class CAMainWin;
class CASVGExport;

class CAPrintCtl : public QObject {
    Q_OBJECT

public:
    CAPrintCtl(CAMainWin* poMainWin);
    ~CAPrintCtl();

public slots:
    void on_uiPrint_triggered();
    void on_uiPrintDirectly_triggered();

protected slots:
    void printSVG(int iExitCode);

protected:
    void printDocument();

    CAMainWin* _poMainWin;
    std::unique_ptr<CASVGExport> _poSVGExport;
    QString _oOutputSVGName;
    bool _showDialog; // used when printing directly
};

#endif // _PRINT_CTL_H
