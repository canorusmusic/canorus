/*!
	Copyright (c) 2007, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CANEXPORT_H_
#define CANEXPORT_H_

#include "export/export.h"

class CAArchive;

class CACanExport : public CAExport {
public:
	CACanExport( QTextStream *stream=0 );
	~CACanExport();
	
	inline CAArchive *archive() { return _archive; }
	inline void setArchive( CAArchive *a ) { _archive = a; }
	
protected:
	void exportDocumentImpl( CADocument* doc );
	
private:
	CAArchive *_archive;
};

#endif /* CANEXPORT_H_ */
