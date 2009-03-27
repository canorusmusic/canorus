/*!
	Copyright (c) 2008, Reinahrd Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef SVGEXPORT_H_
#define SVGEXPORT_H_

// Includes
#include "export/export.h"

// Forward declarations
class CATypesetCtl;

// SVG Export class doing lilypond export internally
// !! exportDocument does not support threading !!
class CASVGExport : public CAExport {
#ifndef SWIG
	Q_OBJECT
#endif

public:
	CASVGExport( QTextStream *stream=0 );
	~CASVGExport();

	QString getTempFilePath();

#ifndef SWIG
signals:
	void svgIsFinished( int iExitCode );

protected slots:
	void outputTypsetterOutput( const QByteArray &roOutput );
	void svgFinished( int iExitCode );

private:
	void startExport();
	void finishExport();
	void exportDocumentImpl(CADocument *doc);

protected:
	CATypesetCtl *_poTypesetCtl;
#endif
};

#endif // SVGEXPORT_H_

