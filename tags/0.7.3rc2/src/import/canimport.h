/*!
	Copyright (c) 2007, Matevž Jekovec, Itay Perl, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef CANIMPORT_H_
#define CANIMPORT_H_

#include "import/import.h"

class CAArchive;

class CACanImport : public CAImport {
public:
	CACanImport( QTextStream *stream=0 );
	~CACanImport();
	
	inline CAArchive *archive() { return _archive; }
	inline void setArchive( CAArchive *a ) { _archive = a; }
	
protected:
	CADocument *importDocumentImpl();
	
private:
	CAArchive *_archive;
};

#endif /* CANIMPORT_H_ */
