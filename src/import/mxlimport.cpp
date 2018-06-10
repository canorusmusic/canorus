/*!
	Copyright (c) 2018, Matevž Jekovec, Reinhard Katzmann, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QDir>
#include <QDebug>
#include <iostream> // debug
#include "import/mxlimport.h"

CAMXLImport::CAMXLImport( QTextStream *stream )
 : CAMusicXmlImport(stream) {
}

CAMXLImport::CAMXLImport( const QString stream )
 : CAMusicXmlImport(stream) {
}

CAMXLImport::~CAMXLImport() {
}

CADocument *CAMXLImport::importDocumentImpl()
{
    int arg = 2;
    _zipArchivePath = fileName();
    // Extract whole archive to temp folder
    zip_extract(fileName().toLatin1().constData(), QDir::tempPath().toLatin1().constData(), [](const char *filename, void *arg) {
        static int i = 0;
        int n = *static_cast<int *>(arg);
        qDebug().noquote() << "Extracted: " << filename << "(" << ++i << " of " << n << ")\n";

        return 0;
    }, &arg);

    QFileInfo containerInfo(QDir::tempPath() + QString("/META-INF/container.xml"));
    QString musicXMLFileName;
    bool eocRes = openContainer(containerInfo);
    if (eocRes) {
        eocRes = readContainerInfo(musicXMLFileName);
        QFileInfo musicXMLFileInfo(QDir::tempPath() + "/" + musicXMLFileName);
        if(musicXMLFileInfo.exists()) {
            setStreamFromFile(musicXMLFileInfo.filePath());
            return CAMusicXmlImport::importDocumentImpl();
        }
        qDebug() << "Failed to find musicxml file " << musicXMLFileInfo.filePath() << " in archive";
    }
    return nullptr;
}

bool CAMXLImport::openContainer(const QFileInfo &containerInfo)
{
    if (containerInfo.exists()) {
        setStreamFromFile(containerInfo.filePath());
    } else {
        qDebug() << "Failed to find container file " << containerInfo.filePath() << " in archive";
        return false;
    }
    return true;
}

bool CAMXLImport::readContainerInfo(QString &musicXMLFileName)
{
    QString containerLine, rootFileLine, mediaTypeLine, fullPathLine;
    do {
        containerLine = stream()->readLine();
        if (containerLine.contains("<rootfiles")) {
            do { // No check for <rootfile> to make logic easier (strictly it's required)
                rootFileLine = stream()->readLine();
                if(rootFileLine.contains("full-path")) {
                    fullPathLine = rootFileLine;
                    if (mediaTypeLine.contains("application/vnd.recordare.musicxml+xml")) {
                        break;
                    }
                }
                if (rootFileLine.contains("<rootfile")) {
                    if (rootFileLine.contains("media-type")) {
                        mediaTypeLine = rootFileLine;
                        if (!mediaTypeLine.contains("application/vnd.recordare.musicxml+xml")) {
                            fullPathLine.clear();
                            continue;
                        }
                        if(!fullPathLine.isNull())
                            break;
                    }
                }
            } while (!rootFileLine.isNull());
        }
    } while (!containerLine.isNull());
    if (!mediaTypeLine.contains("application/vnd.recordare.musicxml+xml") || fullPathLine.isNull()) {
        qDebug() << "No musicxml media " << mediaTypeLine << " found in container " << fullPathLine;
        return false;
    }
    QString fullPathStr = "full-path";
    int pos = fullPathLine.lastIndexOf(fullPathStr) + fullPathStr.length();
    int lastPos = fullPathLine.indexOf("\"", pos+2);
    int num = lastPos - pos -2;
    musicXMLFileName = fullPathLine.mid(pos+2, num);
    //qDebug() << "pos " << pos << " num " << num << " last " << lastPos << " fileName " << musicXMLFileName;
    return true;
}
