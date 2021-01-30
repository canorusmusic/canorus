/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DYNAMIC_H_
#define DYNAMIC_H_

#include "score/mark.h"
#include <QString>

class CANote;

class CADynamic : public CAMark {
public:
    enum CADynamicText {
        ppppp,
        pppp,
        ppp,
        pp,
        p,
        fffff,
        ffff,
        fff,
        ff,
        f,
        fp,
        mf,
        mp,
        rfz,
        sff,
        sf,
        sfz,
        spp,
        sp,
        Custom
    };

    CADynamic(QString text, int volume, CANote* note);
    virtual ~CADynamic();

    std::shared_ptr<CADynamic> cloneDynamic(CAMusElement* elt = nullptr);
    int compare(CAMusElement*);

    inline const QString text() { return _text; }
    inline void setText(const QString t) { _text = t; }
    inline int volume() { return _volume; }
    inline void setVolume(const int v) { _volume = v; }

    static const QString dynamicTextToString(CADynamicText t);
    static CADynamicText dynamicTextFromString(const QString t);

private:
    QString _text;
    int _volume; // volume percantage - from 0% to 100%
};

#endif /* DYNAMIC_H_ */
