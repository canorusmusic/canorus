/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

%{
#include "core/settings.h"

#include "core/file.h"

#include "import/import.h"
#include "import/canorusmlimport.h"
#include "import/canimport.h"
#include "import/lilypondimport.h"

#include "export/export.h"
#include "export/canorusmlexport.h"
#include "export/canexport.h"
#include "export/lilypondexport.h"
#include "export/midiexport.h"
%}

%include "core/settings.h"

%include "core/file.h"

%include "import/import.h"
%include "import/canorusmlimport.h"
%include "import/canimport.h"
%include "import/lilypondimport.h"

%include "export/export.h"
%include "export/canorusmlexport.h"
%include "export/canexport.h"
%include "export/lilypondexport.h"
%include "export/midiexport.h"
