/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/resourcecontainer.h"

#include <QTemporaryFile>

/*!
	\class CAResourceContainer
	\brief Resource manipulation class inherited by CADocument, CASheet etc.
	
	This class is used for creation of new resources out of files on the disk.
	Resources are rarely created directly, but are almost always part of the document
	(eg. audio file of the whole score), sheet (eg. audio file of a theme),
	staff (eg. svg images in contemporary music), voice etc.
	This class is inherited by CADocument, CASheet etc., so you can simply add a
	resource by calling
	CASheet::addResource( "/home/user/title.jpeg" );
	
	CAResourceContainer takes care of creating copies for non-linked resources.
	It picks a random unique name for a new resource in the system temporary file.
	
	\sa CAResource
*/

/*!
	Default constructor.
*/
CAResourceContainer::CAResourceContainer() {
}

CAResourceContainer::~CAResourceContainer() {
	for (int i=0; i<resourceList().size(); i++) {
		delete resourceList().at(i);
	}
	resourceList().clear();
}

void CAResourceContainer::addResource( QString fileName, bool linked ) {
	if (linked) {
		addResource( new CAResource( fileName, fileName, true ) );
	} else {
		addResource( new CAResource( (new QTemporaryFile())->fileName(), fileName, false ) );
	}
}
