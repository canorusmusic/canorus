/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MARK_H_
#define MARK_H_

#include "core/muselement.h"

class CAMark : public CAMusElement {
public:
	enum CAMarkType {
		Undefined = -1,
		Text,
		Tempo,
		Ritardando,
		Dynamic,
		Crescendo,
		Pedal,
		InstrumentChange,
		BookMark,
		RehersalMark,
		Fermata,
		RepeatMark,
		Articulation,
		Fingering
	};
	
	CAMark( CAMarkType type, CAMusElement *associatedElt, int timeStart=-1, int timeLength=-1 );
	CAMark( CAMarkType type, CAContext *context, int timeStart, int timeLength );
	virtual ~CAMark();
	
	virtual CAMusElement *clone();
	virtual int compare( CAMusElement* elt );
	
	inline CAMusElement *associatedElement() { return _associatedElt; }
	inline void setAssociatedElement( CAMusElement* elt ) { _associatedElt = elt; }
	
	inline CAMarkType markType() { return _markType; }
	inline void setMarkType( CAMarkType type ) { _markType = type; }
	
	inline bool isCommon() { return _common; }
	
	static const QString markTypeToString( CAMarkType t );
	static CAMarkType markTypeFromString( const QString s );
	
protected:
	inline void setCommon( bool c ) { _common=c; }
	
private:
	CAMusElement *_associatedElt;
	CAMarkType    _markType;
	bool          _common;        // is mark assigned to a single element only or the whole chord - depends who deletes it!
};

#endif /* MARK_H_ */
