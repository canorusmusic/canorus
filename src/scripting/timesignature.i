/** @file scripting/timesignature.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%{
#include "core/timesignature.h"
%}

/**
 * Swig implementation of CATimeSignature.
 */
%rename(TimeSignature) CATimeSignature;
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
		CATimeSignature(QString MLString, CAStaff *staff, int startTime, QString type = "classical");
		~CATimeSignature();
	
		int beats();
		int beat();
		CATimeSignatureType timeSignatureType();
		const QString timeSignatureML();
		const QString timeSignatureTypeML(); 
};
