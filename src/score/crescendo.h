/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CRESCENDO_H_
#define CRESCENDO_H_

#include "score/mark.h"

class CANote;

class CACrescendo : public CAMark {
public:
    enum CACrescendoType {
        Crescendo,
        Decrescendo
    };

    CACrescendo(int finalVolume, CANote* note, CACrescendoType t = Crescendo, int timeStart = -1, int timeLength = -1);
    virtual ~CACrescendo();

    std::shared_ptr<CACrescendo> cloneCrescendo(CAMusElement* elt = nullptr);
    int compare(CAMusElement*);

    inline int finalVolume() { return _finalVolume; }
    inline void setFinalVolume(const int v) { _finalVolume = v; }
    inline CACrescendoType crescendoType() { return _crescendoType; }
    inline void setCrescendoType(CACrescendoType t) { _crescendoType = t; }

    static const QString crescendoTypeToString(CACrescendoType t);
    static CACrescendoType crescendoTypeFromString(const QString r);

private:
    int _finalVolume; // volume percantage - from 0% to 100%
    CACrescendoType _crescendoType;
};

#endif /* CRESCENDO_H_ */
