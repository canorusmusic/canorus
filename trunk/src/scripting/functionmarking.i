/** @file scripting/functionmarking.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/functionmarking.h"
%}

/**
 * Swig implementation of CAFunctionMarking.
 */
%rename(FunctionMarking) CAFunctionMarking;
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
			T=8,			//Tonic
			S=9,			//Subdominant
			D=10,			//Dominant
			F=11,			//Phrygian (F for Frigio in Italian)
			N=12,			//Napolitan
			L=13			//Lidian
		};
		
		//addedDegrees and alteredDegrees are generated from alterations parameter
		CAFunctionMarking(CAFunctionType function, const QString key, CAFunctionMarkingContext* context, int timeStart, int timeLength, bool minor, CAFunctionType chordArea, bool chordAreaMinor, CAFunctionType tonicDegree, bool tonicDegreeMinor, const QString alterations, bool ellipseSequence);
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
};