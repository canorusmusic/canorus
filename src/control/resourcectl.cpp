/*!
	Copyright (c) 2008-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "control/resourcectl.h"

#include <iostream> // debug
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QFileInfo>

/*!
	\class CAResourceCtl
	\brief Resource control class

	This class is used for creation of new resources out of files on the disk or
	web URLs.

	Resources are rarely created directly (maybe using the plugin interface), but
	usually are part of the document (eg. audio file of the whole score), sheet
	(eg. audio file of a theme), staff (eg. svg images in contemporary music),
	voice etc.
	You can simply add a resource by calling
	CADocument::addResource( new CAResource( "/home/user/title.jpeg", "My image") );

	CAResourceContainer takes care of creating copies for non-linked resources.
	It picks a random unique name for a new resource in the system temporary file.

	\sa CAResource
*/

/*!
	Default constructor.
*/
CAResourceCtl::CAResourceCtl() {
}

CAResourceCtl::~CAResourceCtl() {
}

CAResource *CAResourceCtl::importResource( QString name, QString fileName, CADocument *parent, CAResource::CAResourceType t ) {
	QTemporaryFile *f = new QTemporaryFile(QDir::tempPath()+"/"+name);
	f->open();
	QString targetFile = QFileInfo(*f).absoluteFilePath();
	f->close();
	delete f;

	QFile::copy( fileName, targetFile );

	return new CAResource( QUrl::fromLocalFile(targetFile), name, false, t, parent );
}

CAResource *CAResourceCtl::createEmptyResource( QString name, CADocument *parent, CAResource::CAResourceType t ) {
	QTemporaryFile f(QDir::tempPath()+"/"+name);
	f.open();
	QString fileName = QFileInfo(f).absoluteFilePath();
	f.close();

	return new CAResource( fileName, name, false, t, parent );
}
