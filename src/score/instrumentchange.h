/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef INSTRUMENTCHANGE_H_
#define INSTRUMENTCHANGE_H_

#include "score/mark.h"
#include <QString>

class CANote;

class CAInstrumentChange : public CAMark {
public:
    CAInstrumentChange(int instrument, CANote* note);
    virtual ~CAInstrumentChange();

    CAInstrumentChange* clone(CAMusElement* elt = nullptr);
    int compare(CAMusElement*);

    inline int instrument() { return _instrument; }
    inline void setInstrument(const int instrument) { _instrument = instrument; }

private:
    int _instrument;
};

#endif /* INSTRUMENTCHANGE_H_ */
