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
		enum CADegreeType {
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
			N			//Napolitan
		};
		
		CAFunctionMarking(CADegreeType degree, QString key, CAFunctionMarkingContext* context, int timeStart, int timeLength);
		~CAFunctionMarking();
		
		CADegreeType degree() { return _degree; }
		QString key() { return _key; }
		CADegreeType chordArea() { return _chordArea; }
		CADegreeType sideDegree() { return _chordArea; }
		QList<int> alteredDegrees() { return _alteredDegrees; }
		void setDegree(CADegreeType degree) { _degree = degree; }
		void setKey(QString key) { _key = key; }
		void setChordArea(CADegreeType chordArea) { _chordArea = chordArea; }
		void setSideDegree(CADegreeType sideDegree) { _sideDegree = sideDegree; }
		void setAlteredDegrees(QList<int> degrees) { _alteredDegrees = degrees; }
		void setMinor(bool minor) { _minor = minor; }
		
		bool isSideDegree();
		bool isMinor() { return _minor; }
	
	private:
		CADegreeType _degree;		//degree of the function in the Key
		QString _key;				//C for C-Major, g for g-minor, bes for b-flat-minor, Fis for F-sharp-Major etc.
		CADegreeType _chordArea;	//side degrees have undetermined chord locations (eg. 6th can be treated as chord of Subdominant or Tonic)
		CADegreeType _sideDegree;	//when doing jumps to familiar keys, chords on side degrees are made
		QList<int> _alteredDegrees;	//degree of the chord which are altered according to the current key. These markings are usually written below the function name, eg. -3, -7 for German chord
		bool _minor;				//should the function have a circle drawn?
};

#endif /*FUNCTIONMARKING_H_*/
