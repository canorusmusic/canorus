/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DYNAMIC_H_
#define DYNAMIC_H_

#include <QString>
#include "core/mark.h"

class CANote;

class CADynamic: public CAMark {
public:
	CADynamic( QString text, int volume, CANote *note );
	virtual ~CADynamic();
	
	inline const QString text() { return _text; }
	inline void setText( const QString t ) { _text = t; }
	inline int volume() { return _volume; }
	inline void setVolume( int v ) { _volume = v; }
	
private:
	QString _text;
	int _volume;
};

#endif /* DYNAMIC_H_ */
