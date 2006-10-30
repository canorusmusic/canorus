/** @file core/functionmarking.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef FUNCTIONMARKING_H_
#define FUNCTIONMARKING_H_

#include <QList>
#include <QString>

#include "core/muselement.h"

class CAFunctionMarkingContext;

//TODO: Current translations are mostly made "by feeling". An English/Amercian composer or musicologist should translate attributes the best. -Matevz
class CAFunctionMarking : public CAMusElement {
	public:
		enum CAFunctionType {
			None=0,		//no degree
			I=1,		//1st
			II=2,		//2nd
			III=3,		//3rd
			IV=4,		//4th
			V=5,		//5th
			VI=6,		//6th
			VII=7,		//7th
			T,			//Tonic
			S,			//Subdominant
			D,			//Dominant
			F,			//Phrygian (F for Frigio in Italian)
			N,			//Napolitan
			L			//Lidian
		};
		
		CAFunctionMarking(CAFunctionType function, QString key, CAFunctionMarkingContext* context, int timeStart, int timeLength, CAFunctionType chordArea=None, CAFunctionType tonicDegree=None, bool minor=false, bool ellipseSequence=false);
		~CAFunctionMarking();
		
		CAFunctionType function() { return _function; }
		QString key() { return _key; }
		CAFunctionType chordArea() { return _chordArea; }
		CAFunctionType tonicDegree() { return _tonicDegree; }
		QList<int> alteredDegrees() { return _alteredDegrees; }
		void setFunction(CAFunctionType function) { _function = function; }
		void setKey(QString key) { _key = key; }
		void setChordArea(CAFunctionType chordArea) { _chordArea = chordArea; }
		void setTonicDegree(CAFunctionType tonicDegree) { _tonicDegree = tonicDegree; }
		void setAlteredDegrees(QList<int> degrees) { _alteredDegrees = degrees; }
		void setMinor(bool minor) { _minor = minor; }
		void setEllipse(bool ellipse) { _ellipseSequence = ellipse; }
		
		bool isSideDegree();
		bool isMinor() { return _minor; }
		bool isPartOfEllipse() { return _ellipseSequence; }
		
	private:
		CAFunctionType _function;		///function name
		QString _key;					///C for C-Major, g for g-minor, bes for b-flat-minor, Fis for F-sharp-Major etc.
		CAFunctionType _chordArea;		///side degrees have undetermined chord locations (eg. 6th can be treated as chord of Subdominant or Tonic)
		CAFunctionType _tonicDegree;	///used when doing tonicization (see http://en.wikipedia.org/wiki/Tonicization). None, if the tonic degree should be hidden, degree, to be shown.
		QList<int> _alteredDegrees;		///degree of the chord which are altered according to the current key. These markings are usually written below the function name, eg. -3, -7 for German chord
		bool _minor;					///should the function have a circle drawn?
		bool _ellipseSequence;			///function is part of ellipse?
};

#endif /*FUNCTIONMARKING_H_*/
