/*! 
	Copyright (c) 2007, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QString>
#include <QBuffer> 
#include <QDateTime>
#include <QTemporaryFile>
#include <QDir>

#include <cmath> // pow()
#include <QDebug>

#include "core/tar.h"

/*!
	\class CATar
	\brief Class for the manipulation of tar files

	This class can create and read tar archives, which allow concatenation of multiple files (with directory structure) into a single file.
	
	The archive must be opened using \a open() before writing, and closed with close() when writing is done. Don't forget to close() the archive when you're done!
	For more info on the Tar format see <http://en.wikipedia.org/wiki/Tar_(file_format)>.

*/

const int CATar::CHUNK = 16384; 

/*!
	Creates an empty tar file
*/
CATar::CATar()
:_ok(true)
{
	/* An empty file is a valid tar */
}

/*!
	Destroys a tar archive
*/
CATar::~CATar()
{
	foreach(CATarFile* t, _files)
	{
		delete t->data; // deletes temp file from disk
		delete t;
	}
}

/*!
	Parse the given tar file and allow reading from it.
*/
CATar::CATar(QIODevice& data)
:_ok(true)
{
	parse(data);
}

/*!
	Parses an existing tar file and initializes this object to represent it.

	Parsing stops when a parsing errors occurs. The files that were parsed until the error will be available.
	error() can tell whether an error ocurred.
*/
void CATar::parse(QIODevice& tar)
{
	bool wasOpen = true;
	int pad;

	if(!tar.isOpen()) {
		tar.open(QIODevice::ReadOnly);
		wasOpen = false;
	}
	
	while(!tar.atEnd()) {
		QByteArray hdrba = tar.read(512);
		CATarFile *file;
		QTemporaryFile *tempfile;
		int chksum, chkchksum = 0;

		if(hdrba.size() < 512) {
			_ok = false; 
			break;
		}

		// Check magic first
		QByteArray magic = hdrba.mid(257, 6);
		QByteArray version = hdrba.mid(263, 2);
		if(magic != QString::fromLatin1("ustar") || version[0] != '0' || version[1] != '0') 
		{
			_ok= false;
			continue;
		}
		QBuffer header(&hdrba);
		
		header.open(QIODevice::ReadOnly);
		file = new CATarFile;
		bufncpy(file->hdr.name, header.read(100).data(), 100);
		file->hdr.mode = header.read(8).toUInt(&_ok, 8);
		if(!_ok) return;
		file->hdr.uid = header.read(8).toUInt(&_ok, 8);
		if(!_ok) return;
		file->hdr.gid = header.read(8).toUInt(&_ok, 8);
		if(!_ok) return;
		file->hdr.size = header.read(12).toULongLong(&_ok, 8);
		if(!_ok) return;
		file->hdr.mtime = header.read(12).toUInt(&_ok, 8);
		if(!_ok) return;
		chksum = header.read(8).toInt(&_ok, 8); // recorded checksum
		if(!_ok) return;
		file->hdr.typeflag = header.read(1)[0];
		bufncpy(file->hdr.linkname, header.read(100).data(), 100);
		header.read(6+2); //magic+header
		bufncpy(file->hdr.uname, header.read(32).data(), 32);
		bufncpy(file->hdr.gname, header.read(32).data(), 32);
		header.read(16); // nulls (devmajor, devminor)
		bufncpy(file->hdr.prefix, header.read(155).data(), 155);

		// get real checksum
		for(int i=0; i<148; i++)
			chkchksum += hdrba[i]; 
		chkchksum += int(' ')*8;
		for(int i=156; i<512; i++)
			chkchksum += hdrba[i];

		if(chkchksum != chksum) {
			delete file;
			continue;
		}
		
		tempfile = new QTemporaryFile;
		tempfile->open();
		file->data = tempfile;
		for(unsigned int i=0; i < file->hdr.size / CHUNK; i++)
			file->data->write(tar.read(CHUNK));
		file->data->write(tar.read(file->hdr.size % CHUNK));
		file->data->flush();
		pad = file->hdr.size%512;
		if(pad>0)
			tar.read(512-pad);
		_files << file;
	}
	if(!wasOpen)
		tar.close();
}

/** 
	Returns true if the tar contains a file with the given filename. Otherwise false
	filename may be a relative path.
*/
bool CATar::contains(const QString& filename)
{
	foreach(CATarFile *t, _files) {
		if(filename == t->hdr.name)
			return true;
	}
	return false;
}

/*!
	Adds a file to the tar archive.

	\param filename The full name of the file (including directory path).
	\param data		A reader for the file.
	\param replace	Whether to replace the file, if it's already in the archive. Default is true.
	\return True if the file has been added, false otherwise (e.g., file with the same name exists).
*/
bool CATar::addFile(const QString& filename, QIODevice& data, bool replace /* = true */)
{
	if(contains(filename)) {
		if(!replace)
			return false;
		else
			removeFile(filename);
	}
	CATarFile *file = new CATarFile;
	
	// Basename only for use with prefix if ever required (see below).
	//QString basename = filename.right(filename.lastIndexOf("/")); // \todo slash in windows? win32 tar?
	//bufncpy(file->hdr.name, basename.toUtf8(), basename.toUtf8().size(), 100);
	
	bufncpy(file->hdr.name, filename.toUtf8(), filename.toUtf8().size(), 100);
	
	file->hdr.mode = 0644; // file permissions. set read/write for user, read only for everyone else.
	file->hdr.size = data.size();
	file->hdr.mtime = QDateTime::currentDateTime().toTime_t();  //FIXME
	file->hdr.chksum = 0; // later
	file->hdr.typeflag = '0'; // normal file
	bufncpy(file->hdr.linkname, NULL, 0, 100);

	// Leave user/group info out. Not required AFAICT.
	file->hdr.uid = file->hdr.gid = 0;
	bufncpy(file->hdr.uname, "", 0 ,32);
	bufncpy(file->hdr.gname, "", 0, 32);

	/* if there's need for larger file names with many nested directories, put the directory path (or part of it?) in prefix */
	bufncpy(file->hdr.prefix, NULL,  0, 155);
	QTemporaryFile *tempfile = new QTemporaryFile;
	tempfile->open();
	file->data = tempfile;

	bool wasOpen = true;
	if(!data.isOpen()) {
		data.open(QIODevice::ReadOnly);
		wasOpen = false;
	}
	data.reset(); //seek to the beginning.
	// Save the uncompressed data in a temporary file.
	while(!data.atEnd())
		file->data->write(data.read(CHUNK)); 
	file->data->flush();
	if(!wasOpen)
		data.close();
	_files << file;
	return true;
}

/*
	A convenience method to add a file from a byte array.
*/
bool CATar::addFile(const QString& filename, QByteArray data, bool replace)
{
	QBuffer buf(&data, 0);
	return addFile(filename, buf, replace);
}

/*!
	Remove a file from the archive
*/
void CATar::removeFile(const QString& filename)
{
	foreach(CATarFile *t, _files) {
		if(filename == t->hdr.name) {
			delete t;
			_files.removeAll(t);
		}
	}
}

/*!
	Returns a reader for a file in the tar.	
	If the file is not found, an empty buffer is returned.
	The function returns a smart (auto) pointer to a QIODevice.

	\param filename	The file name (including its path if needed).
*/
CAIOPtr CATar::file(const QString& filename)
{ 
	if(_files.isEmpty())
		return CAIOPtr(new QBuffer());
	foreach(CATarFile *t, _files) {
		if(filename == t->hdr.name) {
			QFile *f = new QFile(t->data->fileName());
			f->open(QIODevice::ReadWrite);
			return CAIOPtr(f);
		}
	}
	return CAIOPtr(new QBuffer());
}

/*!
	Converts the file header to ASCII octal format.
*/
void CATar::writeHeader(QIODevice& dest, int file)
{
	CATarFile *f = _files[file];
	char header[513];
	char *p = header;
	int chksum = 0;
	bufncpyi(p, f->hdr.name, 100);		// Filename
	numToOcti(p, f->hdr.mode, 8);		// File permissions
	numToOcti(p, f->hdr.uid, 8);		// User ID
	numToOcti(p, f->hdr.gid, 8);		// Group ID (*Nix/Mac(?) only)
	numToOcti(p, f->hdr.size, 12);		// File size in bytes
	numToOcti(p, f->hdr.mtime, 12);		// Modification time
	bufncpyi(p, "        ", 8);			// Checksum, the sum of bytes in the header, with the checksum block filled with spaces (ASCII: 32), computed later
	*(p++) = f->hdr.typeflag;		
	bufncpyi(p, f->hdr.linkname, 100);
	bufncpyi(p, "ustar\0", 6);			// Magic
	bufncpyi(p, "00", 2);				// Version
	bufncpyi(p, f->hdr.uname, 32);		// User name
	bufncpyi(p, f->hdr.gname, 32);		// Group name
	bufncpyi(p, NULL, 0, 16); // devminor/devmajor (what are these anyway?)
	bufncpyi(p, f->hdr.prefix, 155);	// Prefix
	bufncpyi(p, NULL, 0, 12);			// Padding to 512 bytes.
		
	for(int i=0; i<500; i++)
		chksum += header[i]; // See above.
	numToOct(header+148, chksum, 8); // Insert real checksum.
	
	dest.write(header, 512); 
	
}

/*!
	Write the tar file into the given device in one call.
	Returns the number of chars written or -1 if an error ocurred.
*/

qint64 CATar::write(QIODevice& dest)
{
	qint64 total=0, i;
	while(!eof(dest)) 
	{
		i = write(dest, CHUNK);
		if(i == -1)
			return -1;
		total += i;
	}

	return total;
}

/*!
	Writes the tar file into the given device in chunks.
	Returns the number of chars written, or -1 if an error occurred.
	\warning You must use the same QIODevice to get the next chunk.

	\param dest The destination device.
	\param chunk max size to write (call again for more). Must be >= 512.
*/
qint64 CATar::write(QIODevice& dest, qint64 chunk)
{
	//bool close = false;
	int ret, pad;
	qint64 total = 0; // Bytes written.
	//qint64 first_pos;
	
	if(chunk < 512)
		return -1;

	if(_files.isEmpty())
		return 0;
		
	if(!_pos.contains(&dest))
		return -2; // dest was not open()'d.
	
	CATarBufInfo& pos = _pos[&dest];
	//first_pos = pos.pos;
	
	if(!dest.isOpen()) {
		if(!dest.open( QIODevice::WriteOnly ))
			return -1;
		pos.close = true;
	}

	if(!dest.isWritable())
	{
		if(pos.close)
			dest.close();
		return -1;
	}

	CATarFile *f;
	while(chunk >= 512)
	{
		f = _files[pos.file];
		f->data->reset();
		if(pos.pos == 0) {
			writeHeader(dest, pos.file);
			pos.pos += 512;
			total += 512;
			chunk -= 512;
		}
		
		f->data->seek(pos.pos - 512);
		ret = dest.write(f->data->read(chunk));
		pos.pos += ret;
		total += ret;
		chunk -= ret;

		if(chunk == 0)
			break;
		pad = f->data->size()%512;
		if(pad>0) {
			ret = dest.write(QByteArray(qMin(chunk,qint64(512-pad)), (char)0)); // Fill up the 512-block with nulls.
			pos.pos += ret;
			total += ret;
			chunk -= ret;
		}
		
		//proceed to next file.
		if(pos.file == _files.size()-1)
		{
			pos.eof = true;
			break;
		}
		else {
			pos.pos = 0; 
			++pos.file;
		}
	}

	if(pos.close && pos.eof) // close on last chunk, if needed.
		dest.close();
	return total;
}

/*
	Tells whether the whole tar has been written to \a dest.
*/
bool CATar::eof(QIODevice& dest)
{
	//qint64 bufsize = _files.last()->data->size();
	if(!_pos.contains(&dest))
		return false;
	CATarBufInfo& pos = _pos[&dest];

	if(_files.isEmpty())
		return true;
	
	return pos.eof;
}

/*!
	Write the first \a len bytes in \a src to \a dest and fill \a dest with ASCII NULs up to \a bufsize.

	Similar strncpy but without enforcing null termination.

	If bufsize is -1 (the default), no NULs are added.
*/
char *CATar::bufncpy(char* dest, const char *src, size_t len, int bufsize)
{
	if(bufsize == -1)
		bufsize = len;
	if((int)len<0) return dest;	
	while(bufsize-- > (int)len)
		dest[bufsize] = 0;
	while(len--)
		dest[len] = src[len];
	return dest;
}

char *CATar::bufncpyi(char*& dest, const char *src, size_t len, int bufsize)
{
	bufncpy(dest, src, len, bufsize);
	return (dest += ((bufsize == -1) ? len : bufsize));
}

/*!
	Write \a num to \a buf in octal, with length \a width.
	Similar to snprintf but wihtout enforcing null termination. Null is inserted if it fits.
*/

char *CATar::numToOct(char* buf, long long num, int width)
{
	if(num >= pow(8, width)) return 0; // Crash somewhere else.
	if(num < pow(8,(width-1))) // null fits
	{
		char len[10];
		sprintf(len, "%%0%do", width-1);
		snprintf(buf, width, len, num); // adds null
	} else {
		while(num)
		{
			int digit = num%8;
			buf[--width] = '0'+digit;
			num = num/8;
		}
	}
	return buf;
}

char *CATar::numToOcti(char*& buf, long long num, int width)
{
	numToOct(buf, num, width);
	return (buf += width);
}	

