/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/figuredbassmark.h"
#include "score/figuredbasscontext.h"

CAFiguredBassMark::CAFiguredBassMark( CAFiguredBassContext *c, int timeStart, int timeLength )
 : CAMusElement(c, timeStart, timeLength) {
	setMusElementType( FiguredBassMark );

}

CAFiguredBassMark::~CAFiguredBassMark() {
	if (_context) {
		_context->remove( this );
	}
}

/*!
	Adds or replaces the given figured bass \a number.
	The new number will not contain any accidentals.
	Allowed values are from 1 to 15.
*/
void CAFiguredBassMark::addNumber( int number ) {
	insertNumber( number );

	if (_accs.contains(number)) {
		_accs.remove(number);
	}
}

/*!
	Adds or replaces the given figured bass \a number.
	Allowed values are from 0 (no number) to 15.

	Show the accidental \a accs:
	- +1 for one sharp
	- +2 for two sharps
	-  0 for neutral
	- -1 for one flat
	- -2 for two flats
 */
void CAFiguredBassMark::addNumber( int number, int accs ) {
	insertNumber( number );

	_accs[number] = accs;
}

/*!
	Removes the given figured bass \a number and its accidental.
*/
void CAFiguredBassMark::removeNumber( int number ) {
	_numbers.removeAll(number);
	_accs.remove(number);
}

/*!
	Inserts or replaces the given \a number into a sorted list of numbers.
*/
void CAFiguredBassMark::insertNumber( int number ) {
	if (!_numbers.contains(number)) {
		// numbers must be sorted
		int i;
		for (i=0;i<_numbers.size() && _numbers[i]<number; i++);
		_numbers.insert(i, number);
	}
}

CAMusElement* CAFiguredBassMark::clone(CAContext* context) {
	if (context && context->contextType() != CAContext::FiguredBassContext) {
		return 0;
	}

	CAFiguredBassMark *fbm = new CAFiguredBassMark( static_cast<CAFiguredBassContext*>(context), timeStart(), timeLength() );
	for (int i=0; i<_numbers.size(); i++) {
		if (_accs.contains(_numbers[i])) {
			fbm->addNumber(_numbers[i], _accs[_numbers[i]]);
		} else {
			fbm->addNumber(_numbers[i]);
		}
	}

	return fbm;
}

int CAFiguredBassMark::compare(CAMusElement *elt) {
	if ( elt->musElementType()!=CAMusElement::FiguredBassMark ) {
		return -1;
	}

	int diff = 0;
	CAFiguredBassMark *other = static_cast<CAFiguredBassMark*>(elt);

	diff += qAbs( other->numbers().size()-numbers().size() );
	if (diff) {
		return diff;
	}

	for (int i=0; i<numbers().size(); i++) {
		if ( numbers()[i]!=other->numbers()[i] ) {
			diff++;
		}
		if ( accs()[numbers()[i]]!=other->accs()[other->numbers()[i]] ) {
			diff++;
		}
	}

	return diff;
}
