/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%{
#include "core/file.h"

#include "import/import.h"
#include "import/canorusmlimport.h"
#include "import/lilypondimport.h"

#include "export/export.h"
#include "export/canorusmlexport.h"
#include "export/lilypondexport.h"
%}

%rename(File) CAFile;
%include "core/file.h"

%rename(Import) CAImport;
%include "import/import.h"
%rename(CanorusMLImport) CACanorusMLImport;
%include "import/canorusmlimport.h"
%rename(LilyPondImport) CALilyPondImport;
%include "import/lilypondimport.h"

%rename(Export) CAExport;
%include "export/export.h"
%rename(CanorusMLExport) CACanorusMLExport;
%include "export/canorusmlexport.h"
%rename(LilyPondExport) CALilyPondExport;
%include "export/lilypondexport.h"
