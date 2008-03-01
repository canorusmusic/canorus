/*!
        Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _EXTERN_PROGRAM_H_
#define _EXTERN_PROGRAM_H_

// Includes
#include <QObject>
#include <QProcess>
#include <QStringList>

// Forward declarations

// This class is used to run a program in the background
// and to get it's input/output via signals

// Class definition
class CAExternProgram : public QObject
{
  Q_OBJECT

public:
	CAExternProgram( bool bRcvStdErr = true, bool bRcvStdOut = true );
	~CAExternProgram();

	void setProgramName( const QString &roProgram );
	void setProgramPath( const QString &roPath );
	// Warning: Setting all parameters overwrites all
	// parameters added via addParameter method!
	void setParameters( const QStringList &roParams );
	void inline setParamDelimiter( QString oDelimiter = " " )
	{ _oParamDelimiter = oDelimiter; }

	inline const QStringList &getParameters() { return _oParameters; }
	inline bool getRunning() 
	{ return _poExternProgram->state() == QProcess::Running; }
	inline const QString &getParamDelimiter() { return _oParamDelimiter; }
	int getExitState();

	void addParameter( const QString &roParam, bool bAddDelimiter = true );
	inline void clearParameters() { _oParameters.clear(); }
	bool execProgram( const QString &roCwd = "." );

signals:
	void nextOutput( const QByteArray &roData );
	void programExited( int iExitCode );
  
protected slots:
	void rcvProgramOutput();
	void programError( QProcess::ProcessError ) { programExited(); }
	void programFinished( int, QProcess::ExitStatus ) {  programExited(); }

protected:
	void programExited();

	// References to the real objects(!)
	QProcess    *_poExternProgram;       // Process object running the watched program
	QString      _oProgramName;          // Program name to be run
	QString      _oProgramPath;          // Program path being added to the program name
	QStringList  _oParameters;           // List of program parameters
	QString      _oParamDelimiter;       // delimiter between the single parameters
	bool         _bRcvStdErr;            // 'true': Receive program output from stderr
};

#endif // _EXTERN_PROGRAM_

