/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/mimedata.h"
#include "core/context.h"

/*!
	Subclass of QMimeData which incorporates list of Music elements for
	copy/paste functionality.
	
	MIME types for Canorus contexts are "application/canorus-contexts".
*/

const QString CAMimeData::CANORUS_MIME_TYPE = "application/canorus-contexts";

CAMimeData::CAMimeData()
 : QMimeData() {
}

CAMimeData::CAMimeData( QList<CAContext*> list )
 : QMimeData() {
	setContexts( list );
}

CAMimeData::~CAMimeData() {
	for (int i=0; i<contexts().size(); i++)
		delete contexts().at(i);
}

QStringList CAMimeData::formats() const {
	QStringList curFormats = QMimeData::formats();
	if ( hasContexts() )
		curFormats << CANORUS_MIME_TYPE;
	return curFormats;
}

bool CAMimeData::hasFormat(const QString format) const {
	return formats().contains( format );
}
