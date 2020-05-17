/*!
	Copyright (c) 2007-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef TEMPO_H_
#define TEMPO_H_

#include "score/mark.h"
#include "score/playable.h"

class CATempo : public CAMark {
public:
    CATempo(CAPlayableLength l, unsigned char bpm, CAMusElement* m);
    virtual ~CATempo();

    std::shared_ptr<CATempo> cloneTempo(CAMusElement* elt = nullptr);
    int compare(CAMusElement* elt);

    inline unsigned char bpm() { return _bpm; }
    inline void setBpm(unsigned char bpm) { _bpm = bpm; }
    inline CAPlayableLength beat() { return _beat; }
    inline void setBeat(CAPlayableLength l) { _beat = l; }

private:
    CAPlayableLength _beat;
    unsigned char _bpm; // beats per minute
};

#endif /* TEMPO_H_ */
