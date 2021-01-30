/*!
	Copyright (c) 2006-2019, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include "control/externprogram.h"
#include <QDebug>

/*!	\class CAExternProgram
	\brief Start a program as extern background process

	This class is used to start an extern program as background process.
	Parameters are the program name and it's parameters
	The output of the program can be fetched via signal/slots
	Another signal is sent when the program finished including it's state

	Constructor: If the stderr output should not be shown, \a bRcvStdErr has
	             has to be set to false.
*/

CAExternProgram::CAExternProgram(bool bRcvStdErr /* = true */, bool bRcvStdOut /* = true */)
  : _poExternProgram(std::make_unique<QProcess>())
{
    _bRcvStdErr = bRcvStdErr;
    _oParamDelimiter = " ";
    connect(_poExternProgram.get(), SIGNAL(error(QProcess::ProcessError)), this, SLOT(programError(QProcess::ProcessError)));
    connect(_poExternProgram.get(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(programFinished(int, QProcess::ExitStatus)));
    if (bRcvStdOut)
        connect(_poExternProgram.get(), SIGNAL(readyReadStandardOutput()), this, SLOT(rcvProgramStdOut()));
    if (bRcvStdErr)
        connect(_poExternProgram.get(), SIGNAL(readyReadStandardError()), this, SLOT(rcvProgramStdErr()));
}

/*!
	Defines the program executable name to be run

	This method let's you define the \a oProgramm
	executable name. If the program cannot be found
	in the PATH then you either need to add it to the
	program name or define it separately via setProgramPath

	\sa setParameters( QString oParams )
	\sa setProgramPath( QString oPath )
	\sa execProgram( QString oCwd )
*/
void CAExternProgram::setProgramName(const QString& roProgram)
{
    // Make sure that the program name is defined
    if (!roProgram.isEmpty())
        _oProgramName = roProgram;
    else
        qWarning("ExternProgram: Ignoring program name being empty!");
}

/*!
	Defines the program path of the program executable

	This method let's you define the \a oPath name where
	the executable name can be found. You only need to set it
	if the program executable name cannot be found in the PATH.

	\sa setProgramName( QString oProgram )
*/
void CAExternProgram::setProgramPath(const QString& roPath)
{
    // Make sure that the program path is defined
    if (!roPath.isEmpty())
        _oProgramPath = roPath;
    else
        qWarning("ExternProgram: Ignoring program path being empty!");
}

/*!
	Defines the parameters of the program to be run

	This method let's you define all the \a oParams being added
	to the executable name at once. It removed all existant parameters.
  If you need a finer parameter control you can use this method to just
  define the first parameter if you need to change them.

	\sa addParameter( QString oParam, bool bAddSpaces = true )
	\sa setProgramName( QString oProgram )
*/
void CAExternProgram::setParameters(const QStringList& roParams)
{
    // Make sure that the parameters are defined
    if (!roParams.isEmpty())
        _oParameters = roParams;
    else
        qWarning("ExternProgram: Ignoring parameters being empty!");
}

/*!
	Returns the exit status of the finished program

	Returns "-1" if the program is still running, else
	the exist status of the process

	\sa setProgramName( QString oProgram )
	\sa QProcess::exitStatus()
*/
int CAExternProgram::getExitState()
{
    int iExitStatus = -1;
    if (getRunning())
        qWarning("ExternProgram: Getting exit state while program is still running!");
    else
        iExitStatus = static_cast<int>(_poExternProgram->exitStatus());
    return iExitStatus;
}

/*!
	Adds a new parameter to the list of parameters

	This method allows a fine grain control of the parameters
	being added to the program. Beside the \a oParam the delimiter
	is automatically add inserted if you don't set \a bAddDelimiter
	to false (the delimiter defaults to a single space character)

	\sa setParameters( QString oParams )
	\sa setParamDelimiter( QString oDelimiter )
*/
void CAExternProgram::addParameter(const QString& roParam, bool bAddDelimiter /* = true */)
{
    // Make sure that the parameters are defined
    if (!roParam.isEmpty()) {
        if (bAddDelimiter)
            _oParameters += QString(_oParamDelimiter + roParam);
        else
            _oParameters += roParam;
    } else
        qWarning("ExternProgram: Ignoring additional parameter being empty!");
}

/*!
	Runs the program in the specified \a oCwd (working directory)

	This method tries to start the specified program using the
	specified parameters. If a working directory is specified the
	it is applied to the process before the program starts.
	Errors on start are immediately reported to the error console.

	\sa setParameters( QString oParams )
	\sa setProgram( QString oProgram )
*/
bool CAExternProgram::execProgram(const QString& roCwd /* = "." */)
{
    if (_oProgramName.isEmpty()) {
        qCritical("ExternProgram: Could not run program, no program name specified!");
        return false;
    }
    if (!roCwd.isEmpty())
        _poExternProgram->setWorkingDirectory(roCwd);

    // Add optional path (including dash, so there doesn't need to be a dash at the end of the path)
    if (_oProgramPath.isEmpty()) {
        _poExternProgram->start(_oProgramName, _oParameters);
        qDebug("Started %s with parameters %s", _oProgramName.toLatin1().data(),
            _oParameters.join(" ").toLatin1().data());
    } else
        _poExternProgram->start(_oProgramPath + "/" + _oProgramName, _oParameters);
    // Wa it until program was started
    if (!_poExternProgram->waitForStarted()) {
        qCritical("ExternProgram: Could not run program %s! Error %s", _oProgramName.toLatin1().constData(),
            QString("%1 " + _poExternProgram->errorString()).arg(_poExternProgram->error()).toLatin1().constData());
        return false;
    }
    return true;
}

/*!
	This method sends a signal when data is received by the extern program

	To receive data from the extern program create a signal/slot connection
	to the nextOutput signal. This class does not cache any data!
	The \o roData contains the data received by the process output to
	either stdout or stderr. Use the constructor to define if you don't wish to
	either receive standartd output or standard error.

	\sa QProcess::setProgram( QString oProgram )
*/
void CAExternProgram::rcvProgramOutput(const QByteArray& roData)
{
    // The data is not stored but has to be immediately read by the using object
    // Else the data ist lost.
    // If you need a data caching please create a sub class from this class like
    // CACacheExternProgram or include it in a CACacheProgramData class or use
    // the cache class Qt provides named QCache for such a purpose.
    emit nextOutput(roData);
}

/*!
	This slot sends a signal when the extern program is finished or reports an error

	To get acknowledged by the program end and when an error occurs connect to the
	signal programExited. Only errors leading to a program crash are reported this
	way others are only put out to the console

	\sa QProcess::setProgram( QString oProgram )
*/
void CAExternProgram::programExited()
{
    if (getRunning()) {
        qCritical("%s",
            QString("ExternProgram: program %1 reported error %2!" + _poExternProgram->errorString()).arg(_poExternProgram->error()).toLatin1().constData());
        return;
    }
    // Check if the program exited normally else put out error message
    if (_poExternProgram->exitStatus() != QProcess::NormalExit) {
        qCritical() << "ExternProgram: program" << _oProgramName << " didn't finish correctly! Exit code"
                    << _poExternProgram->exitCode() << " " << _poExternProgram->errorString();
        emit programExited(_poExternProgram->exitCode());
    }

    if (_poExternProgram->error() == QProcess::FailedToStart) {
        qCritical() << "ExternProgram: program" << _oProgramName << "didn't start correctly! Error code"
                    << _poExternProgram->error() << " " << _poExternProgram->errorString();
        emit programExited(-1);
    }

    emit programExited(_poExternProgram->exitCode());
}
