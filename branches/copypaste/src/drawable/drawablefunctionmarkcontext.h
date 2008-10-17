/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef DRAWABLEFUNCTIONMARKCONTEXT_H_
#define DRAWABLEFUNCTIONMARKCONTEXT_H_

#include "drawable/drawablecontext.h"

class CAFunctionMarkContext;

class CADrawableFunctionMarkContext : public CADrawableContext {
	public:
		CADrawableFunctionMarkContext(CAFunctionMarkContext *c, int x, int y, int numberOfLines=2);
		~CADrawableFunctionMarkContext();
		
		void draw(QPainter *p, const CADrawSettings s);
		CADrawableFunctionMarkContext *clone();
		
		void setNumberOfLines(int number) { _numberOfLines = number; }
		int numberOfLines() { return _numberOfLines; }
		
		enum CAFunctionMarkLine {
			Upper,	// used for function name in tonicization
			Middle,	// used for general function names
			Lower	// used for chord areas, ellipse sign etc.
		};
		
		int yPosLine(CAFunctionMarkLine part);	// Returns the Y coordinate of the top of the given line
		void nextLine() { _currentLineIdx = ++_currentLineIdx % _numberOfLines; }
		int currentLineIdx() { return _currentLineIdx; }
	
	private:
		int _numberOfLines;	// Number of lines the context can consist. Usually, this number is 2. But when doing research on scores, this could be expanded
		int _currentLineIdx;
};

#endif /* DRAWABLEFUNCTIONMARKCONTEXT_H_ */
