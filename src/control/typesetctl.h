/*!
        Copyright (c) 2006-2019, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef TYPESET_CTL_H
#define TYPESET_CTL_H

// Includes
#include <QObject>
#include <QStringList>
#include <QTemporaryFile>
#include <QVariant>
#include <QVector>

#include <memory>

// Forward declarations
class CAExternProgram;
class CAExport;
class CADocument;
class CASheet;

class CATypesetCtl : public QObject {
    Q_OBJECT

public:
    CATypesetCtl();
    ~CATypesetCtl();

    void setTypesetter(const QString& roProgramName, const QString& roProgramPath = "");
    void setPS2PDF(const QString& roProgrammName, const QString& roProgramPath = "",
        const QStringList& roParams = (QStringList() << QString("")));
    virtual void setExpOption(const QVariant& roName, const QVariant& roValue);
    virtual void setTSetOption(const QVariant& roName, const QVariant& roValue, bool bSpace = false, bool bShortParam = true);
    inline void setPDFConversion(bool bConversion) { _bPDFConversion = bConversion; }
    inline void setExporter(CAExport* poExport) { _poExport = poExport; }
    // Attention: .pdf automatically added and removed if it was added internally
    void exportDocument(CADocument* poDoc);
    void exportSheet(CASheet* poSheet);
    void runTypesetter();

    inline bool getPDFConversion() { return _bPDFConversion; }
    inline CAExport* getExporter() { return _poExport; }
    inline QString getTempFilePath() { return _oOutputFileName; }
    bool waitForFinished(int iMSecs);

signals:
    void nextOutput(const QByteArray& roData);
    void nextStep();
    void typesetterFinished(int iExitCode);

protected slots:
    void rcvTypesetterOutput(const QByteArray& roData);
    void typsetterExited(int iExitCode);

protected:
    bool createPDF();

    std::unique_ptr<CAExternProgram> _poTypesetter; // Transforms exported file to pdf / postscript
    std::unique_ptr<CAExternProgram> _poConvPS2PDF; // Transforms postscripts files to pdf if needed
    CAExport* _poExport; // Transforms canorus document to typesetter format
    QVector<QVariant> _oExpOptList; // List of options for export
    QVector<QVariant> _oTSetOptList; // List of options for typesetter
    std::unique_ptr<QTemporaryFile> _poOutputFile; // Output file for pdf (also used for exported file)
    QString _oOutputFileName; // Output file name for pdf (temporary file deletes it on close)
    bool _bPDFConversion; // Do a conversion from postscript to pdf
    bool _bOutputFileNameFirst; // File name as first parameter ? (Default: No)
};

#endif // TYPESET_CTL_H
