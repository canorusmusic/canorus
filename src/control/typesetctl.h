/*!
        Copyright (c) 2006-2008, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef _TYPESET_CTL_H_
#define _TYPESET_CTL_H_

// Includes
#include <QObject>
#include <QVariant>
#include <QVector>
#include <QStringList>
#include <QFile>

// Forward declarations
class CAExternProgram;
class CAExport;
class CADocument;

class CATypesetCtl : public QObject
{
	Q_OBJECT

public:
	CATypesetCtl();
	~CATypesetCtl();

	void setTypesetter( const QString &roProgramName, const QString &roProgramPath = "" );
	void setPS2PDF( const QString &roProgrammName, const QString &roProgramPath = "",
                  const QStringList &roParams = (QStringList() << QString("") ) );
	virtual void setExpOption( const QVariant &roName, const QVariant &roValue );
	virtual void setTSetOption( const QVariant &roName, const QVariant &roValue );
	inline void setPDFConversion( bool bConversion ) { _bPDFConversion = bConversion; }
	// Attention: .pdf automatically added and removed if it was added internally
	bool createPDF();

	inline bool getPDFConversion() { return _bPDFConversion; }

signals:
  void nextOutput( QByteArray oData );
	void nextStep();

protected slots:
	void rcvTypesetterOutput();
	void typsetterFinished( int iExitCode );

protected:
	void exportDocument( CADocument *poDoc );
	void runTypesetter();

	CAExternProgram    *_poTypesetter;    // Transforms exported file to pdf / postscript
	CAExternProgram    *_poConvPS2PDF;    // Transforms postscripts files to pdf if needed
	CAExport           *_poExport;        // Transforms canorus document to typesetter format
	QVector<QVariant>   _oExpOptList;		  // List of options for export
	QVector<QVariant>   _oTSetOptList;		// List of options for typesetter
	QFile               _oOutputFile;     // Output file for pdf (also used for exported file)
	bool                _bPDFConversion;  // Do a conversion from postscript to pdf
};

#endif // _TYPESET_CTL_H_

