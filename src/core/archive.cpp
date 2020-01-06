/*!
	Copyright (c) 2007-2020, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QByteArray>
#include <QTemporaryFile>
#include <QString>
#include <QRegExp>
#include <zlib.h> 

#ifdef Q_OS_WIN
#	include <QSysInfo>
#endif

#include "core/tar.h"
#include "core/archive.h"

/*!
	\class CAArchive
	\brief Class for the manipulation of a gzipped tar archive (tar.gz)
	
	This class allows read/write operations on tar.gz archives.
	\warning This is not a CATar subclass as it does not represent a tar file, but a gzipped file. The uncompressed content is a tar file. 

	See RFC 1952 for the GZIP specification.
*/

const int CAArchive::CHUNK = 16384;
const QString CAArchive::COMMENT = "Canorus Archive v"+QString(CANORUS_VERSION).remove(QRegExp("[a-z]*$"));

/*!
	Creates and empty archive
*/
CAArchive::CAArchive()
:_err(false)
{
	_tar = new CATar();
}

/*!
	Read an existing archive.
*/
CAArchive::CAArchive(QIODevice& arch)
:_err(false)
{
	parse(arch);
}

/*!
	Destory the archive
*/
CAArchive::~CAArchive()
{
	delete _tar;
}

/*!
	Parse/decompress an existing archive
*/
void CAArchive::parse(QIODevice& arch)
{
	bool close = false;
	int ret;
	z_stream strm;
	QTemporaryFile tar;
	QBuffer in, out;
	gz_header header = gz_header();

	in.buffer().resize(CHUNK);
	out.buffer().resize(CHUNK);
	tar.open();

	if(!arch.isOpen()) {
		if(!arch.open(QIODevice::ReadOnly)) {
			_err = true; // _tar is invalid.
			return;
		}
		close = true;
	}
	
	header.os = getOS();
	header.comment = new unsigned char[21];
	header.comm_max = 21;

	// decompress
	arch.reset();
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, 31);
	ret = (ret == Z_OK) ? inflateGetHeader(&strm, &header) : ret; 
	if(ret != Z_OK) //clean up, set error and return
	{
		delete[] header.comment;
		inflateEnd(&strm);
		if(close)
			arch.close();
		return;
	}

	// decompress until deflate stream ends or EOF 
	do {
		strm.avail_in = arch.read(in.buffer().data(), CHUNK);
		if(strm.avail_in == 0)
			break;
		// The code purposely could cut contents of the array.
		// For strings it would only work with ASCII code nothing else
		strm.next_in = reinterpret_cast<Bytef *>(in.buffer().data());
		do {
			strm.avail_out = CHUNK;
            // The code purposely could cut contents of the array.
            // For strings it would only work with ASCII code nothing else
			strm.next_out = reinterpret_cast<Bytef *>(out.buffer().data());
			ret = inflate(&strm, Z_NO_FLUSH);
			if((ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) || // buffer error is not fatal
				tar.write(out.buffer().data(), CHUNK - strm.avail_out) != CHUNK - strm.avail_out) {
				
				_err = true;
				break;
			}
		} while (strm.avail_out == 0);
 	} while(ret != Z_STREAM_END && !_err);
	inflateEnd(&strm);

	if(ret != Z_STREAM_END)
		_err = true;
	
	if(!_err) {
		QRegExp re("Canorus Archive v(\\d+\\.\\d+)");
        // The code purposely could cut contents of the array.
        // For strings it would only work with ASCII code nothing else
		if(re.indexIn(reinterpret_cast<char*>(header.comment)) != -1)
			_version = re.cap(1);
		else {
			_err = true;
		}
		
		tar.reset();
		_tar = new CATar(tar);
	}

	delete[] header.comment;
	if(close)
		arch.close();
}

/*!
	Write the tar.gz archive into the given device.
	Returns the number of byte written, or -1 on error.
*/

qint64 CAArchive::write(QIODevice& dest)
{
	bool close = false;
	int ret, flush;
	qint64 total = 0, read;
	z_stream strm;
	gz_header header = gz_header();
	QBuffer in, out;

	if(!dest.isOpen())
	{
		if(!dest.open(QIODevice::WriteOnly))
			return -1;
		close = true;
	}

	if(!dest.isWritable() || error())
	{
		if(close)
			dest.close();
		return -1;
	}
	
	header.os = getOS(); 
	header.comment = new unsigned char[COMMENT.size()+1];
    // The code purposely could cut contents of the array.
    // For strings it would only work with ASCII code nothing else
	strcpy(reinterpret_cast<char*>(header.comment), COMMENT.toLatin1().data());
	
	in.open(QIODevice::ReadWrite);
	out.open(QIODevice::ReadWrite);
	in.buffer().resize(CHUNK);
	out.buffer().resize(CHUNK);
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
	ret = (ret == Z_OK) ? deflateSetHeader(&strm, &header) : ret;
	if(ret != Z_OK)
	{
		deflateEnd(&strm);
		delete[] header.comment;
		if(close)
			dest.close();
		in.close();
		out.close();
		return -1;
	}

	//compress until EOF
	_tar->open(in);
	do { 
		in.reset();
		ret = _tar->write(in, CHUNK);
		if(ret == -1)
		{
			_err = true;
			break;
		}
		flush = _tar->eof(in) ? Z_FINISH : Z_NO_FLUSH;
		strm.avail_in = ret;
        // The code purposely could cut contents of the array.
        // For strings it would only work with ASCII code nothing else
		strm.next_in = reinterpret_cast<unsigned char*>(in.buffer().data());
		do {
			strm.avail_out = CHUNK;
            // The code purposely could cut contents of the array.
            // For strings it would only work with ASCII code nothing else
			strm.next_out = reinterpret_cast<unsigned char*>(out.buffer().data());
			ret = deflate(&strm, flush);
			if(ret == Z_STREAM_ERROR)
				_err = true;
			read = (!_err) ? dest.write(out.buffer().data(), CHUNK - strm.avail_out) : 0;
			if(_err || (read != CHUNK - strm.avail_out)) {
				_err = true;
				break;
			}
			total += read;
		} while(strm.avail_out == 0);
		if(strm.avail_in != 0)
			_err = true;
	} while (flush != Z_FINISH && !_err);
	
	deflateEnd(&strm);
	if(ret != Z_STREAM_END)
		_err = true;
	
	delete[] header.comment;
	if(close)
		dest.close();
	_tar->close(in);
	in.close();
	out.close();
	return (_err) ? -1 : total;
}

/*!
	Return an operating system ID for use in a GZip header. 
	See RFC 1952.
*/

int CAArchive::getOS()
{
#ifdef Q_OS_WIN
	if(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)
		return 11; // rfc 1952: "NTFS filesystem (NT)"
	else
		return 0; // rfc 1952: "FAT filesystem (MS-DOS, OS/2, NT/Win32"
#else // Mac or Linux/Unix/FreeBSD/...
	return 3;  // rfc 1952: "Unix". That what gzip does on Darwin (though there's 7 for "Macintosh").
#endif
}

