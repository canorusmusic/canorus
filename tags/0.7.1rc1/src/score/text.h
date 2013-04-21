/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef TEXT_H_
#define TEXT_H_

#include <QString>
#include "score/mark.h"

class CAText : public CAMark {
public:
	CAText( const QString text, CAPlayable *m );
	virtual ~CAText();

	inline const QString text() { return _text; }
	inline void setText( const QString t ) { _text = t; }

	CAText* clone(CAMusElement* elt=0);
	int compare(CAMusElement *elt);

private:
	QString _text;
};

#endif /* TEXT_H_ */
