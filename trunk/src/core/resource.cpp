/*!
	Copyright (c) 2008-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QFile>
#include <iostream>

#include "core/resource.h"
#include "core/document.h"

/*!
	\class CAResource
	\brief Different resources included in the file

	CAResource is a wrapper for any file attached to the document.
	Resource files are usually the recorded midi files, transcripts of
	the score or a scanned music, audio and video files, score in other
	formats, images in the score etc.

	CAResource contains a valid absolute path (_fileName) to the actual file:
	- If the resource is external (not included in file, but linked), _linked is
	  True and a _fileName is the absolute path of the linked file (or URL).
	- If the resource is internal (saved along the file), _linked is False and a
	  _fileName is an absolute path to the extracted file in the system temporary
	  directory.

	When the resources are saved, internal resources are saved as
	\sa CAResourceContainer
*/

/*!
	Default constructor.
*/
CAResource::CAResource( QUrl url, QString name, bool linked, CAResourceType t, CADocument *parent ) {
	setName(name);
	setUrl(url);
	setLinked(linked);
	setResourceType(t);
	setDocument(parent);
}

CAResource::~CAResource() {
	if (document()) {
		document()->removeResource(this);
	}

	if (!isLinked()) {
		QFile::remove( url().toLocalFile() );
	}
}

/*!
	Copies the resource to the specified \a fileName.
	Overwrites the specified \a fileName, if the file already exists.
 */
bool CAResource::copy( QString fileName ) {
	if ( QFile::exists( fileName ) ) {
		QFile::remove( fileName );
	}

	return QFile::copy( url().toLocalFile(), fileName );
}

/*!
	Converts the given \a type to string. Usually called when saving the resource.
 */
QString CAResource::resourceTypeToString( CAResourceType type ) {
	switch (type) {
	case Image:    return "image";
	case Sound:    return "sound";
	case Movie:    return "movie";
	case Document: return "document";
	case Other:    return "other";
	default:       return "";
	}
}

/*!
	Converts the given string \a type to CAResourceType. Usually called when opening the resource.
 */
CAResource::CAResourceType CAResource::resourceTypeFromString( QString type ) {
	if (type=="image")         { return Image; }
	else if (type=="sound")    { return Sound; }
	else if (type=="movie")    { return Movie; }
	else if (type=="document") { return Document; }
	else if (type=="other")    { return Other; }
	else                       { return Undefined; }
}
