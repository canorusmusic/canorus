/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/barline.h"
#include "score/mark.h"
#include "score/staff.h"

/*!
	\class CABarline

	\brief Music element which represents a barline in the score.

	This class represents any type of barlines in the score.
	Barline type is defined in _barlineType.

	\sa CAMusElement
*/

/*!
	Creates a barline of type \a type, its parent \a staff and \a startTime.
	\a timeLength of a barline is always 0.
*/
CABarline::CABarline(CABarlineType type, CAStaff* staff, int startTime)
    : CAMusElement(staff, startTime)
{
    setMusElementType(CAMusElement::Barline);
    setBarlineType(type);
}

/*!
	Destroys the barline.
*/
CABarline::~CABarline()
{
}

std::shared_ptr<CAMusElement> CABarline::cloneRealElement(CAContext* context)
{
    return cloneBarline(context);
}

std::shared_ptr<CABarline> CABarline::cloneBarline(CAContext *context)
{
    std::shared_ptr<CABarline> b = std::make_shared<CABarline>(barlineType(), static_cast<CAStaff*>(context), timeStart());

    for (int i = 0; i < markList().size(); i++) {
        auto m = (markList()[i]->cloneMark(b.get()));
        b->addMark(m.get());
    }

    return b;
}

int CABarline::compare(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::Barline)
        return -1;

    int diffs = 0;
    if (barlineType() != static_cast<CABarline*>(elt)->barlineType())
        diffs++;

    return diffs;
}

/*!
	Converts the given barline's \a type to QString.

	\sa CABarlineType, barlineTypeFromString()
*/
const QString CABarline::barlineTypeToString(CABarlineType type)
{
    switch (type) {
    case Single:
        return "single";
    case Double:
        return "double";
    case End:
        return "end";
    case RepeatOpen:
        return "repeat-open";
    case RepeatClose:
        return "repeat-close";
    case RepeatCloseOpen:
        return "repeat-close-open";
    case Dotted:
        return "dotted";
    default:
        return "";
    }
}

/*!
	Converts the barline's \a type in QString to CABarlineType.

	\sa CABarlineType, barlineTypeToString()
*/
CABarline::CABarlineType CABarline::barlineTypeFromString(const QString type)
{
    if (type == "single")
        return Single;
    else if (type == "double")
        return Double;
    else if (type == "end")
        return End;
    else if (type == "repeat-open")
        return RepeatOpen;
    else if (type == "repeat-close")
        return RepeatClose;
    else if (type == "repeat-close-open")
        return RepeatCloseOpen;
    else if (type == "dotted")
        return Dotted;
    else
        return Single;
}
