/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/file.h"

CAFile::CAFile() : QThread() {
	_progress = 0;
	_stream = 0;
}

CAFile::~CAFile() {
}
