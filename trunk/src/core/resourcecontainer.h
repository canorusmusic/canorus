/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef RESOURCECONTAINER_H_
#define RESOURCECONTAINER_H_

#include <QString>
#include <QList>

#include "core/resource.h"

class CAResourceContainer {
public:
	CAResourceContainer();
	virtual ~CAResourceContainer();
	
	void addResource( QString fileName, bool linked );
	inline void addResource( CAResource* r ) { _resourceList << r; }
	inline void deleteResource( CAResource* r ) { removeResource(r); delete r; }
	inline void removeResource( CAResource* r ) { _resourceList.removeAll(r); }
	inline QList<CAResource*>& resourceList() { return _resourceList; }
	
private:
	QList<CAResource*> _resourceList;
};

#endif /* RESOURCECONTAINER_H_ */
