/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/mimedata.h"

/*!
	Subclass of QMimeData which incorporates list of Music elements for
	copy/paste functionality.
	
	MIME types for Canorus music elements are "application/canorus-muselements".
*/

const QString CAMimeData::CANORUS_MIME_TYPE = "application/canorus-muselements";

CAMimeData::CAMimeData()
 : QMimeData() {
}

CAMimeData::CAMimeData( QList<CAMusElement*> list )
 : QMimeData() {
	setMusElements( list );
}

CAMimeData::~CAMimeData() {
}

QStringList CAMimeData::formats() {
	QStringList curFormats = QMimeData::formats();
	if ( hasMusElements() )
		curFormats << CANORUS_MIME_TYPE;
	return curFormats;
}

bool CAMimeData::hasFormat(const QString format) {
	return formats().contains( format );
}
