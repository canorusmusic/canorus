/*! 
	Copyright (c) 2007, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef ARCHIVE_H_
#define ARCHIVE_H_

#include <QBuffer>
#include "core/tar.h"
#include <iostream>

class QByteArray;

class CAArchive {
public:
	CAArchive();
	CAArchive(QIODevice& arch);
	qint64 write(QIODevice& dest);
	virtual ~CAArchive();

	// interface to CATar
	inline void addFile(const QString& filename, QIODevice& data) { if(!error()) _tar->addFile(filename, data); }
	inline void addFile(const QString& filename, QByteArray data) { if(!error()) _tar->addFile(filename, data); }
	inline void removeFile(const QString& filename) { if(!error()) _tar->removeFile(filename); }
	inline CAIOPtr file(const QString& filename) { if(!error()) return _tar->file(filename); else return CAIOPtr(new QBuffer());  }
	inline bool error() { return _err ||  _tar->error(); }	
protected:
	static const int CHUNK;
	static const char* VERSION;
	bool _err;
	void parse(QIODevice&);
	int getOS();

	CATar *_tar;
};

#endif /* ARCHIVE_H_ */
