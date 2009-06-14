/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef LILYPONDIMPORT_H_
#define LILYPONDIMPORT_H_

#include <QString>
#include <QStack>

#include "score/voice.h"
#include "score/rest.h"
#include "score/keysignature.h"
#include "score/clef.h"
#include "score/timesignature.h"
#include "score/barline.h"
#include "score/lyricscontext.h"
#include "score/syllable.h"
#include "score/playablelength.h"
#include "score/diatonicpitch.h"

#include "import/import.h"

class QTextStream;

class CALilyPondImport : public CAImport {
public:
	// Constructors
	CALilyPondImport( const QString in );
	CALilyPondImport( QTextStream *in=0 );
	CALilyPondImport( CADocument *document, QTextStream *in=0 );
	inline void setTemplateVoice( CAVoice *voice ) { _templateVoice = voice; }

	// Destructor
	virtual ~CALilyPondImport();

	const QString readableStatus();
	CASheet *importSheetImpl();

private:
	void initLilyPondImport();

	static const QRegExp WHITESPACE_DELIMITERS;
	static const QRegExp SYNTAX_DELIMITERS;
	static const QRegExp DELIMITERS;

	// Internal time signature
	struct CATime {
		int beats;
		int beat;
	};

	enum CALilyPondDepth {
		Score,
		Layout,
		Voice,
		Chord
	};

	// Actual import of the input string
	CAVoice *importVoiceImpl();
	CALyricsContext *importLyricsContextImpl();

	inline CAVoice *curVoice() { return _curVoice; }
	inline void setCurVoice(CAVoice *voice) { _curVoice = voice; }

	const QString parseNextElement();
	const QString peekNextElement();
	void addError(QString description, int lineError = 0, int charError = 0);

	//////////////////////
	// Helper functions //
	//////////////////////
	CAPlayableLength playableLengthFromLilyPond( QString &playableElt, bool parse=false );

	bool isNote(const QString elt);
	CADiatonicPitch relativePitchFromLilyPond(QString &note, CADiatonicPitch prevPitch, bool parse=false);
	bool isRest(const QString elt);
	CARest::CARestType restTypeFromLilyPond(QString& rest, bool parse=false);
	CAClef::CAPredefinedClefType predefinedClefTypeFromLilyPond( const QString clef );
	int clefOffsetFromLilyPond( const QString clef );
	CABarline::CABarlineType barlineTypeFromLilyPond(const QString bar);
	CADiatonicKey::CAGender diatonicKeyGenderFromLilyPond(QString gender);
	CATime timeSigFromLilyPond(QString time);

	CAMusElement* findSharedElement(CAMusElement *elt);

	///////////////////////////
	// Getter/Setter methods //
	///////////////////////////
	inline QString& in() { return *stream()->string(); }
	inline CALilyPondDepth curDepth() { return _depth.top(); }
	inline void pushDepth(CALilyPondDepth depth) { _depth.push(depth); }
	inline CALilyPondDepth popDepth() { return _depth.pop(); }
	inline int curLine() { return _curLine; }
	inline int curChar() { return _curChar; }

	// Attributes
	CAVoice *_curVoice;
	CASlur *_curSlur;
	CASlur *_curPhrasingSlur;
	QStack<CALilyPondDepth> _depth; // which block is currently processed
	int _curLine, _curChar;
	QList<QString> _errors;
	QList<QString> _warnings;

	inline CAVoice *templateVoice() { return _templateVoice; }
	CAVoice *_templateVoice; // used when importing voice to set the staff etc.

	CADocument *_document;
};

#endif /* LILYPONDIMPORT_H_ */
