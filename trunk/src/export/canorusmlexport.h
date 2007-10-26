/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CANORUSMLEXPORT_H_
#define CANORUSMLEXPORT_H_

#include <QDomElement>

#include "export/export.h"

class CACanorusMLExport : public CAExport {
public:
	CACanorusMLExport( QTextStream *stream=0 );
	virtual ~CACanorusMLExport();
	
	void exportDocumentImpl( CADocument *doc );
	
private:
	void exportVoiceImpl( CAVoice* voice, QDomElement &dVoice );
};

#endif /* CANORUSMLEXPORT_H_ */
