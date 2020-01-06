/*!
	Copyright (c) 2006-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CANORUSMLEXPORT_H_
#define CANORUSMLEXPORT_H_

#include <QColor>
#include <QDomElement>

#include "export/export.h"
#include "score/diatonickey.h"
#include "score/diatonicpitch.h"
#include "score/playablelength.h"

class CAMusElement;
class CAFiguredBassContext;

class CACanorusMLExport : public CAExport {
public:
    CACanorusMLExport(QTextStream* stream = nullptr);
    virtual ~CACanorusMLExport();

    void exportDocumentImpl(CADocument* doc);

private:
    using CAExport::exportVoiceImpl;
    void exportVoiceImpl(CAVoice* voice, QDomElement& dVoice);
    void exportFiguredBass(CAFiguredBassContext* c, QDomElement& domParent);
    void exportMarks(CAMusElement* associatedElt, QDomElement& domParent);
    void exportPlayableLength(CAPlayableLength l, QDomElement& domParent);
    void exportDiatonicPitch(CADiatonicPitch p, QDomElement& domParent);
    void exportDiatonicKey(CADiatonicKey k, QDomElement& domParent);
    void exportColor(CAMusElement* elt, QDomElement& domParent);
    void exportTime(CAMusElement* elt, QDomElement& domParent);
    void exportResources(CADocument*, QDomElement&);

    QDomElement _dTuplet;
    QColor _color; // foreground color of elements
};

#endif /* CANORUSMLEXPORT_H_ */
