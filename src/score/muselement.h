/*!
	Copyright (c) 2006-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MUSELEMENT_H_
#define MUSELEMENT_H_

#include <QColor>
#include <QList>
#include <QString>

#include <memory>

class CAContext;
class CAMusElement;
class CAPlayable;
class CAMark;
class CANoteCheckerError;

class CAMusElement {
public:
    enum CAMusElementType {
        Undefined = 0,
        Note,
        Rest,
        MidiNote,
        Barline,
        Clef,
        TimeSignature,
        KeySignature,
        Slur,
        Tuplet,
        Syllable,
        FunctionMark,
        FiguredBassMark,
        Mark,
        ChordName
    };

    CAMusElement(CAContext* context, int timeStart, int timeLength = 0);
    virtual ~CAMusElement();
    CAMusElement(const CAMusElement&) = delete;
    CAMusElement& operator=(const CAMusElement&) = delete;
    std::shared_ptr<CAMusElement> cloneMusElement(CAContext *context = nullptr) {
        return cloneRealElement(context);
    }
    virtual int compare(CAMusElement* elt) = 0;

    CAMusElementType musElementType() { return _musElementType; }

    inline CAContext* context() { return _context; }
    inline void setContext(CAContext* context) { _context = context; }

    inline virtual int timeStart() const { return _timeStart; }
    inline void setTimeStart(int time) { _timeStart = time; }
    inline virtual int timeLength() const { return _timeLength; }
    inline void setTimeLength(int length) { _timeLength = length; }
    inline int timeEnd() { return timeStart() + timeLength(); }

    inline virtual int realTimeStart() { return _timeStart; } // TODO: calculates and returns time in miliseconds
    inline virtual int realTimeLength() { return _timeLength; } // TODO: calculates and returns time in miliseconds
    inline int realTimeEnd() { return realTimeStart() + realTimeLength(); } // TODO: calculates and returns time in miliseconds

    inline const QString name() { return _name; }
    inline void setName(const QString name) { _name = name; }

    inline bool isVisible() { return _visible; }
    inline void setVisible(const bool v) { _visible = v; }

    inline const QColor color() { return _color; }
    inline void setColor(const QColor c) { _color = c; }

    inline const QList<CAMark*> markList() { return _markList; }
    void addMark(CAMark* mark);
    void addMarks(QList<CAMark*> marks);
    inline void removeMark(CAMark* mark) { _markList.removeAll(mark); }

    inline const QList<CANoteCheckerError*>& noteCheckerErrorList() { return _noteCheckerErrorList; }
    inline void addNoteCheckerError(CANoteCheckerError* nce) { _noteCheckerErrorList << nce; }
    inline void removeNoteCheckerError(CANoteCheckerError* nce) { _noteCheckerErrorList.removeAll(nce); }

    bool isPlayable();

    static const QString musElementTypeToString(CAMusElementType);
    static CAMusElementType musElementTypeFromString(const QString);

protected:
    virtual std::shared_ptr<CAMusElement> cloneRealElement(CAContext* context = nullptr) = 0;
    inline void setMusElementType(CAMusElementType type) { _musElementType = type; }
    void clearMarkList();

    CAMusElementType _musElementType;
    QList<CAMark*> _markList;
    QList<CANoteCheckerError*> _noteCheckerErrorList;
    CAContext* _context;
    int _timeStart;
    int _timeLength;
    bool _visible;
    QColor _color;
    QString _name;
};
#endif /* MUSELEMENT_H_ */
