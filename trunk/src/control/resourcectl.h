/*!
	Copyright (c) 2008-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef RESOURCECTL_H_
#define RESOURCECTL_H_

#include <QString>
#include <QList>

#include "core/resource.h"

class CAResourceCtl {
public:
	CAResourceCtl();
	virtual ~CAResourceCtl();

	CAResource *importResource( QString name, QString fileName, CADocument *parent=0, CAResource::CAResourceType t=CAResource::Other );
	CAResource *createEmptyResource( QString name, CADocument *parent=0, CAResource::CAResourceType t=CAResource::Other );
};

#endif /* RESOURCECTL_H_ */
