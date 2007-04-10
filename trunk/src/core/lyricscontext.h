/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef LYRICSCONTEXT_H_
#define LYRICSCONTEXT_H_

#include "core/context.h"

#include <QList>

class CASyllable;

class CALyricsContext : public CAContext {
public:
	CALyricsContext(CASheet *s, const QString name);
	~CALyricsContext();
	void clear();
	CAMusElement* findNextMusElement(CAMusElement*);
	CAMusElement* findPrevMusElement(CAMusElement*);
	bool removeMusElement(CAMusElement*, bool);

private:
	QList<CASyllable*> _syllableList;
};

#endif /* LYRICSCONTEXT_H_ */
