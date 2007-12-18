/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef ARTICULATION_H_
#define ARTICULATION_H_

#include "core/mark.h"
#include "core/note.h"

class CAArticulation : public CAMark {
public:
	enum CAArticulationType {
		Accent,
		Marcato,
		Staccatissimo,
		Espressivo,
		Staccato,
		Tenuto,
		Portato,
		UpBow,
		DownBow,
		Flageolet,
		LHeel,
		RHeel,
		LToe,
		RToe,
		Open,
		Stopped,
		Turn,
		ReverseTurn,
		Trill,
		Prall,
		Mordent,
		PrallPrall,
		PrallMordent,
		UpPrall,
		DownPrall,
		UpMordent,
		DownMordent,
		PrallDown,
		PrallUp,
		LinePrall,
		Signumcongruentiae
	};
	
	CAArticulation( CANote *n );
	virtual ~CAArticulation();
	
	inline CANote *associatedNote() { return static_cast<CANote*>(associatedElement()); }
	inline void *setAssociatedNote( CANote* n ) { setAssociatedElement(n); }
	
	inline CAArticulationType articulationType() { return _articulationType; }
	inline void setArticulationType( CAArticulationType t ) { _articulationType = t; }
	
private:
	CAArticulationType _articulationType;
};

#endif /* ARTICULATION_H_ */
