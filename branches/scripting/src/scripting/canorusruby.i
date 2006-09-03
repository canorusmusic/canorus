/** @file context.i
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

%module CanorusRuby

//convert returned QString value to Ruby's String format in UTF8 encoding
%typemap(out) const QString {
	$result = rb_str_new2($1.toUtf8().data());
}

//convert Ruby's string to QString in UTF8 encoding
%typemap(in) const QString {
	$1 = QString::fromUtf8(STR2CSTR($input));
}

%include "scripting/context.i"
%include "scripting/document.i"
%include "scripting/muselement.i"
%include "scripting/sheet.i"

%{
#ifdef __cplusplus
extern "C"
#endif
SWIGEXPORT void Init_CanorusRuby(void);

VALUE toRubyDocument(void *document) {
	if (!SWIGTYPE_p_CADocument)
		Init_CanorusRuby();
	return SWIG_Ruby_NewPointerObj((void*)document, SWIGTYPE_p_CADocument, 0);
}
%}