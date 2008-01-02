/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <QString>

class CAResource {
public:
	enum CAResourceType {
		Image,     // vector or bitmap image or an icon
		Sound,     // sound sample, midi etc.
		Movie,     // movie clip
		Document,  // Canorus, pdf or other score document
		Other      // other resources
	};
	
	CAResource( QString fileName, QString name, bool linked=false, CAResourceType t=Other );
	virtual ~CAResource();
	
	inline void setName( const QString n ) { _name = n; }
	inline const QString name() { return _name; }
	
	inline void setFileName( const QString file ) { _fileName = file; }
	inline const QString fileName() { return _fileName; }
	
	inline void setResourceType( const CAResourceType t ) { _resType = t; }
	inline const CAResourceType resourceType() { return _resType; }
	
	inline void setLinked( bool l ) { _linked = l; }
	inline bool isLinked() { return _linked; }
	
private:
	QString _name;
	QString _fileName;
	CAResourceType _resType;
	bool _linked;
};

#endif /* RESOURCE_H_ */
