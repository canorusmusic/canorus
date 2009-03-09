/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef TRANSPOSE_H_
#define TRANSPOSE_H_

#include <QList>
#include <QSet>

#include "core/interval.h"
#include "core/diatonickey.h"

class CAMusElement;
class CASheet;
class CAContext;

class CATranspose {
public:
	CATranspose();
	CATranspose( CASheet *sheet );
#ifndef SWIG
	CATranspose( QList<CAContext*> contexts );
#endif
	CATranspose( QList<CAMusElement*> selection );
	~CATranspose();

	void transposeBySemitones( int semitones );
	void transposeByInterval( CAInterval );
	void transposeByKeySig( CADiatonicKey from, CADiatonicKey to, int direction );

	void addSheet( CASheet *s );
	void addContext( CAContext *context );
	void addMusElement( CAMusElement *musElt) { _elements << musElt; }

private:
	QSet<CAMusElement*> _elements;
};

#endif /* TRANSPOSE_H_ */
