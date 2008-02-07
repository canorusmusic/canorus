/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/interval.h"

CAInterval::CAInterval( int qlt, int qnt ) {
	setQuality( qlt );
	setQuantity( qnt );
}
