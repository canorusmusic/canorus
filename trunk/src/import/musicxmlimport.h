/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MUSICXMLIMPORT_H_
#define MUSICXMLIMPORT_H_

#include "import/import.h"

class CADocument;
class QDomNode;

class CAMusicXmlImport: public CAImport {
public:
	CAMusicXmlImport( QTextStream *stream=0 );
	CAMusicXmlImport( const QString stream );
	virtual ~CAMusicXmlImport();

private:
	void initMusicXmlImport();
	CADocument* importDocumentImpl();

	void parseNode( QDomNode* );

	CADocument *_document;
};

#endif /* MUSICXMLIMPORT_H_ */
