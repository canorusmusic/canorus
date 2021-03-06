/*!
	Copyright (c) 2008-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef RESOURCECTL_H_
#define RESOURCECTL_H_

#include <QList>
#include <QString>

#include <memory>

#include "score/resource.h"

class CAResourceCtl {
public:
    CAResourceCtl();
    virtual ~CAResourceCtl();

    static std::shared_ptr<CAResource> importResource(QString name, QString fileName, bool isLinked = false, CADocument* parent = nullptr, CAResource::CAResourceType t = CAResource::Other);
    static std::shared_ptr<CAResource> createEmptyResource(QString name, CADocument* parent = nullptr, CAResource::CAResourceType t = CAResource::Other);
    static void deleteResource(std::shared_ptr<CAResource>);
};

#endif /* RESOURCECTL_H_ */
