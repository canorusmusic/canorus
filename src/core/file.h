/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef FILE_H_
#define FILE_H_

#include <QThread>

class CAFile : public QThread {
public:
	CAFile();
	virtual ~CAFile();
	
	inline const int status() { return _status; }
	inline const float progress() { return _progress; }
	virtual static const QString statusToReadable( int status ) = 0;
	
protected:
	inline void setStatus( const int status ) { _status = status; }
	inline void setProgress( const float progress ) { _progress = progress; }
	inline void setStream( QTextStream *stream ) { _stream = stream; }
	
private:
	int _status;
	float _progress;
	QTextStream *_stream;
};

#endif /* FILE_H_ */
