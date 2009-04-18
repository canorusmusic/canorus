/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QString>
#include <QDomDocument>
#include <QTextStream>
#include <QVariant>
#include <QDir>

#include "export/canorusmlexport.h"

#include "score/document.h"
#include "score/resource.h"
#include "score/sheet.h"
#include "score/context.h"
#include "score/staff.h"
#include "score/voice.h"
#include "score/note.h"
#include "score/rest.h"
#include "score/clef.h"
#include "score/muselement.h"
#include "score/keysignature.h"
#include "score/timesignature.h"
#include "score/barline.h"

#include "score/mark.h"
#include "score/text.h"
#include "score/tempo.h"
#include "score/bookmark.h"
#include "score/articulation.h"
#include "score/crescendo.h"
#include "score/instrumentchange.h"
#include "score/dynamic.h"
#include "score/ritardando.h"
#include "score/fermata.h"
#include "score/repeatmark.h"
#include "score/fingering.h"

#include "score/lyricscontext.h"
#include "score/syllable.h"

#include "score/functionmarkcontext.h"
#include "score/functionmark.h"

CACanorusMLExport::CACanorusMLExport( QTextStream *stream )
 : CAExport(stream) {
}

CACanorusMLExport::~CACanorusMLExport() {
}

/*!
	Saves the document to CanorusML XML format.
	It uses DOM object internally for writing the XML output.
*/
void CACanorusMLExport::exportDocumentImpl( CADocument *doc ) {
	int depth = 0;

	// CADocument
	QDomDocument dDoc("canorusml");

	// Add encoding
	dDoc.appendChild(dDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" "));

	// Root node - <canorus-document>
	QDomElement dCanorusDocument = dDoc.createElement("canorus-document"); dDoc.appendChild(dCanorusDocument);
	// Add program version
	QDomElement dCanorusVersion = dDoc.createElement("canorus-version"); dCanorusDocument.appendChild(dCanorusVersion);
	dCanorusVersion.appendChild(dDoc.createTextNode(CANORUS_VERSION));

	// Document content node - <document>
	QDomElement dDocument = dDoc.createElement("document");
	dCanorusDocument.appendChild(dDocument);

	if (!doc->title().isEmpty())
		dDocument.setAttribute("title", doc->title());
	if (!doc->subtitle().isEmpty())
		dDocument.setAttribute("subtitle", doc->subtitle());
	if (!doc->composer().isEmpty())
		dDocument.setAttribute("composer", doc->composer());
	if (!doc->arranger().isEmpty())
		dDocument.setAttribute("arranger", doc->arranger());
	if (!doc->poet().isEmpty())
		dDocument.setAttribute("poet", doc->poet());
	if (!doc->textTranslator().isEmpty())
		dDocument.setAttribute("text-translator", doc->textTranslator());
	if (!doc->dedication().isEmpty())
		dDocument.setAttribute("dedication", doc->dedication());
	if (!doc->copyright().isEmpty())
		dDocument.setAttribute("copyright", doc->copyright());
	if (!doc->comments().isEmpty())
		dDocument.setAttribute("comments", doc->comments());

	dDocument.setAttribute( "date-created", doc->dateCreated().toString(Qt::ISODate) );
	dDocument.setAttribute( "date-last-modified", doc->dateLastModified().toString(Qt::ISODate) );
	dDocument.setAttribute( "time-edited", doc->timeEdited() );

	for (int sheetIdx=0; sheetIdx < doc->sheetCount(); sheetIdx++) {
		setProgress( qRound(((float)sheetIdx / doc->sheetCount()) * 100) );

		// CASheet
		QDomElement dSheet = dDoc.createElement("sheet"); dDocument.appendChild(dSheet);
		dSheet.setAttribute("name", doc->sheetAt(sheetIdx)->name());

		for (int contextIdx=0; contextIdx < doc->sheetAt(sheetIdx)->contextCount(); contextIdx++) {
			// (CAContext)
			CAContext *c = doc->sheetAt(sheetIdx)->contextAt(contextIdx);

			switch (c->contextType()) {
				case CAContext::Staff: {
					// CAStaff
					CAStaff *staff = static_cast<CAStaff*>(c);
					QDomElement dStaff = dDoc.createElement("staff"); dSheet.appendChild(dStaff);
					dStaff.setAttribute("name", staff->name());
					dStaff.setAttribute("number-of-lines", staff->numberOfLines());

					for (int voiceIdx=0; voiceIdx < staff->voiceCount(); voiceIdx++) {
						// CAVoice
						CAVoice *v = staff->voiceAt(voiceIdx);
						QDomElement dVoice = dDoc.createElement("voice"); dStaff.appendChild(dVoice);
						dVoice.setAttribute("name", v->name());
						dVoice.setAttribute("midi-channel", v->midiChannel());
						dVoice.setAttribute("midi-program", v->midiProgram());
						dVoice.setAttribute("stem-direction", CANote::stemDirectionToString(v->stemDirection()));

						exportVoiceImpl( v, dVoice ); // writes notes, clefs etc.
					}

					break;
				}
				case CAContext::LyricsContext: {
					// CALyricsContext
					CALyricsContext *lc = static_cast<CALyricsContext*>(c);
					QDomElement dlc = dDoc.createElement("lyrics-context"); dSheet.appendChild(dlc);
					dlc.setAttribute("name", lc->name());
					dlc.setAttribute("stanza-number", lc->stanzaNumber());
					dlc.setAttribute("associated-voice-idx", doc->sheetAt(sheetIdx)->voiceList().indexOf(lc->associatedVoice()));

					QList<CASyllable*> syllables = lc->syllableList();
					for (int i=0; i<syllables.size(); i++) {
						QDomElement s = dDoc.createElement("syllable"); dlc.appendChild(s);
						s.setAttribute( "time-start", syllables[i]->timeStart() );
						s.setAttribute( "time-length", syllables[i]->timeLength() );
						s.setAttribute( "text", syllables[i]->text() );
						s.setAttribute( "hyphen", syllables[i]->hyphenStart() );
						s.setAttribute( "melisma", syllables[i]->melismaStart() );

						if ( syllables[i]->associatedVoice() && doc->sheetAt(sheetIdx)->voiceList().contains(syllables[i]->associatedVoice()) ) {
							s.setAttribute( "associated-voice-idx", doc->sheetAt(sheetIdx)->voiceList().indexOf(syllables[i]->associatedVoice()) );
						}
					}

					break;
				}
				case CAContext::FunctionMarkContext: {
					// CAFunctionMarkContext
					CAFunctionMarkContext *fmc = static_cast<CAFunctionMarkContext*>(c);
					QDomElement dFmc = dDoc.createElement("function-mark-context"); dSheet.appendChild(dFmc);
					dFmc.setAttribute("name", fmc->name());

					QList<CAFunctionMark*> elts = fmc->functionMarkList();
					for (int i=0; i<elts.size(); i++) {
						QDomElement dFm = dDoc.createElement("function-mark"); dFmc.appendChild(dFm);
						dFm.setAttribute( "time-start", elts[i]->timeStart() );
						dFm.setAttribute( "time-length", elts[i]->timeLength() );
						dFm.setAttribute( "function", CAFunctionMark::functionTypeToString(elts[i]->function()) );
						dFm.setAttribute( "minor", elts[i]->isMinor() );
						dFm.setAttribute( "chord-area", CAFunctionMark::functionTypeToString(elts[i]->chordArea()) );
						dFm.setAttribute( "chord-area-minor", elts[i]->isChordAreaMinor() );
						dFm.setAttribute( "tonic-degree", CAFunctionMark::functionTypeToString(elts[i]->tonicDegree()) );
						dFm.setAttribute( "tonic-degree-minor", elts[i]->isTonicDegreeMinor() );
						exportDiatonicKey( elts[i]->key(), dFm );
						//dFm.setAttribute( "altered-degrees", elts[i]->alteredDegrees() );
						//dFm.setAttribute( "added-degrees", elts[i]->addedDegrees() );
						dFm.setAttribute( "ellipse", elts[i]->isPartOfEllipse() );
					}
				}
			}
		}
	}

	exportResources(doc, dCanorusDocument);

	out() << dDoc.toString();
}

/*!
	Used for writing the voice node in XML output.
	It uses DOM object internally for writing the XML output.
	This method is usually called by saveDocument().

	\sa exportDocumentImpl()
*/
void CACanorusMLExport::exportVoiceImpl( CAVoice* voice, QDomElement& dVoice ) {
	QDomDocument dDoc = dVoice.ownerDocument();
	for (int i=0; i<voice->musElementCount(); i++) {
		CAMusElement *curElt = voice->musElementAt(i);
		QDomElement dElt;
		switch (curElt->musElementType()) {
			case CAMusElement::Note: {
				CANote *note = static_cast<CANote*>(curElt);

				if ( note->isFirstInTuplet() ) {
					_dTuplet = dDoc.createElement("tuplet");
					_dTuplet.setAttribute( "number", note->tuplet()->number() );
					_dTuplet.setAttribute( "actual-number", note->tuplet()->actualNumber() );
					dVoice.appendChild( _dTuplet );
				}

				dElt = dDoc.createElement("note");
				if ( note->tuplet() ) {
					_dTuplet.appendChild( dElt );
				} else {
					dVoice.appendChild(dElt);
				}

				if (note->stemDirection()!=CANote::StemPreferred)
					dElt.setAttribute("stem-direction", CANote::stemDirectionToString(note->stemDirection()));

				exportPlayableLength( note->playableLength(), dElt );
				exportDiatonicPitch( note->diatonicPitch(), dElt );

				if ( note->tieStart() ) {
					QDomElement dTie = dDoc.createElement("tie"); dElt.appendChild( dTie );
					dTie.setAttribute("slur-style", CASlur::slurStyleToString( note->tieStart()->slurStyle() ));
					dTie.setAttribute("slur-direction", CASlur::slurDirectionToString( note->tieStart()->slurDirection() ));
				}
				if ( note->slurStart() ) {
					QDomElement dSlur = dDoc.createElement("slur-start"); dElt.appendChild( dSlur );
					dSlur.setAttribute("slur-style", CASlur::slurStyleToString( note->slurStart()->slurStyle() ));
					dSlur.setAttribute("slur-direction", CASlur::slurDirectionToString( note->slurStart()->slurDirection() ));
				}
				if ( note->slurEnd() ) {
					QDomElement dSlur = dDoc.createElement("slur-end"); dElt.appendChild( dSlur );
				}
				if ( note->phrasingSlurStart() ) {
					QDomElement dPhrasingSlur = dDoc.createElement("phrasing-slur-start"); dElt.appendChild( dPhrasingSlur );
					dPhrasingSlur.setAttribute("slur-style", CASlur::slurStyleToString( note->phrasingSlurStart()->slurStyle() ));
					dPhrasingSlur.setAttribute("slur-direction", CASlur::slurDirectionToString( note->phrasingSlurStart()->slurDirection() ));
				}
				if ( note->phrasingSlurEnd() ) {
					QDomElement dPhrasingSlur = dDoc.createElement("phrasing-slur-end"); dElt.appendChild( dPhrasingSlur );
				}

				break;
			}
			case CAMusElement::Rest: {
				CARest *rest = static_cast<CARest*>(curElt);

				if ( rest->isFirstInTuplet() ) {
					_dTuplet = dDoc.createElement("tuplet");
					_dTuplet.setAttribute( "number", rest->tuplet()->number() );
					_dTuplet.setAttribute( "actual-number", rest->tuplet()->actualNumber() );
					dVoice.appendChild( _dTuplet );
				}

				dElt = dDoc.createElement("rest");
				if ( rest->tuplet() ) {
					_dTuplet.appendChild( dElt );
				} else {
					dVoice.appendChild(dElt);
				}

				dElt.setAttribute("rest-type", CARest::restTypeToString(rest->restType()));

				exportPlayableLength( rest->playableLength(), dElt );

				break;
			}
			case CAMusElement::Clef: {
				CAClef *clef = static_cast<CAClef*>(curElt);
				dElt = dDoc.createElement("clef"); dVoice.appendChild(dElt);
				dElt.setAttribute("clef-type", CAClef::clefTypeToString(clef->clefType()));
				dElt.setAttribute("c1", clef->c1());
				dElt.setAttribute("offset", clef->offset());

				break;
			}
			case CAMusElement::KeySignature: {
				CAKeySignature *key = static_cast<CAKeySignature*>(curElt);
				dElt = dDoc.createElement("key-signature"); dVoice.appendChild(dElt);
				dElt.setAttribute("key-signature-type", CAKeySignature::keySignatureTypeToString(key->keySignatureType()));

				if ( key->keySignatureType()==CAKeySignature::MajorMinor ) {
					exportDiatonicKey( key->diatonicKey(), dElt );
				} else
				if (key->keySignatureType()==CAKeySignature::Modus) {
					dElt.setAttribute("modus", CAKeySignature::modusToString(key->modus()));
				}
				//! \todo Custom accidentals in key signature saving -Matevz
				// exportDiatonicPitch( key->diatonicKey().diatonicPitch(), dKey );

				break;
			}
			case CAMusElement::TimeSignature: {
				CATimeSignature *time = static_cast<CATimeSignature*>(curElt);
				dElt = dDoc.createElement("time-signature"); dVoice.appendChild(dElt);
				dElt.setAttribute("time-signature-type", CATimeSignature::timeSignatureTypeToString(time->timeSignatureType()));
				dElt.setAttribute("beats", time->beats());
				dElt.setAttribute("beat", time->beat());

				break;
			}
			case CAMusElement::Barline: {
				CABarline *barline = static_cast<CABarline*>(curElt);
				dElt = dDoc.createElement("barline"); dVoice.appendChild(dElt);
				dElt.setAttribute("barline-type", CABarline::barlineTypeToString(barline->barlineType()));

				break;
			}
		}

		exportTime( curElt, dElt );
		exportColor( curElt, dElt );

		exportMarks( curElt, dElt );
	}
}

void CACanorusMLExport::exportMarks( CAMusElement *elt, QDomElement& domElt ) {
	for (int i=0; i<elt->markList().size(); i++) {
		CAMark *mark = elt->markList()[i];
		if ( !mark->isCommon() || elt->musElementType()!=CAMusElement::Note ||
		     elt->musElementType()==CAMusElement::Note && static_cast<CANote*>(elt)->isFirstInChord() ) {
			QDomElement dMark = domElt.ownerDocument().createElement("mark"); domElt.appendChild(dMark);
			dMark.setAttribute("time-start", mark->timeStart());
			dMark.setAttribute("time-length", mark->timeLength());
			dMark.setAttribute("mark-type", CAMark::markTypeToString(mark->markType()));

			switch (mark->markType()) {
			case CAMark::Text: {
				CAText *text = static_cast<CAText*>(mark);
				dMark.setAttribute("text", text->text());
				break;
			}
			case CAMark::Tempo: {
				CATempo *tempo = static_cast<CATempo*>(mark);
				dMark.setAttribute("bpm", tempo->bpm());
				exportPlayableLength( tempo->beat(), dMark );
				break;
			}
			case CAMark::Ritardando: {
				CARitardando *rit = static_cast<CARitardando*>(mark);
				dMark.setAttribute("ritardando-type", CARitardando::ritardandoTypeToString(rit->ritardandoType()));
				dMark.setAttribute("final-tempo", rit->finalTempo());
				break;
			}
			case CAMark::Dynamic: {
				CADynamic *dyn = static_cast<CADynamic*>(mark);
				dMark.setAttribute("volume", dyn->volume());
				dMark.setAttribute("text", dyn->text());
				break;
			}
			case CAMark::Crescendo: {
				CACrescendo *cresc = static_cast<CACrescendo*>(mark);
				dMark.setAttribute("final-volume", cresc->finalVolume());
				dMark.setAttribute("crescendo-type", CACrescendo::crescendoTypeToString(cresc->crescendoType()));
				break;
			}
			case CAMark::Pedal: {
				// none
				break;
			}
			case CAMark::InstrumentChange: {
				CAInstrumentChange *ic = static_cast<CAInstrumentChange*>(mark);
				dMark.setAttribute("instrument", ic->instrument());
				break;
			}
			case CAMark::BookMark: {
				CABookMark *b = static_cast<CABookMark*>(mark);
				dMark.setAttribute("text", b->text());
				break;
			}
			case CAMark::RehersalMark: {
				// none
				break;
			}
			case CAMark::Fermata: {
				CAFermata *f = static_cast<CAFermata*>(mark);
				dMark.setAttribute("fermata-type", CAFermata::fermataTypeToString(f->fermataType()));
				break;
			}
			case CAMark::RepeatMark: {
				CARepeatMark *r = static_cast<CARepeatMark*>(mark);
				dMark.setAttribute("repeat-mark-type", CARepeatMark::repeatMarkTypeToString(r->repeatMarkType()));
				if (r->repeatMarkType()==CARepeatMark::Volta) {
					dMark.setAttribute("volta-number", r->voltaNumber());
				}
				break;
			}
			case CAMark::Articulation: {
				CAArticulation *a = static_cast<CAArticulation*>(mark);
				dMark.setAttribute("articulation-type", CAArticulation::articulationTypeToString(a->articulationType()));
				break;
			}
			case CAMark::Fingering: {
				CAFingering *f = static_cast<CAFingering*>(mark);
				dMark.setAttribute("original", f->isOriginal());
				for (int i=0; i<f->fingerList().size(); i++)
					dMark.setAttribute(QString("finger%1").arg(i), CAFingering::fingerNumberToString(f->fingerList()[i]));
				break;
			}
			}

			exportColor( mark, dMark );
		}

	}
}

void CACanorusMLExport::exportColor( CAMusElement *elt, QDomElement& domParent ) {
	if ( elt->color()!=QColor() ) {
		domParent.setAttribute( "color", QVariant(elt->color()).toString() );
	}
}

void CACanorusMLExport::exportTime( CAMusElement *elt, QDomElement& domParent ) {
	domParent.setAttribute("time-start", elt->timeStart());

	if ( elt->isPlayable() ) {
		domParent.setAttribute("time-length", elt->timeLength());
	}
}

void CACanorusMLExport::exportPlayableLength( CAPlayableLength l, QDomElement& domParent ) {
	QDomElement dl = domParent.ownerDocument().createElement("playable-length"); domParent.appendChild(dl);
	dl.setAttribute( "music-length", CAPlayableLength::musicLengthToString(l.musicLength()) );
	dl.setAttribute( "dotted", l.dotted() );
}

void CACanorusMLExport::exportDiatonicPitch( CADiatonicPitch p, QDomElement& domParent ) {
	QDomElement dp = domParent.ownerDocument().createElement("diatonic-pitch"); domParent.appendChild(dp);
	dp.setAttribute( "note-name", p.noteName() );
	dp.setAttribute( "accs", p.accs() );
}

void CACanorusMLExport::exportDiatonicKey( CADiatonicKey k, QDomElement& domParent ) {
	QDomElement dk = domParent.ownerDocument().createElement("diatonic-key"); domParent.appendChild(dk);
	dk.setAttribute("gender", CADiatonicKey::genderToString(k.gender()));
	exportDiatonicPitch( k.diatonicPitch(), dk );
}

/*!
	Exports the resources to exported filename files/ directory.

	There are 4 possible scenarios:
	1) Linked resource, resource is remote (eg. http, https resource on the web):
	   Only resource url is stored inside the xml file.
	2) Linked resource, resource is local (eg. large video on the disk):
	   Relative path to the resource is calculated from the directory where the
	   document is being saved.
	3) Attached resource:
	   Resource is copied from the tmp/ directory to the directory where the document
	   is being saved + "filename files/". eg. "content.xml files/myImageXXXX.png"
 */
void CACanorusMLExport::exportResources( CADocument *doc, QDomElement& dCanorusDocument ) {
	for (int i=0; i<doc->resourceList().size(); i++) {
		CAResource *r = doc->resourceList()[i];
		QUrl url;

		if (r->isLinked()) {
			// linked resource, calculate relative path of the resource to the document where it's being saved
			if (r->url().scheme()=="file" && file()) {
				// local file
				QDir outDir(QFileInfo(*file()).absolutePath());
				url = QUrl::fromLocalFile( outDir.relativeFilePath( r->url().toLocalFile() ) );
			} else {
				// remote file
				url = r->url();
			}
		} else if (file()) {
			// attached resource, copy the resource to "filename files/" directory
			QString targetDir = QFileInfo(*file()).absolutePath();
			QString targetFileName = QFileInfo(*file()).fileName();

			// create directory if it doesn't exist
			if (!QDir(targetDir+"/"+targetFileName+" files").exists()) {
				QDir(targetDir).mkdir(targetFileName+" files");
			}

			// copies resource /tmp/qt_tempXXXX -> myDocument files/qt_tempXXXX
			r->copy( targetDir+"/"+targetFileName+" files/" + QFileInfo(r->url().toLocalFile()).fileName() );

			// generates relative path
			url = QString("file://") + targetFileName + " files/" + QFileInfo(r->url().toLocalFile()).fileName();
		} else {
			// saving to stream - usually when compressing to .can format
			// copying is done in CACanExport class
			url = QString("file://content.xml files/")+QFileInfo(r->url().toLocalFile()).fileName();
		}

		QDomElement dResource = dCanorusDocument.ownerDocument().createElement("resource");

		dResource.setAttribute("name", r->name());
		dResource.setAttribute("description", r->description());
		dResource.setAttribute("linked", r->isLinked());
		dResource.setAttribute("resource-type", CAResource::resourceTypeToString(r->resourceType()));
		dResource.setAttribute("url", url.toString());

		dCanorusDocument.appendChild(dResource);
	}
}
