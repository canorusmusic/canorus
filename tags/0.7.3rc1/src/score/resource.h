/*!
	Copyright (c) 2008-2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <QUrl>

class CADocument;

class CAResource {
public:
	enum CAResourceType {
		Undefined=-1, // error
		Image,        // vector or bitmap image or an icon
		Sound,        // sound sample, midi etc.
		Movie,        // movie clip
		Document,     // Canorus, pdf or other score document
		Other         // other resources
	};

	CAResource( QUrl fileName, QString name, bool linked=false, CAResourceType t=Other, CADocument *c=0 );
	virtual ~CAResource();

	inline void setName( const QString n ) { _name = n; }
	inline const QString name() { return _name; }

	inline void setDescription( const QString n ) { _description = n; }
	inline const QString description() { return _description; }

	inline void setUrl( const QUrl url ) { _url = url; }
	inline const QUrl url() { return _url; }

	inline void setResourceType( const CAResourceType t ) { _resType = t; }
	inline const CAResourceType resourceType() { return _resType; }

	inline void setLinked( bool l ) { _linked = l; }
	inline bool isLinked() { return _linked; }

	inline void setDocument( CADocument *d ) { _document = d; }
	inline CADocument *document() { return _document; }

	bool copy( QString fileName );

	static QString resourceTypeToString( CAResourceType type );
	static CAResourceType resourceTypeFromString( QString type );

private:
	QString _name;
	QString _description;
	QUrl _url;               // Absolute path to resource. Becomes relative if "file://" scheme when saved.
	CAResourceType _resType;
	bool _linked;
	CADocument *_document;
};

#endif /* RESOURCE_H_ */
