/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <QString>

class CASheet;
class CAMusElement;

class CAContext {
public:
	CAContext(CASheet *s, const QString name);
	virtual ~CAContext();
	virtual CAContext* clone( CASheet* ) = 0;
	
	enum CAContextType {
		Staff,
		FunctionMarkingContext,
		LyricsContext
	};
	
	const QString name() { return _name; } 
	void setName(const QString name) { _name = name; }
	
	CAContextType contextType() { return _contextType; }
	
	CASheet *sheet() { return _sheet; }
	void setSheet(CASheet *sheet) { _sheet = sheet; }
	
	virtual void clear() = 0;
	virtual CAMusElement *findNextMusElement(CAMusElement *elt) = 0;
	virtual CAMusElement *findPrevMusElement(CAMusElement *elt) = 0;
	virtual bool removeMusElement(CAMusElement *elt, bool cleanup = true) = 0;
	
protected:
	void setContextType( CAContextType t ) { _contextType = t; }
	
	CASheet *_sheet;
	QString _name;
	CAContextType _contextType;
};
#endif /* CONTEXT_H_ */
