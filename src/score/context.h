/*!
	Copyright (c) 2006-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <QString>

#include <memory>

class CASheet;
class CAMusElement;

class CAContext {
public:
    CAContext(const QString name, CASheet* s);
    virtual ~CAContext();
    virtual std::shared_ptr<CAContext> cloneRealContext(CASheet*) = 0;

    enum CAContextType {
        Staff,
        LyricsContext,
        FunctionMarkContext,
        FiguredBassContext,
        ChordNameContext
    };

    const QString name() { return _name; }
    void setName(const QString name) { _name = name; }

    CAContextType contextType() { return _contextType; }

    CASheet* sheet() { return _sheet; }
    void setSheet(CASheet* sheet) { _sheet = sheet; }

    virtual void clear() = 0;
    virtual CAMusElement* next(CAMusElement* elt) = 0;
    virtual CAMusElement* previous(CAMusElement* elt) = 0;
    virtual bool remove(CAMusElement* elt) = 0;

protected:
    void setContextType(CAContextType t) { _contextType = t; }

    CASheet* _sheet;
    QString _name;
    CAContextType _contextType;
};
#endif /* CONTEXT_H_ */
