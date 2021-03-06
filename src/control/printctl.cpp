/*!
        Copyright (c) 2006-2019, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include <QDesktopServices>
#include <QMessageBox>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QSvgRenderer>
#include <QUrl>

#include "canorus.h"
#include "control/printctl.h"
#include "core/settings.h"
#include "export/svgexport.h"
#include "ui/mainwin.h"

CAPrintCtl::CAPrintCtl(CAMainWin* poMainWin)
{
    setObjectName("oPrintCtl");
    _poMainWin = poMainWin;
    _poSVGExport = std::make_unique<CASVGExport>();
    _showDialog = true;
    if (poMainWin == nullptr)
        qCritical("PrintCtl: No mainwindow instance available!");
    else
        CACanorus::connectSlotsByName(_poMainWin, this);
    connect(_poSVGExport.get(), SIGNAL(svgIsFinished(int)), this, SLOT(printSVG(int)));
}

// Destructor
CAPrintCtl::~CAPrintCtl() = default;

void CAPrintCtl::on_uiPrint_triggered()
{
    _showDialog = true;
    printDocument();
}

void CAPrintCtl::on_uiPrintDirectly_triggered()
{
    _showDialog = false;
    printDocument();
}

void CAPrintCtl::printDocument()
{
    QDir oPath(QDir::tempPath());
    QFile oTempFile(oPath.absolutePath() + "/print.svg");
    QString oTempFileName(oPath.absolutePath() + "/print.svg");
    if (!oTempFile.remove()) {
        qWarning("PrintCtl: Could not remove old print file %s, error %s", oTempFile.fileName().toLatin1().constData(),
            oTempFile.errorString().toLatin1().constData());
        oTempFile.unsetError();
    }
    qDebug("PrintCtl: Print triggered via main window");
    // The exportDocument method defines the temporary file name and
    // directory, so we can only read it after the creation
    _poSVGExport->setStreamToFile(oTempFileName);
    //_poSVGExport->exportDocument( _poMainWin->document() );
    _poSVGExport->exportSheet(_poMainWin->currentSheet());
    _poSVGExport->wait();
    const QString roTempPath = _poSVGExport->getTempFilePath();
    // Copy the name for later output on the printer
    _oOutputSVGName = roTempPath;
}

void CAPrintCtl::printSVG(int iExitCode)
{
    if (iExitCode) {
        QMessageBox::critical(_poMainWin, tr("Error while printing"), tr("Error while running the typesetter.\n\nPlease install LilyPond (visit http://www.lilypond.org) and check the settings."));
        return;
    }

    // High resolution requires huge amount of memory :-(
    //QPrinter oPrinter;
    QPrinter oPrinter(QPrinterInfo::defaultPrinter(), QPrinter::ScreenResolution);

    QPainter oPainter;
    oPrinter.setFullPage(true);
    QPrintDialog oPrintDlg(&oPrinter);
    QDir oPath(QDir::tempPath());
    QFile oTempFile(oPath.absolutePath() + "/print.svg");
    QSvgRenderer oRen;
    oTempFile.setFileName(_oOutputSVGName + ".svg");
    // Start by letting the user select the print settings
    if (oRen.load(oPath.absolutePath() + "/print.svg") && (!_showDialog || oPrintDlg.exec())) {
        // Actual printing with the help of the painter
        oPainter.begin(&oPrinter);
        // Draw the SVG image
        //oPainter.drawImage(0,0,poSVGPage->renderToImage(oPrinter.resolution(),oPrinter.resolution()));
        oPainter.setRenderHints(QPainter::Antialiasing);
        oRen.render(&oPainter);
        oPainter.end();
    }
}
