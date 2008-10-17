/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <QUrl>

class CAResourceContainer;

class CAResource {
public:
	enum CAResourceType {
		Image,     // vector or bitmap image or an icon
		Sound,     // sound sample, midi etc.
		Movie,     // movie clip
		Document,  // Canorus, pdf or other score document
		Other      // other resources
	};

	CAResource( QUrl fileName, QString name, bool linked=false, CAResourceType t=Other, CAResourceContainer *c=0 );
	virtual ~CAResource();

	inline void setName( const QString n ) { _name = n; }
	inline const QString name() { return _name; }

	inline void setUrl( const QUrl url ) { _url = url; }
	inline const QUrl url() { return _url; }

	inline void setResourceType( const CAResourceType t ) { _resType = t; }
	inline const CAResourceType resourceType() { return _resType; }

	inline void setLinked( bool l ) { _linked = l; }
	inline bool isLinked() { return _linked; }

	inline void setResourceContainer( CAResourceContainer *c ) { _resourceContainer = c; }
	inline CAResourceContainer *resourceContainer() { return _resourceContainer; }

	bool copy( QString fileName );

private:
	QString _name;
	QUrl _url;
	CAResourceType _resType;
	bool _linked;
	CAResourceContainer *_resourceContainer;
};

#endif /* RESOURCE_H_ */
