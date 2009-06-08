/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/figuredbasscontext.h"
#include "score/figuredbassmark.h"

CAFiguredBassContext::CAFiguredBassContext( QString name, CASheet *sheet )
 : CAContext(name, sheet) {
	setContextType( FiguredBassContext );
}

CAFiguredBassContext::~CAFiguredBassContext() {
	clear();
}

void CAFiguredBassContext::addFiguredBassMark( CAFiguredBassMark *m ) {
	// TODO
}

void CAFiguredBassContext::repositFiguredBassMarks() {
	// TODO
}

CAContext* CAFiguredBassContext::clone( CASheet* s ) {
	CAFiguredBassContext *newFbc = new CAFiguredBassContext( name(), s );

	for (int i=0; i<_figuredBassMarkList.size(); i++) {
		CAFiguredBassMark *newFbm = static_cast<CAFiguredBassMark*>(_figuredBassMarkList[i]->clone(newFbc));
		newFbc->addFiguredBassMark( newFbm );
	}
	return newFbc;
}

void CAFiguredBassContext::clear() {
	while(!_figuredBassMarkList.isEmpty())
		delete _figuredBassMarkList.takeFirst();
}

CAMusElement* CAFiguredBassContext::next( CAMusElement* elt ) {
	if (elt->musElementType()!=CAMusElement::FiguredBassMark)
		return 0;

	int i = _figuredBassMarkList.indexOf(static_cast<CAFiguredBassMark*>(elt));
	if (i!=-1 && ++i<_figuredBassMarkList.size())
		return _figuredBassMarkList[i];
	else
		return 0;
}

CAMusElement* CAFiguredBassContext::previous( CAMusElement* elt ) {
	if (elt->musElementType()!=CAMusElement::FiguredBassMark)
		return 0;

	int i = _figuredBassMarkList.indexOf(static_cast<CAFiguredBassMark*>(elt));
	if (i!=-1 && --i>-1)
		return _figuredBassMarkList[i];
	else
		return 0;
}

bool CAFiguredBassContext::remove( CAMusElement *elt ) {
	if (!elt || elt->musElementType()!=CAMusElement::FiguredBassMark)
		return false;

	bool success=false;
	success = _figuredBassMarkList.removeAll(static_cast<CAFiguredBassMark*>(elt));

	if(success)
		delete elt;

	return success;
}
