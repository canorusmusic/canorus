/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLECHORDNAME_H_
#define DRAWABLECHORDNAME_H_

#include "layout/drawablemuselement.h"
#include "score/chordname.h"

class CADrawableChordNameContext;

class CADrawableChordName : public CADrawableMusElement {
public:
    CADrawableChordName(CAChordName*, CADrawableChordNameContext*, double x, double y);
    ~CADrawableChordName();
    void draw(QPainter* p, const CADrawSettings s);
    CADrawableChordName* clone(CADrawableContext* c = 0);

    CAChordName* chordName() { return static_cast<CAChordName*>(musElement()); }

    static const double DEFAULT_TEXT_SIZE;

private:
    QString drawableDiatonicPitch();
};

#endif /* DRAWABLECHORDNAME_H_ */
