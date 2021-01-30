/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CLEF_H_
#define CLEF_H_

#include <QString>

#include "score/muselement.h"
#include "score/staff.h"

class CAContext;

class CAClef : public CAMusElement {
public:
    enum CAPredefinedClefType {
        Undefined = -1,
        Treble,
        Bass,
        French,
        Soprano,
        Mezzosoprano,
        Alto,
        Tenor,
        Baritone,
        Varbaritone,
        Subbass,
        Percussion,
        Tablature
    };

    enum CAClefType {
        F,
        G,
        C,
        PercussionHigh,
        PercussionLow,
        Tab
    };

    CAClef(CAPredefinedClefType type, CAStaff* staff, int time, int offsetInterval = 0);
    CAClef(CAClefType type, int c1, CAStaff* staff, int time, int offset = 0);
    std::shared_ptr<CAMusElement> cloneRealElement(CAContext* context = nullptr);
    std::shared_ptr<CAClef> cloneClef(CAContext* context = nullptr);
    CAStaff* staff() { return static_cast<CAStaff*>(context()); }

    void setPredefinedType(CAPredefinedClefType type);
    CAClefType clefType() { return _clefType; }
    int c1() { return _c1; }
    int centerPitch() { return _centerPitch; }
    int compare(CAMusElement* elt);

    void setClefType(CAClefType type);

    inline void setOffset(int offset)
    {
        _c1 += _offset;
        _c1 -= (_offset = offset);
    }
    inline int offset() { return _offset; }

    static const QString clefTypeToString(CAClefType);
    static CAClefType clefTypeFromString(const QString);

    static int offsetFromReadable(const int offsetInterval);
    static int offsetToReadable(const int offset);

private:
    CAClefType _clefType;
    int _c1; // Location of middle C in the staff from the bottom line up
    int _centerPitch; // Location of the clefs physical center (where the clef's glyph is going to be rendered)
    int _offset;
};
#endif /* CLEF_H_ */
