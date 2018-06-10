/*!
	Copyright (c) 2018, Matevž Jekovec, Reinhard Katzmann, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MXLIMPORT_H_
#define MXLIMPORT_H_

#include "import/import.h"
#include "import/musicxmlimport.h"
#include "zip/zip.h"

#include <QSharedPointer>
#include <QFileInfo>

class CAMXLImport: public CAMusicXmlImport {
public:
	CAMXLImport( QTextStream *stream=nullptr );
	CAMXLImport( const QString stream );
	virtual ~CAMXLImport();

    inline QTextStream *txtStream() { return _txtStream; }
	inline void setTxtStream( QTextStream *stream ) { _txtStream = stream; }

protected:
	CADocument *importDocumentImpl();

private:
    bool openContainer(const QFileInfo &containerInfo);
    bool readContainerInfo(QString &musicXMLFileName);

	QTextStream *_txtStream=nullptr;
    QString     _zipArchivePath;
};

#endif /* MUSICXMLIMPORT_H_ */
