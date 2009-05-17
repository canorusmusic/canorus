/*!
	Copyright (c) 2008, Reinahrd Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef PDFEXPORT_H_
#define PDFEXPORT_H_

// Includes
#include "export/export.h"

// Forward declarations
class CATypesetCtl;

// PDF Export class doing lilypond export internally
// !! exportDocument does not support threading !!
class CAPDFExport : public CAExport {
#ifndef SWIG
	Q_OBJECT
#endif

public:
	CAPDFExport( QTextStream *stream=0 );
	~CAPDFExport();

	QString getTempFilePath();
#ifndef SWIG
signals:
	void pdfIsFinished( int iExitCode );

protected slots:
	void outputTypsetterOutput( const QByteArray &roOutput );
	void pdfFinished( int iExitCode );

private:
	void startExport();
	void finishExport();
	void exportDocumentImpl(CADocument *doc);
	void exportSheetImpl(CASheet *poSheet);
	void runTypesetter();

protected:
	CATypesetCtl *_poTypesetCtl;
#endif
};

#endif // PDFEXPORT_H_

