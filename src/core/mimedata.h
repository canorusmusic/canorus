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

class CAContext;

class CAMimeData : public QMimeData {
public:
	CAMimeData();
	CAMimeData( QList<CAContext*> list );
	virtual ~CAMimeData();

	bool hasFormat(const QString) const;
	QStringList formats() const;

	inline void setContexts( QList<CAContext*> list ) { _contexts = list; }
	inline const QList<CAContext*>& contexts() const { return _contexts; }
	inline bool hasContexts() const { return _contexts.size(); }

	static const QString CANORUS_MIME_TYPE;
private:
	QList<CAContext*> _contexts;
};

#endif /* MIMEDATA_H_ */
