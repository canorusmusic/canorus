/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MUSELEMENT_H_
#define MUSELEMENT_H_

#include <QString>
#include <QList>

class CAContext;
class CAMusElement;
class CAPlayable;
class CAMark;

class CAMusElement {
public:
	enum CAMusElementType {
		Undefined = 0,
		Note,
		Rest,
		Barline,
		Clef,
		TimeSignature,
		KeySignature,
		Slur,
		Tuplet,
		Syllable,
		FunctionMark,
		Mark
	};

	CAMusElement(CAContext *context, int timeStart, int timeLength=0);
	virtual ~CAMusElement();

	virtual CAMusElement* clone()=0;
	virtual int compare(CAMusElement *elt) = 0;

	CAMusElementType musElementType() { return _musElementType; }

	inline CAContext *context() { return _context; }
	inline CAContext *setContext(CAContext *context) { _context = context; }

	inline int timeStart() { return _timeStart; }
	inline void setTimeStart(int time) { _timeStart = time; }
	inline int timeLength() { return _timeLength; }
	inline void setTimeLength(int length) { _timeLength = length; }
	inline int timeEnd() { return _timeStart + _timeLength; }

	inline const QString name() { return _name; }
	inline void setName(const QString name) { _name = name; }

	inline const QList<CAMark*> markList() { return _markList; }
	void addMark( CAMark *mark );
	void addMarks( QList<CAMark*> marks );
	inline void removeMark( CAMark* mark ) { _markList.removeAll(mark); }

	bool isPlayable();

	static const QString musElementTypeToString(CAMusElementType);
	static CAMusElementType musElementTypeFromString(const QString);

protected:
	inline void setMusElementType( CAMusElementType type ) { _musElementType = type; }

	QList< CAMark* > _markList;
	CAMusElementType _musElementType;
	CAContext *_context;
	int _timeStart;
	int _timeLength;
	QString _name;
};
#endif /* MUSELEMENT_H_ */
