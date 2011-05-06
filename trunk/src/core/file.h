/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef FILE_H_
#define FILE_H_

#include <QThread>
#include <QFile>

class QTextStream;

class CAFile : public QThread {
public:
	CAFile();
	virtual ~CAFile();

	inline const int status() { return _status; }
	inline const int progress() { return _progress; }
	virtual const QString readableStatus() = 0;
	void setStreamFromFile( const QString filename );
	void setStreamToFile( const QString filename );
	void setStreamFromDevice( QIODevice* device );
	void setStreamToDevice( QIODevice* device );

	void setStreamToString();
	QString getStreamAsString();

protected:
	inline void setStatus( const int status ) { _status = status; }
	inline void setProgress( const int progress ) { _progress = progress; }

	inline QTextStream *stream() { return _stream; }
	virtual void setStream( QTextStream *stream ) { _stream = stream; }

	inline QFile *file() { return _file; }
	inline void setFile( QFile *file ) { _file = file; }

private:
	int _status;          // status number
	int _progress;        // percentage of the work already done
	QTextStream *_stream;
	QFile *_file;
	bool _deleteStream;	 // whether to delete stream when destroyed.
};

#endif /* FILE_H_ */
