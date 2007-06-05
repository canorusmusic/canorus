/** @file drawable/drawablefunctionmarkingcontext.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef DRAWABLEFUNCTIONMARKINGCONTEXT_H_
#define DRAWABLEFUNCTIONMARKINGCONTEXT_H_

#include "drawable/drawablecontext.h"

class CAFunctionMarkingContext;

class CADrawableFunctionMarkingContext : public CADrawableContext {
	public:
		CADrawableFunctionMarkingContext(CAFunctionMarkingContext *c, int x, int y, int numberOfLines=2);
		~CADrawableFunctionMarkingContext();
		
		void draw(QPainter *p, const CADrawSettings s);
		CADrawableFunctionMarkingContext *clone();
		
		void setNumberOfLines(int number) { _numberOfLines = number; }
		int numberOfLines() { return _numberOfLines; }
		
		enum CAFunctionMarkingLine {
			Upper,	//used for function name in tonicization
			Middle,	//used for general function names
			Lower	//used for chord areas, ellipse sign etc.
		};
		
		int yPosLine(CAFunctionMarkingLine part);	///Return the Y coordinate of the top of the given line
		void nextLine() { _currentLineIdx = ++_currentLineIdx % _numberOfLines; }
		int currentLineIdx() { return _currentLineIdx; }
	
	private:
		int _numberOfLines;	///Number of lines the context can consist. Usually, this number is 2. But when doing research on scores, this could be expanded
		int _currentLineIdx;
};

#endif /*DRAWABLEFUNCTIONMARKINGCONTEXT_H_*/
