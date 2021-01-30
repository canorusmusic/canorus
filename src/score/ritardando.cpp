/*!
	Copyright (c) 2008-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/ritardando.h"
#include "score/playable.h"

/*!
	\class CARitardando
	\brief Ritardando and Accellerando marks

	Relative tempo marks.

	Ritardando starts with the current tempo and linearily decreases the tempo
	to the final tempo. The original tempo is restored after the ritardando is finished.

	Accellerando does the opposite.
*/

CARitardando::CARitardando(int finalTempo, CAPlayable* p, int timeLength, CARitardandoType t)
    : CAMark(CAMark::Ritardando, p, p->timeStart(), timeLength)
{
    setFinalTempo(finalTempo);
    setRitardandoType(t);
}

CARitardando::~CARitardando()
{
}

std::shared_ptr<CARitardando> CARitardando::cloneRitardando(CAMusElement* elt)
{
    return std::make_shared<CARitardando>(finalTempo(), (elt->isPlayable()) ? static_cast<CAPlayable*>(elt) : nullptr, timeLength(), ritardandoType());
}

int CARitardando::compare(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::Mark)
        return -2;

    if (static_cast<CAMark*>(elt)->markType() != CAMark::Ritardando)
        return -1;

    if (static_cast<CARitardando*>(elt)->finalTempo() != finalTempo())
        return 1;

    if (static_cast<CARitardando*>(elt)->ritardandoType() != ritardandoType())
        return 1;

    return 0;
}

const QString CARitardando::ritardandoTypeToString(CARitardandoType t)
{
    switch (t) {
    case Ritardando:
        return "Ritardando";
    case Accellerando:
        return "Accellerando";
    }
    return "Ritardando";
}

CARitardando::CARitardandoType CARitardando::ritardandoTypeFromString(const QString r)
{
    if (r == "Ritardando") {
        return Ritardando;
    } else if (r == "Accellerando") {
        return Accellerando;
    }
    return Ritardando;
}
