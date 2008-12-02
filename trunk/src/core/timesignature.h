/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef TIMESIGNATURE_H_
#define TIMESIGNATURE_H_

#include <QString>

#include "core/muselement.h"

class CAStaff;
class CAContext;

class CATimeSignature : public CAMusElement {
	public:
		enum CATimeSignatureType {
			Classical,	// Ordinary numbers, C for 4/4, C| for 2/2
			Number,		// Force to always show numbers!
			Mensural,
			Neomensural,
			Baroque
		};
		
		CATimeSignature(int beats, int beat, CAStaff *staff, int startTime, CATimeSignatureType type = Classical);
		
		CATimeSignature *clone(CAContext* context=0);
		~CATimeSignature();
	
		int beats() { return _beats; }
		void setBeats(int beats) { _beats = beats; }
		
		int beat() { return _beat; }
		void setBeat(int beat) { _beat = beat; }
		
		CATimeSignatureType timeSignatureType() { return _timeSignatureType; }
		
		const QString timeSignatureML();  // Deprecated
		const QString timeSignatureTypeML(); // Deprecated
		
		static const QString timeSignatureTypeToString(CATimeSignatureType);
		static CATimeSignatureType timeSignatureTypeFromString(const QString);
		
		int compare(CAMusElement *elt);
		
	private:
		int _beats;
		int _beat;
		CATimeSignatureType _timeSignatureType;
};
#endif /*TIMESIGNATURE_H_*/
