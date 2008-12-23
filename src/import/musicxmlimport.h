/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MUSICXMLIMPORT_H_
#define MUSICXMLIMPORT_H_

#include <QXmlStreamReader>
#include <QString>
#include <QStack>
#include <QHash>
#include <QMultiHash>

#include "core/playablelength.h"
#include "core/diatonicpitch.h"
#include "import/import.h"

class CADocument;
class CASheet;
class CAStaff;
class CAKeySignature;
class CATimeSignature;

class CAMusicXmlImport: public CAImport, private QXmlStreamReader {
public:
	CAMusicXmlImport( QTextStream *stream=0 );
	CAMusicXmlImport( const QString stream );
	virtual ~CAMusicXmlImport();

	const QString readableStatus();

private:
	void initMusicXmlImport();
	CADocument* importDocumentImpl();

	void readHeader();
	void readScorePartwise();
	void readScoreTimewise();
	void readWork();
	void readIdentification();
	void readDefaults();
	void readPartList();
	void readPart();
	void readMeasure( CAStaff* );
	void readAttributes( CAStaff* );
	void readNote( CAStaff*, int );

	QString         _musicXmlVersion;

	CADocument *_document;
	QHash<QString, CAStaff*> _staffMap;
	QHash<CAStaff*, int> _midiChannel;
	QHash<CAStaff*, int> _midiProgram;
	QHash<CAStaff*, int> _divisions;
};

#endif /* MUSICXMLIMPORT_H_ */
