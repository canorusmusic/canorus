/*!
	Copyright (c) 2006-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/functionmarkcontext.h"
#include "score/functionmark.h"
#include "score/playable.h"
#include "score/sheet.h"

/*!
	\class CAFunctionMarkContext
	\brief Context for function marks

	This class represents a context which holds various function marks.
	As CAStaff is a parent context for CANote, CARest and other staff elements,
	CAFunctionMarkContext is a parent context for CAFunctionMark.

	\sa CAContext
*/

CAFunctionMarkContext::CAFunctionMarkContext(const QString name, CASheet* sheet)
    : CAContext(name, sheet)
{
    _contextType = CAContext::FunctionMarkContext;

    repositionElements();
}

CAFunctionMarkContext::~CAFunctionMarkContext()
{
    clear();
}

CAFunctionMarkContext* CAFunctionMarkContext::clone(CASheet* s)
{
    CAFunctionMarkContext* newFmc = new CAFunctionMarkContext(name(), s);

    for (int i = 0; i < _functionMarkList.size(); i++) {
        CAFunctionMark* newFm = _functionMarkList[i]->clone(newFmc);
        newFmc->addFunctionMark(newFm);
    }

    return newFmc;
}

void CAFunctionMarkContext::clear()
{
    for (int i = 0; i < _functionMarkList.size(); i++)
        delete _functionMarkList[i];

    _functionMarkList.clear();
}

/*!
	Adds an already created function mark to this context.
*/
void CAFunctionMarkContext::addFunctionMark(CAFunctionMark* function, bool replace)
{
    int i;
    for (i = _functionMarkList.size() - 1; i > 0 && _functionMarkList[i]->timeStart() > function->timeStart(); i--)
        ;
    _functionMarkList.insert(i + 1, function);
    if (replace && i < _functionMarkList.size() && i >= 0 && _functionMarkList[i]->isEmpty()) {
        _functionMarkList.removeAt(i);
    } else if (!replace) {
        i++;
        while (++i < _functionMarkList.size())
            _functionMarkList[i]->setTimeStart(_functionMarkList[i]->timeStart() + function->timeLength());
    }
}

CAMusElement* CAFunctionMarkContext::next(CAMusElement* elt)
{
    int idx = _functionMarkList.indexOf(static_cast<CAFunctionMark*>(elt));
    if (idx == -1)
        return nullptr;

    if (++idx >= _functionMarkList.size())
        return nullptr;
    else
        return _functionMarkList[idx];
}

CAMusElement* CAFunctionMarkContext::previous(CAMusElement* elt)
{
    int idx = _functionMarkList.indexOf(static_cast<CAFunctionMark*>(elt));
    if (idx == -1)
        return nullptr;

    if (--idx < 0)
        return nullptr;
    else
        return _functionMarkList[idx];
}

bool CAFunctionMarkContext::remove(CAMusElement* elt)
{
    return _functionMarkList.removeAll(static_cast<CAFunctionMark*>(elt));
}

CAMusElement *CAFunctionMarkContext::insertEmptyElement(int timeStart) {
    CAFunctionMark *newElt = new CAFunctionMark(CAFunctionMark::Undefined, false, CADiatonicKey("C"), this, timeStart, 1);
    addFunctionMark(newElt, false);

    return newElt;
}

/*!
    It repositions the functions (sets timeStart and timeLength) one by one according to the chords
    above the context.

    If two functions contain the same timeStart, they are treated as modulation and will contain
    the same timeStart after reposition is done as well!
*/
void CAFunctionMarkContext::repositionElements()
{
    int ts, tl;
    int curIdx; // contains current position in _functionMarkList
    QList<CAPlayable*> chord;
    for (ts = 0, curIdx = 0;
         (sheet() && (chord = sheet()->getChord(ts)).size()) || curIdx < _functionMarkList.size(); ts += tl) {
        tl = (chord.size() ? chord[0]->timeLength() : 256);
        for (int i = 0; i < chord.size(); i++)
            if (chord[i]->timeLength() < tl)
                tl = chord[i]->timeLength();

        if (curIdx == _functionMarkList.size()) { // add new empty functions, if chords still exist
            insertEmptyElement(ts);
            curIdx++;
        }

        // apply timeStart and timeLength to existing function marks
        for (int startIdx = curIdx; curIdx == 0 || (curIdx < _functionMarkList.size() && _functionMarkList[curIdx]->timeStart() == _functionMarkList[startIdx]->timeStart()); curIdx++) {
            _functionMarkList[curIdx]->setTimeLength(tl);
            _functionMarkList[curIdx]->setTimeStart(ts);
        }
    }
}

/*!
	Returns the function marks at the exact given \a timeStart.
	This function is usually called to determine the number of possible modulations of the
	same chord at the given time.
*/
QList<CAFunctionMark*> CAFunctionMarkContext::functionMarkAt(int timeStart)
{
    int i;
    QList<CAFunctionMark*> ret;

    // seek to the given time
    for (i = 0; i < _functionMarkList.size() && _functionMarkList[i]->timeStart() < timeStart; i++)
        ;

    for (; i < _functionMarkList.size() && _functionMarkList[i]->timeStart() == timeStart; i++) {
        ret << _functionMarkList[i];
    }

    return ret;
}

/*!
	\var CAFunctionMarkContext::_functionMarkList
	List of all the function marks sorted by timeStart
*/
