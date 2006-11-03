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
			T,			//Tonic
			S,			//Subdominant
			D,			//Dominant
			F,			//Phrygian (F for Frigio in Italian)
			N,			//Napolitan
			L			//Lidian
		};
		//TODO: No default parameters implemented yet - problems with Swig's QString typemap something :( -Matevz
		CAFunctionMarking(CAFunctionType function, const QString key, CAFunctionMarkingContext* context, int timeStart, int timeLength, CAFunctionType chordArea, bool chordAreaMinor, CAFunctionType tonicDegree, bool minor, bool ellipseSequence);
		CAFunctionMarking *clone();
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
		
		int compare(CAMusElement *function);
};