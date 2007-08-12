/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MIMETYPE_H_
#define MIMETYPE_H_

#include <QMimeData>
#include <QList>
#include <QStringList>

class CAMusElement;

class CAMimeData : public QMimeData {
public:
	CAMimeData();
	CAMimeData( QList<CAMusElement*> list );
	virtual ~CAMimeData();
	
	bool hasFormat(const QString);
	QStringList formats();
	
	inline void setMusElements( QList<CAMusElement*> musElements ) { _musElements = musElements; }
	inline QList<CAMusElement*> musElements() { return _musElements; }
	inline bool hasMusElements() { return _musElements.size(); }
	
	static const QString CANORUS_MIME_TYPE;
private:
	QList<CAMusElement*> _musElements;
};

#endif /* MIMEDATA_H_ */
