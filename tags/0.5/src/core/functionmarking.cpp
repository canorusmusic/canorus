/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/functionmarking.h"
#include "core/functionmarkingcontext.h"
#include "core/mark.h"

/*!
	\class CAFunctionMarking
	\brief Represents a function marking in the score
	
	Function markings are used to describe a harmony flow of the score.
	Current implementation uses a standard European-German nomenclature of harmony.
	
	\todo Current translations of terms are mostly done "by heart". An English/Amercian
	composer or musicologist should translate attributes the best. -Matevz
	
	\sa CADrawableFunctionMarking, CAFunctionMarkingContext
*/

CAFunctionMarking::CAFunctionMarking(CAFunctionType function, bool minor, const QString key, CAFunctionMarkingContext* context, int timeStart, int timeLength, CAFunctionType chordArea, bool chordAreaMinor, CAFunctionType tonicDegree, bool tonicDegreeMinor, const QString alterations, bool ellipseSequence)
 : CAMusElement(context, timeStart, timeLength) {
 	_musElementType = CAMusElement::FunctionMarking;
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

CAFunctionMarking::~CAFunctionMarking() {
}

bool CAFunctionMarking::isSideDegree() {
	if (_function==I ||
	    _function==II ||
	    _function==III ||
	    _function==IV ||
	    _function==V ||
	    _function==VI ||
	    _function==VII)
		return true;
	else
		return false;
}

CAFunctionMarking *CAFunctionMarking::clone() {
	CAFunctionMarking *newElt;
	newElt = new CAFunctionMarking(function(), isMinor(), key(), (CAFunctionMarkingContext*)_context, timeStart(), timeLength(), chordArea(), isChordAreaMinor(), tonicDegree(), isTonicDegreeMinor(), "", isPartOfEllipse());
	newElt->setAlteredDegrees(_alteredDegrees);
	newElt->setAddedDegrees(_addedDegrees);
	
	for (int i=0; i<markList().size(); i++) {
		CAMark *m = static_cast<CAMark*>(markList()[i]->clone());
		m->setAssociatedElement( newElt );
		newElt->addMark( m );
	}
	
	return newElt;
}

void CAFunctionMarking::clear() {
	setFunction( Undefined );
	setChordArea( Undefined );
	setTonicDegree( T );
	setKey( "C" );
}

int CAFunctionMarking::compare( CAMusElement *func ) {
	int diffs=0;
	
	if (func->musElementType()!=CAMusElement::FunctionMarking)
		return -1;
	
	CAFunctionMarking *fm = static_cast<CAFunctionMarking*>(func);
	if ( fm->function() != function() ) diffs++;
	if ( fm->chordArea()!= chordArea() ) diffs++;
	if ( fm->tonicDegree() != tonicDegree() ) diffs++;
	if ( fm->key() != key() ) diffs++;
	if ( fm->addedDegrees() != addedDegrees()) diffs++;
	if ( fm->alteredDegrees() != alteredDegrees()) diffs++;
	
	return diffs;
}

/*!
	Reads \a alterations and sets alteredDegrees and addedDegrees.
	Sixte ajoutee and other added degrees have +/- sign after the number.
	Stable alterations have +/- sign before the number.
	\a alterations consists first of added degrees and then altered degrees.
*/
void CAFunctionMarking::setAlterations(const QString alterations) {
	if (alterations.isEmpty())
		return;
	
	int i=0;	//index of the first character that belongs to the degree
	int rightIdx;
	
	//added degrees:
	_addedDegrees.clear();
	while (i<alterations.size() && alterations[i]!='+' && alterations[i]!='-') {
		if (alterations.indexOf('+',i+1)==-1)
			rightIdx = alterations.indexOf('-',i+1);
		else if (alterations.indexOf('-',i+1)==-1)
			rightIdx = alterations.indexOf('+',i+1);
		else
			rightIdx = alterations.indexOf('+',i+1)<alterations.indexOf('-',i+1)?alterations.indexOf('+',i+1):alterations.indexOf('-',i+1);
		
		QString curDegree = alterations.mid(i, rightIdx-i+1);
		curDegree.insert(0, curDegree[curDegree.size()-1]);	// move the last + or - before the string
		curDegree.chop(1);
		_addedDegrees << curDegree.toInt();
		i=rightIdx+1;
	}
	
	// altered degrees:
	_alteredDegrees.clear();
	while (i<alterations.size()) {
		if (alterations.indexOf('+',i+1)==-1 && alterations.indexOf('-',i+1)!=-1)
			rightIdx = alterations.indexOf('-',i+1);
		else if (alterations.indexOf('-',i+1)==-1 && alterations.indexOf('+',i+1)!=-1)
			rightIdx = alterations.indexOf('+',i+1);
		else if (alterations.indexOf('-',i+1)!=-1 && alterations.indexOf('+',i+1)!=-1)
			rightIdx = alterations.indexOf('+',i+1)<alterations.indexOf('-',i+1)?alterations.indexOf('+',i+1):alterations.indexOf('-',i+1);
		else
			rightIdx = alterations.size();
		
		_alteredDegrees << alterations.mid(i, rightIdx-i).toInt();
		i=rightIdx;
	}
}

const QString CAFunctionMarking::functionTypeToString(CAFunctionMarking::CAFunctionType type) {
	switch (type) {
		case T:         return "T"; break;
		case S:         return "S"; break;
		case D:         return "D"; break;
		case I:         return "I";	break;
		case II:        return "II"; break;
		case III:       return "III"; break;
		case IV:        return "IV"; break;
		case V:         return "V"; break;
		case VI:        return "VI"; break;
		case VII:       return "VII"; break;
		case N:         return "N";	break;
		case F:         return "F"; break;
		case L:         return "L"; break;
		case K:         return "K"; break;
		case Undefined: return "undefined"; break;	
	}
	
	return "undefined";
}

CAFunctionMarking::CAFunctionType CAFunctionMarking::functionTypeFromString(const QString type) {
	if (type=="T") return T; else
	if (type=="S") return S; else
	if (type=="D") return D; else
	if (type=="I") return I; else
	if (type=="II") return II; else
	if (type=="III") return III; else
	if (type=="IV") return IV; else
	if (type=="V") return V; else
	if (type=="VI") return VI; else
	if (type=="VII") return VII; else
	if (type=="N") return N; else
	if (type=="L") return L; else
	if (type=="F") return F; else
	if (type=="K") return K; else
	if (type=="undefined") return Undefined;
	
	return Undefined;
}

/*!
	\enum CAFunctionMarking::CAFunctionType
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
