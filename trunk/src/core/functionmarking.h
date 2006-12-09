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
			Undefined=0,//no degree - extend the previous one
			I=1,		//1st
			II=2,		//2nd
			III=3,		//3rd
			IV=4,		//4th
			V=5,		//5th
			VI=6,		//6th
			VII=7,		//7th
			T=8,			//Tonic
			S=9,			//Subdominant
			D=10,			//Dominant
			F=11,			//Phrygian (F for Frigio in Italian)
			N=12,			//Napolitan
			L=13			//Lidian
		};
		
		//addedDegrees and alteredDegrees are generated from alterations parameter
		CAFunctionMarking(CAFunctionType function, const QString key, CAFunctionMarkingContext* context, int timeStart, int timeLength, bool minor, CAFunctionType chordArea=Undefined, bool chordAreaMinor=false, CAFunctionType tonicDegree=Undefined, bool tonicDegreeMinor=false, const QString alterations="", bool ellipseSequence=false);
		CAFunctionMarking* clone();
		~CAFunctionMarking();
		
		CAFunctionType function() { return _function; }
		QString key() { return _key; }
		CAFunctionType chordArea() { return _chordArea; }
		CAFunctionType tonicDegree() { return _tonicDegree; }
		QList<int> alteredDegrees() { return _alteredDegrees; }
		QList<int> addedDegrees() { return _addedDegrees; }
		void setFunction(CAFunctionType function) { _function = function; }
		void setKey(QString key) { _key = key; }
		void setChordArea(CAFunctionType chordArea) { _chordArea = chordArea; }
		void setChordAreaMinor(bool minor) { _chordAreaMinor = minor; }
		void setTonicDegree(CAFunctionType tonicDegree) { _tonicDegree = tonicDegree; }
		void setTonicDegreeMinor(CAFunctionType minor) { _tonicDegreeMinor = minor; }
		void setAlteredDegrees(QList<int> degrees) { _alteredDegrees = degrees; }
		void setAddedDegrees(QList<int> degrees) { _addedDegrees = degrees; }
		void setMinor(bool minor) { _minor = minor; }
		void setEllipse(bool ellipse) { _ellipseSequence = ellipse; }
		void setAlterations(const QString alterations);	///Read alterations and set alteredDegrees and addedDegrees
		
		bool isSideDegree();
		
		bool isMinor() { return _minor; }
		bool isChordAreaMinor() { return _chordAreaMinor; }
		bool isTonicDegreeMinor() { return _tonicDegreeMinor; }
		bool isPartOfEllipse() { return _ellipseSequence; }
		
		int compare(CAMusElement *function);
		
	private:
		CAFunctionType _function;		///function name
		QString _key;					///C for C-Major, g for g-minor, bes for b-flat-minor, Fis for F-sharp-Major etc.
		CAFunctionType _chordArea;		///side degrees have undetermined chord locations (eg. 6th can be treated as chord of Subdominant or Tonic)
		bool _chordAreaMinor;			///is chord area minor?
		CAFunctionType _tonicDegree;	///used when doing tonicization (see http://en.wikipedia.org/wiki/Tonicization). None, if the tonic degree should be hidden, degree, to be shown.
		bool _tonicDegreeMinor;			///is tonic degree minor?
		QList<int> _alteredDegrees;		///degree of the chord which are altered according to the current key. These markings are usually written below the function name, eg. -3, -7 for German chord
		QList<int> _addedDegrees;		///degrees of the chord which are added to or substracted from the basic. eg. sixte ajoutée in cadence
		bool _minor;					///should the function have a circle drawn?
		bool _ellipseSequence;			///function is part of ellipse?
};

#endif /*FUNCTIONMARKING_H_*/
