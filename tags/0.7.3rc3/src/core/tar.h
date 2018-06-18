/*! 
	Copyright (c) 2007, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef TAR_H_
#define TAR_H_

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QHash>
#include <memory>
using std::unique_ptr;

class QIODevice;

typedef unique_ptr<QIODevice> CAIOPtr;

class CATar
{
public:
	CATar();
	CATar(QIODevice&);
	virtual ~CATar();
	bool addFile(const QString& filename, QIODevice& data, bool replace = true);
	bool addFile(const QString& filename, QByteArray data, bool replace = true);
	void removeFile(const QString& filename);
	inline bool contains(const QString& filename);
	CAIOPtr file(const QString& filename);
	qint64 write(QIODevice& dest, qint64 chunk);
	qint64 write(QIODevice& dest);
	inline bool open(QIODevice& dest) { if(_pos.contains(&dest)) { return false; } _pos[&dest].pos = _pos[&dest].file = _pos[&dest].eof = 0; return true; }
	inline void close(QIODevice& dest) { _pos.remove(&dest); }
	bool eof(QIODevice& dest);
	inline bool error() { return !_ok; }
protected:
	static const int CHUNK;
	typedef struct {			/* size in bytes (ASCII) */	
		char	name[101];		/* 100 */
		quint32	mode;			/* 8   */
		quint32	uid;			/* 8   */
		quint32	gid;			/* 8   */
		quint64	size; 			/* 12  */
		quint64	mtime; 			/* 12  */
		quint32	chksum;			/* 8   */
		char	typeflag;		/* 1   */
		char	linkname[101];	/* 100 */
		/* magic "ustar\0" */	/* 6   */			
		/* version "00" */		/* 2   */
		char	uname[33];		/* 32  */
		char	gname[33];		/* 32  */
		// NULs					/* 16  */
		char	prefix[156];	/* 155 */
		// NULs					/* 12  */
	} CATarHeader;
	typedef struct {
		CATarHeader	hdr;
		QFile*		data;
	} CATarFile;
	QList<CATarFile*> _files;
	void parse(QIODevice& data);
	bool _ok;
	typedef struct {
		qint64 pos;
		qint32 file;
		bool close;
		bool eof;
	} CATarBufInfo;
	QHash<QIODevice*, CATarBufInfo> _pos;
	// helper functions
	char *bufncpy(char*, const char*, size_t, int = -1);
	char *bufncpyi(char*&, const char*, size_t, int = -1);
	char *numToOct(char*, qint64, int);
	char *numToOcti(char*&, qint64, int);
	void writeHeader(QIODevice& dest, int file);
};

#endif /* TAR_H_ */
