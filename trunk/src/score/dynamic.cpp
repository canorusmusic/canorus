/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/dynamic.h"
#include "score/note.h"

/*!
	\class CADynamic
	\brief Absolute dynamic marks

	Absolute dynamic marks eg. piano, pp, sfz, forte.
*/

CADynamic::CADynamic( QString text, int volume, CANote *note )
 : CAMark( CAMark::Dynamic, note ) {
	setText(text);
	setVolume(volume);
}

CADynamic::~CADynamic() {
}

CADynamic* CADynamic::clone(CAMusElement *elt) {
	return new CADynamic( text(), volume(), (elt->musElementType()==CAMusElement::Note)?static_cast<CANote*>(elt):0 );
}

int CADynamic::compare( CAMusElement *elt ) {
	if (elt->musElementType()!=CAMusElement::Mark)
		return -2;

	if (static_cast<CAMark*>(elt)->markType()!=CAMark::Dynamic)
		return -1;

	if (static_cast<CADynamic*>(elt)->text()!=text())
		return 1;

	return 0;
}

const QString CADynamic::dynamicTextToString( CADynamicText t ) {
	switch (t) {
		case ppppp: return "ppppp";
		case pppp: return "pppp";
		case ppp: return "ppp";
		case pp: return "pp";
		case p: return "p";
		case fffff: return "fffff";
		case ffff: return "ffff";
		case fff: return "fff";
		case ff: return "ff";
		case f: return "f";
		case fp: return "fp";
		case mf: return "mf";
		case mp: return "mp";
		case rfz: return "rfz";
		case sff: return "sff";
		case sf: return "sf";
		case sfz: return "sfz";
		case spp: return "spp";
		case sp: return "sp";
		case Custom: return "";
	}
}

CADynamic::CADynamicText CADynamic::dynamicTextFromString( const QString t ) {
	if (t=="ppppp") return ppppp;
	if (t=="pppp") return pppp;
	if (t=="ppp") return ppp;
	if (t=="pp") return pp;
	if (t=="p") return p;
	if (t=="fffff") return fffff;
	if (t=="ffff") return ffff;
	if (t=="fff") return fff;
	if (t=="ff") return ff;
	if (t=="f") return f;
	if (t=="fp") return fp;
	if (t=="mf") return mf;
	if (t=="mp") return mp;
	if (t=="rfz") return rfz;
	if (t=="sff") return sff;
	if (t=="sf") return sf;
	if (t=="sfz") return sfz;
	if (t=="spp") return spp;
	if (t=="sp") return sp;
	return Custom;
}
