/*!
	Copyright (c) 2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "layout/drawablechordname.h"
#include "layout/drawablechordnamecontext.h"

#include "score/chordnamecontext.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>

const double CADrawableChordName::DEFAULT_TEXT_SIZE = 16;

CADrawableChordName::CADrawableChordName(CAChordName* s, CADrawableChordNameContext* c, double x, double y)
    : CADrawableMusElement(s, c, x, y)
{
    setDrawableMusElementType(DrawableChordName);

    // some work to compute the drawable width
    QFont font("Century Schoolbook L");
    font.setPixelSize(qRound(DEFAULT_TEXT_SIZE));
    QFontMetricsF fm(font);
    qreal textWidth;
    if (!drawableDiatonicPitch().isEmpty()) {
        textWidth = fm.width(drawableDiatonicPitch());
        font.setPixelSize(qRound(DEFAULT_TEXT_SIZE * 0.75));
        fm = QFontMetricsF(font);
        textWidth += fm.width(chordName()->qualityModifier());
    } else {
        // syntax error, print qualityModifier() which includes everything
        textWidth = fm.width(chordName()->qualityModifier());
    }
    setWidth(textWidth < 11 ? 11 : textWidth); // set minimum text width at least 11 points

    setHeight(qRound(DEFAULT_TEXT_SIZE));
}

CADrawableChordName::~CADrawableChordName()
{
}

void CADrawableChordName::draw(QPainter* p, const CADrawSettings s)
{
    QPen pen(s.color);
    pen.setWidth(qRound(1.2 * s.z));
    pen.setCapStyle(Qt::RoundCap);
    p->setPen(pen);
    QFont font("Century Schoolbook L");
    font.setPixelSize(qRound(DEFAULT_TEXT_SIZE * s.z));
    p->setFont(font);
    QString dChordPitch = drawableDiatonicPitch();
    if (dChordPitch.isEmpty()) {
        // syntax error, print qualityModifier() which includes everything
        p->drawText(s.x, s.y + qRound(height() * s.z), chordName()->qualityModifier());
        return;
    }

    p->drawText(s.x, s.y + qRound(height() * s.z), dChordPitch);
    QFontMetricsF fm(font);
    qreal w = fm.width(dChordPitch);

    font.setPixelSize(qRound(DEFAULT_TEXT_SIZE * s.z * 0.75));
    p->setFont(font);
    p->drawText(s.x + w, s.y + qRound(height() * s.z * 0.5), chordName()->qualityModifier());
}

CADrawableChordName* CADrawableChordName::clone(CADrawableContext* c)
{
    return new CADrawableChordName(
        chordName(),
        (c ? static_cast<CADrawableChordNameContext*>(c) : static_cast<CADrawableChordNameContext*>(drawableContext())),
        xPos(),
        yPos());
}

/*!
 * \brief Converts CADiatonicPitch to a chord-style pitch, e.g. "cis" -> "C#"
 * \return Chord-style pitch as QString
 */
QString CADrawableChordName::drawableDiatonicPitch()
{
    QString chordPitch = CADiatonicPitch::diatonicPitchToString(chordName()->diatonicPitch());
    if (chordPitch.isEmpty()) {
        return "";
    }

    chordPitch = chordPitch[0].toUpper(); // chord-style pitch is upper case

    // now add sharps or flats
    if (chordName()->diatonicPitch().accs() < 0) {
        chordPitch += QString("b").repeated(chordName()->diatonicPitch().accs() * (-1));
    } else if (chordName()->diatonicPitch().accs() > 0) {
        chordPitch += QString("#").repeated(chordName()->diatonicPitch().accs());
    }
    return chordPitch;
}
