/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef RITARDANDO_H_
#define RITARDANDO_H_

#include "score/mark.h"

class CAPlayable;

class CARitardando : public CAMark {
public:
    enum CARitardandoType {
        Ritardando,
        Accellerando
    };

    CARitardando(int finalTempo, CAPlayable* p, int timeLength, CARitardandoType t = Ritardando);
    virtual ~CARitardando();

    std::shared_ptr<CARitardando> cloneRitardando(CAMusElement* elt = nullptr);
    int compare(CAMusElement*);

    inline int finalTempo() { return _finalTempo; }
    inline void setFinalTempo(const int t) { _finalTempo = t; }
    inline CARitardandoType ritardandoType() { return _ritardandoType; }
    inline void setRitardandoType(CARitardandoType t) { _ritardandoType = t; }

    static const QString ritardandoTypeToString(CARitardandoType t);
    static CARitardandoType ritardandoTypeFromString(const QString r);

private:
    int _finalTempo; // tempo bpm at the end
    CARitardandoType _ritardandoType;
};

#endif /* RITARDANDO_H_ */
