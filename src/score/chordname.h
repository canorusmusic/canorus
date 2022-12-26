/*!
	Copyright (c) 2019-2022, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CHORDNAME_H_
#define CHORDNAME_H_

#include "score/diatonicpitch.h"
#include "score/muselement.h"
#include <QString>

class CAChordNameContext;

class CAChordName : public CAMusElement {
public:
    CAChordName(CADiatonicPitch pitch, QString qualityModifier, CAChordNameContext* parent, int timeStart, int timeLength);
    virtual ~CAChordName();

    CAChordName& operator+=(CAInterval);

    CADiatonicPitch diatonicPitch() { return _diatonicPitch; }
    void setDiatonicPitch(CADiatonicPitch dp) { _diatonicPitch = dp; }

    QString qualityModifier() { return _qualityModifier; }
    void setQualityModifier(QString qm) { _qualityModifier = qm; }

    CAChordName* clone(CAContext* c);
    int compare(CAMusElement* elt);

    bool importFromString(const QString& text);

private:
    CADiatonicPitch _diatonicPitch;
    QString _qualityModifier;
};

#endif /* CHORDNAME_H_ */
