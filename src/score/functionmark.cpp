/*!
	Copyright (c) 2006-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/functionmark.h"
#include "score/functionmarkcontext.h"
#include "score/mark.h"

/*!
	\class CAFunctionMark
	\brief Represents a function mark in the score

	Function marks are used to describe a harmony flow of the score.
	Current implementation uses a standard European-German nomenclature of harmony.

	\todo Current translations of terms are mostly done "by heart". An English/Amercian
	composer or musicologist should translate attributes the best. -Matevz

	\sa CADrawableFunctionMark, CAFunctionMarkContext
*/

CAFunctionMark::CAFunctionMark(CAFunctionType function, bool minor, const CADiatonicKey key, CAFunctionMarkContext* context, int timeStart, int timeLength, CAFunctionType chordArea, bool chordAreaMinor, CAFunctionType tonicDegree, bool tonicDegreeMinor, const QString alterations, bool ellipseSequence)
    : CAMusElement(context, timeStart, timeLength)
{
    _musElementType = CAMusElement::FunctionMark;
    _function = function;
    _tonicDegree = tonicDegree;
    _tonicDegreeMinor = tonicDegreeMinor;
    _key = key;
    _chordArea = chordArea;
    _chordAreaMinor = chordAreaMinor;
    _minor = minor;
    _ellipseSequence = ellipseSequence;

    setAlterations(alterations);
}

CAFunctionMark::~CAFunctionMark()
{
}

bool CAFunctionMark::isSideDegree()
{
    if (_function == I || _function == II || _function == III || _function == IV || _function == V || _function == VI || _function == VII)
        return true;
    else
        return false;
}

std::shared_ptr<CAMusElement> CAFunctionMark::cloneRealElement(CAContext* context)
{
    return cloneFunctionMark(context);
}

std::shared_ptr<CAFunctionMark> CAFunctionMark::cloneFunctionMark(CAContext *context)
{
    std::shared_ptr<CAFunctionMark> newElt;
    newElt = std::make_shared<CAFunctionMark>(function(), isMinor(), key(), static_cast<CAFunctionMarkContext*>(context), timeStart(), timeLength(), chordArea(), isChordAreaMinor(), tonicDegree(), isTonicDegreeMinor(), "", isPartOfEllipse());
    newElt->setAlteredDegrees(_alteredDegrees);
    newElt->setAddedDegrees(_addedDegrees);

    for (int i = 0; i < markList().size(); i++) {
        auto m = (markList()[i]->cloneMark(newElt.get()));
        newElt->addMark(m.get());
    }

    return newElt;
}

void CAFunctionMark::clear()
{
    setFunction(Undefined);
    setChordArea(Undefined);
    setTonicDegree(T);
    setKey(CADiatonicKey("C"));
}

int CAFunctionMark::compare(CAMusElement* func)
{
    int diffs = 0;

    if (func->musElementType() != CAMusElement::FunctionMark)
        return -1;

    CAFunctionMark* fm = static_cast<CAFunctionMark*>(func);
    if (fm->function() != function())
        diffs++;
    if (fm->chordArea() != chordArea())
        diffs++;
    if (fm->tonicDegree() != tonicDegree())
        diffs++;
    if (fm->key() != key())
        diffs++;
    if (fm->addedDegrees() != addedDegrees())
        diffs++;
    if (fm->alteredDegrees() != alteredDegrees())
        diffs++;

    return diffs;
}

/*!
	Reads \a alterations and sets alteredDegrees and addedDegrees.
	Sixte ajoutee and other added degrees have +/- sign after the number.
	Stable alterations have +/- sign before the number.
	\a alterations consists first of added degrees and then altered degrees.
*/
void CAFunctionMark::setAlterations(const QString alterations)
{
    if (alterations.isEmpty())
        return;

    int i = 0; //index of the first character that belongs to the degree
    int rightIdx;

    //added degrees:
    _addedDegrees.clear();
    while (i < alterations.size() && alterations[i] != '+' && alterations[i] != '-') {
        if (alterations.indexOf('+', i + 1) == -1)
            rightIdx = alterations.indexOf('-', i + 1);
        else if (alterations.indexOf('-', i + 1) == -1)
            rightIdx = alterations.indexOf('+', i + 1);
        else
            rightIdx = alterations.indexOf('+', i + 1) < alterations.indexOf('-', i + 1) ? alterations.indexOf('+', i + 1) : alterations.indexOf('-', i + 1);

        QString curDegree = alterations.mid(i, rightIdx - i + 1);
        curDegree.insert(0, curDegree[curDegree.size() - 1]); // move the last + or - before the string
        curDegree.chop(1);
        _addedDegrees << curDegree.toInt();
        i = rightIdx + 1;
    }

    // altered degrees:
    _alteredDegrees.clear();
    while (i < alterations.size()) {
        if (alterations.indexOf('+', i + 1) == -1 && alterations.indexOf('-', i + 1) != -1)
            rightIdx = alterations.indexOf('-', i + 1);
        else if (alterations.indexOf('-', i + 1) == -1 && alterations.indexOf('+', i + 1) != -1)
            rightIdx = alterations.indexOf('+', i + 1);
        else if (alterations.indexOf('-', i + 1) != -1 && alterations.indexOf('+', i + 1) != -1)
            rightIdx = alterations.indexOf('+', i + 1) < alterations.indexOf('-', i + 1) ? alterations.indexOf('+', i + 1) : alterations.indexOf('-', i + 1);
        else
            rightIdx = alterations.size();

        _alteredDegrees << alterations.mid(i, rightIdx - i).toInt();
        i = rightIdx;
    }
}

const QString CAFunctionMark::functionTypeToString(CAFunctionMark::CAFunctionType type)
{
    switch (type) {
    case T:
        return "T";
    case S:
        return "S";
    case D:
        return "D";
    case I:
        return "I";
    case II:
        return "II";
    case III:
        return "III";
    case IV:
        return "IV";
    case V:
        return "V";
    case VI:
        return "VI";
    case VII:
        return "VII";
    case N:
        return "N";
    case F:
        return "F";
    case L:
        return "L";
    case K:
        return "K";
    case Undefined:
        return "undefined";
    }

    return "undefined";
}

CAFunctionMark::CAFunctionType CAFunctionMark::functionTypeFromString(const QString type)
{
    if (type == "T")
        return T;
    else if (type == "S")
        return S;
    else if (type == "D")
        return D;
    else if (type == "I")
        return I;
    else if (type == "II")
        return II;
    else if (type == "III")
        return III;
    else if (type == "IV")
        return IV;
    else if (type == "V")
        return V;
    else if (type == "VI")
        return VI;
    else if (type == "VII")
        return VII;
    else if (type == "N")
        return N;
    else if (type == "L")
        return L;
    else if (type == "F")
        return F;
    else if (type == "K")
        return K;
    else if (type == "undefined")
        return Undefined;

    return Undefined;
}

/*!
	\enum CAFunctionMark::CAFunctionType
	Name of the function (tonic, subdominant, etc.), its chord area or the tonic degree.
	Possible names are:
	- Undefined
		no degree - extend the previous one
	- I
		1st (usually never used)
	- II
		2nd
	- III
		3rd
	- IV
		4th (no subdominant leading tone in minor key)
	- V
		5th (no dominant leading tone in minor key)
	- VI
		6th
	- VII
		7th
	- T
		Tonic
	- S
		Subdominant
	- D
		Dominant
	- F
		Phrygian (F for Frigio in Italian)
	- N
		Napolitan
	- L
		Lidian
	- K
		Cadenze chord (see http://en.wikipedia.org/wiki/Cadence_%28music%29). K stands for "kadenze" in German. This is standard nomenclature.
*/
