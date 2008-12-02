/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef BOOKMARK_H_
#define BOOKMARK_H_

#include <QString>
#include "core/mark.h"

class CABookMark: public CAMark {
public:
	CABookMark( const QString text, CAMusElement *m );
	virtual ~CABookMark();
	
	inline const QString text() { return _text; }
	inline void setText( const QString t ) { _text = t; }
	
	CABookMark* clone(CAMusElement* elt=0);
	int compare(CAMusElement *elt);

private:
	QString _text;
};

#endif /* BOOKMARK_H_ */
