/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef FINGERING_H_
#define FINGERING_H_

#include <QList>
#include "score/mark.h"

class CANote;

class CAFingering: public CAMark {
public:
	enum CAFingerNumber {
		First  = 1,
		Second = 2,
		Third  = 3,
		Fourth = 4,
		Fifth  = 5,
		Thumb,
		LHeel,
		RHeel,
		LToe,
		RToe,
		Undefined
	};

	CAFingering( CAFingerNumber finger, CANote *m, bool italic=false );
	CAFingering( QList<CAFingerNumber> fingers, CANote *m, bool italic=false );
	virtual ~CAFingering();

	CAFingering *clone(CAMusElement* elt=0);
	int compare(CAMusElement *elt);

	inline CAFingerNumber finger()             { return (_fingerList.size()?_fingerList[0]:Undefined); }
	inline void setFinger(CAFingerNumber f)    { _fingerList.clear(); _fingerList << f; }
	inline QList<CAFingerNumber>& fingerList() { return _fingerList; }
	inline void addFinger( CAFingerNumber f )  { _fingerList << f; }
	inline void removeFingerAt( int idx )      { _fingerList.removeAt(idx); }

	inline bool isOriginal() { return _original; }
	inline void setOriginal( bool original ) { _original = original; }

	static const QString fingerNumberToString( CAFingerNumber n );
	static CAFingerNumber fingerNumberFromString( const QString s );

private:
	QList<CAFingerNumber> _fingerList;
	bool _original;
};

#endif /* FINGERING_H_ */
