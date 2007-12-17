/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef FINGERING_H_
#define FINGERING_H_

#include <QList>
#include "core/mark.h"

class CANote;

class CAFingering: public CAMark {
public:
	CAFingering( int finger, CANote *m, bool italic=false );
	CAFingering( QList<int> fingers, CANote *m, bool italic=false );
	virtual ~CAFingering();
	
	inline int finger()                   { return _fingerList[0]; }
	inline QList<int>& fingerList()       { return _fingerList; }
	inline void addFinger( int f )        { _fingerList << f; }
	inline void removeFingerAt( int idx ) { _fingerList.removeAt(idx); }
	inline bool isItalic() { return _italic; }
	inline void setItalic( bool i ) { _italic = i; }
	
private:
	QList<int> _fingerList;
	bool _italic;
};

#endif /* FINGERING_H_ */
