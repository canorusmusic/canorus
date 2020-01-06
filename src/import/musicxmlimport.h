/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MUSICXMLIMPORT_H_
#define MUSICXMLIMPORT_H_

#include <QHash>
#include <QMultiHash>
#include <QStack>
#include <QString>
#include <QXmlStreamReader>

#include "import/import.h"
#include "score/diatonicpitch.h"
#include "score/playablelength.h"

class CADocument;
class CASheet;
class CAStaff;
class CAClef;
class CAKeySignature;
class CATimeSignature;
class CATempo;

class CAMusicXmlImport : public CAImport, private QXmlStreamReader {
#ifndef SWIG
    Q_OBJECT
#endif
public:
    CAMusicXmlImport(QTextStream* stream = 0);
    CAMusicXmlImport(const QString stream);
    virtual ~CAMusicXmlImport();

    const QString readableStatus();

protected:
    CADocument* importDocumentImpl();

private:
    void initMusicXmlImport();

    void readHeader();
    void readScorePartwise();
    void readScoreTimewise();
    void readWork();
    void readIdentification();
    void readDefaults();
    void readPartList();
    void readPart();
    void readMeasure(QString partId);
    void readAttributes(QString partId);
    void readNote(QString partId, int);
    void readForward(QString partId, int);
    void readSound(QString partId);
    CAVoice* addVoiceIfNeeded(QString partId, int staff, int voice);
    void addStavesIfNeeded(QString partId, int staves);

    QString _musicXmlVersion;

    CADocument* _document;
    QHash<QString, QHash<int, CAVoice*>> _partMapVoice; // part name -> map of voice number : voice
    QHash<QString, QList<CAStaff*>> _partMapStaff; // part name -> list of staffs
    QHash<QString, QHash<int, CAClef*>> _partMapClef; // part name -> map of staff number : last clef
    QHash<QString, QHash<int, CAKeySignature*>> _partMapKeySig; // part name -> map of staff number : last keysig
    QHash<QString, QHash<int, CATimeSignature*>> _partMapTimeSig; // part name -> map of staff number : last timesig
    QHash<QString, int> _midiChannel; // 1-16
    QHash<QString, int> _midiProgram; // 1-128
    QHash<QString, QString> _partName;
    QHash<QString, int> _divisions; // part name -> divisions
    int _tempoBpm; // current tempo buffer, append to first found note, set to -1 then
};

#endif /* MUSICXMLIMPORT_H_ */
