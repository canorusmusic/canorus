/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MUSICXMLEXPORT_H_
#define MUSICXMLEXPORT_H_

#include "export/export.h"

class CAMusicXmlExport : public CAExport {
public:
	CAMusicXmlExport( QTextStream *stream=0 );
	virtual ~CAMusicXmlExport();

private:
	void exportDocumentImpl(CADocument *doc);
};

#endif /* MUSICXMLEXPORT_H_ */
