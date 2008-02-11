/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CANORUSMLEXPORT_H_
#define CANORUSMLEXPORT_H_

#include <QDomElement>

#include "export/export.h"
#include "core/playablelength.h"
#include "core/diatonicpitch.h"
#include "core/diatonickey.h"

class CAMusElement;

class CACanorusMLExport : public CAExport {
public:
	CACanorusMLExport( QTextStream *stream=0 );
	virtual ~CACanorusMLExport();
	
	void exportDocumentImpl( CADocument *doc );
	
private:
	void exportVoiceImpl( CAVoice* voice, QDomElement &dVoice );
	void exportMarks( CAMusElement *associatedElt, QDomElement& domParent );
	void exportPlayableLength( CAPlayableLength l, QDomElement& domParent );
	void exportDiatonicPitch( CADiatonicPitch p, QDomElement& domParent );
	void exportDiatonicKey( CADiatonicKey k, QDomElement& domParent );
};

#endif /* CANORUSMLEXPORT_H_ */
