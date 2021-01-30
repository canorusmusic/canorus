/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef FUNCTIONMARKCONTEXT_H_
#define FUNCTIONMARKCONTEXT_H_

#include <QList>
#include <QString>

#include "score/context.h"

class CASheet;
class CAFunctionMark;

class CAFunctionMarkContext : public CAContext {
public:
    CAFunctionMarkContext(const QString name, CASheet* sheet);
    ~CAFunctionMarkContext();

    std::shared_ptr<CAContext> cloneRealContext(CASheet*);
    std::shared_ptr<CAFunctionMarkContext> cloneFMC(CASheet* s);

    inline const QList<CAFunctionMark*>& functionMarkList() { return _functionMarkList; }
    QList<CAFunctionMark*> functionMarkAt(int timeStart);
    void addFunctionMark(CAFunctionMark* mark, bool replace = true);
    void addEmptyFunction(int timeStart, int timeLength);

    void repositFunctions();

    void clear();
    CAMusElement* next(CAMusElement* elt);
    CAMusElement* previous(CAMusElement* elt);
    bool remove(CAMusElement* elt);

private:
    QList<CAFunctionMark*> _functionMarkList;
};
#endif /* FUNCTIONMARKCONTEXT_H_*/
