/** @file timesignature.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef TIMESIGNATURE_H_
#define TIMESIGNATURE_H_

#include "core/muselement.h"

class CAStaff;

class CATimeSignature : public CAMusElement {
	public:
		enum CATimeSignatureType {
			Classical,	///Ordinary numbers, C for 4/4, C| for 2/2
			Number,		///Force to always show numbers!
			Mensural,
			Neomensural,
			Baroque
		};
		
		/**
		 * Create a time signature with a beat beat and number of beats beats.
		 * 
		 * eg. 3/4 time signature should be called new CATimeSignature(3, 4, staff, startTime);
		 * 
		 * @param beats Number of beats in the measure (nominator).
		 * @param beat The measure beat (denominator).
		 * @param staff Pointer to the staff which this time signature belongs to.
		 * @param startTime Start time.
		 */
		CATimeSignature(int beats, int beat, CAStaff *staff, int startTime, CATimeSignatureType type = Classical);
		~CATimeSignature();
	
		int beats() { return _beats; }
		int beat() { return _beat; }
		CATimeSignatureType timeSignatureType() { return _timeSignatureType; }
		
	private:
		int _beats;
		int _beat;
		CATimeSignatureType _timeSignatureType;
};

#endif /*TIMESIGNATURE_H_*/
