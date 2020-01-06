/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MIDIIMPORT_H_
#define MIDIIMPORT_H_

#include <QStack>
#include <QString>
#include <QTextStream>

//#include "core/muselementfactory.h"

#include "score/barline.h"
#include "score/clef.h"
#include "score/diatonicpitch.h"
#include "score/keysignature.h"
#include "score/lyricscontext.h"
#include "score/playablelength.h"
#include "score/rest.h"
#include "score/syllable.h"
#include "score/timesignature.h"
#include "score/voice.h"

#include "import/import.h"

class QTextStream;
class CAMidiDevice;
class CAMidiImportEvent;
class CAMidiNote;

class CAMidiImport : public CAImport {
#ifndef SWIG
    Q_OBJECT
#endif
public:
    // Constructor
    CAMidiImport(CADocument* document = nullptr, QTextStream* in = nullptr);

    // Destructor
    virtual ~CAMidiImport();

    // close midi in file after import
    void closeFile();

    // where the real work is done
    CADocument* importDocumentImpl();
    CASheet* importSheetImpl();
    QList<QList<CAMidiNote*>> importMidiNotes();

    const QString readableStatus();
    QList<int> midiProgramList() { return _midiProgramList; }

private:
    // Alternatives during developement
    CASheet* importSheetImplPmidiParser(CASheet* sheet);
    void importMidiEvents();

    void initMidiImport();

    static const QRegExp WHITESPACE_DELIMITERS;
    static const QRegExp SYNTAX_DELIMITERS;
    static const QRegExp DELIMITERS;

    // Internal time signature
    struct CATime {
        int beats;
        int beat;
    };

private:
    enum CALilyPondDepth {
        Score,
        Layout,
        Voice,
        Chord
    };

    inline CAVoice* curVoice() { return _curVoice; }
    inline void setCurVoice(CAVoice* voice) { _curVoice = voice; }

    void addError(QString description, int lineError = 0, int charError = 0);

    // the next four objects should be moved to CADiatonicPitch, doubles are in CAKeybdInput
    CADiatonicPitch _actualKeySignature;
    signed char _actualKeySignatureAccs[7];
    int _actualKeyAccidentalsSum;
    CADiatonicPitch matchPitchToKey(CAVoice* voice, int midiPitch);

    //////////////////////
    // Helper functions //
    //////////////////////

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
    CAVoice* _curVoice;
    CASlur* _curSlur;
    CASlur* _curPhrasingSlur;
    QStack<CALilyPondDepth> _depth; // which block is currently processed
    int _curLine, _curChar;
    QList<QString> _errors;
    QList<QString> _warnings;
    QList<int> _midiProgramList; // list of first instruments in the channel or -1, if not defined

    //inline CAVoice *templateVoice() { return _templateVoice; }
    //CAVoice *_templateVoice; // used when importing voice to set the staff etc.

    //////////////////////
    // Helper functions //
    //////////////////////

    CADocument* _document;
    QVector<QList<QList<CAMidiImportEvent*>*>*> _allChannelsEvents;
    QList<CAMidiImportEvent*> _eventsX;
    void writeMidiFileEventsToScore_New(CASheet* sheet);
    void writeMidiChannelEventsToVoice_New(int channel, int voiceIndex, CAStaff* staff, CAVoice* voice);
    QVector<int> _allChannelsMediumPitch;
    QVector<CAClef*> _allChannelsClef;
    QVector<CAKeySignature*> _allChannelsKeySignatures;
    QVector<CAMidiImportEvent*> _allChannelsTimeSignatures;

    // When voices are built these functions are used to create or determine the current clef/signature
    int _actualClefIndex;
    CAMusElement* getOrCreateClef(int time, int voiceIndex, CAStaff* staff, CAVoice* voice);
    int _actualKeySignatureIndex;
    int getNextKeySignatureTime();
    CAMusElement* getOrCreateKeySignature(int time, int voiceIndex, CAStaff* staff, CAVoice* voice);
    int _actualTimeSignatureIndex;
    CAMusElement* getOrCreateTimeSignature(int time, int voiceIndex, CAStaff* staff, CAVoice* voice);
    int _numberOfAllVoices;
    void fixAccidentals(CASheet* s);
};

#endif /* MIDIIMPORT_H_ */
