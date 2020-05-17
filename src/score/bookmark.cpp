/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/bookmark.h"

/*!
	\class CABookMark
	\brief A bookmark shortcut

	Arbitrary text above or below *any* element with a shortcut key.
*/

CABookMark::CABookMark(const QString s, CAMusElement* elt)
    : CAMark(CAMark::BookMark, elt)
{
    setText(s);
}

CABookMark::~CABookMark()
{
}

std::shared_ptr<CABookMark> CABookMark::cloneBookMark(CAMusElement* elt)
{
    return std::make_shared<CABookMark>(text(), elt);
}

int CABookMark::compare(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::Mark)
        return -2;

    if (static_cast<CAMark*>(elt)->markType() != CAMark::BookMark)
        return -1;

    if (static_cast<CABookMark*>(elt)->text() != text())
        return 1;

    return 0;
}
