/*!
        Copyright (c) 2009, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QString>
#include <QStringList>
#include <iostream> // debug

#include "control/stylusctl.h"
#include "ui/mainwin.h"
#include "canorus.h"

#include "core/playablelength.h"
#include "core/clef.h"
#include "core/muselementfactory.h"

CAStylusCtl::CAStylusCtl() {
	if (CACanorus::locateResourceDir("images/styluscodebook/barline.png").size()) {
		initSamples( CACanorus::locateResourceDir("images/styluscodebook/barline.png")[0] );
	}
}

CAStylusCtl::~CAStylusCtl() {
}

void CAStylusCtl::initSamples( QDir samplesDir ) {
	QStringList samples = samplesDir.entryList(QStringList() << "*.png");

	for (int i=0; i<samples.size(); i++) {
		_samples << QImage(samplesDir.absolutePath()+"/"+samples[i]);
		_sampleNames << samples[i].left( samples[i].length()-4 );
	}
}

void CAStylusCtl::detectAndInsertElement( QImage& stylusMask, QPoint centerCoords, CAMainWin* mainWin ) {
	QImage croppedImage = cropImage(stylusMask);
	QImage resizedImage = croppedImage.scaled( 64, 64 );

	QList<int> scores = corr( resizedImage );

	int maxScoreIdx=-1;
	for (int i=0; i<scores.size(); i++) {
		if ( maxScoreIdx==-1 || scores[maxScoreIdx] < scores[i] ) {
			maxScoreIdx = i;
		}
	}

	if (maxScoreIdx==-1) {
		return;
	}

	if ( _sampleNames[maxScoreIdx] == "n2" ) {
		mainWin->musElementFactory()->setMusElementType( CAMusElement::Note );
		mainWin->musElementFactory()->setPlayableLength( CAPlayableLength(CAPlayableLength::Half) );
	} else
	if ( _sampleNames[maxScoreIdx] == "n4" ) {
		mainWin->musElementFactory()->setMusElementType( CAMusElement::Note );
		mainWin->musElementFactory()->setPlayableLength( CAPlayableLength(CAPlayableLength::Quarter) );
	} else
	if ( _sampleNames[maxScoreIdx] == "n8" ) {
		mainWin->musElementFactory()->setMusElementType( CAMusElement::Note );
		mainWin->musElementFactory()->setPlayableLength( CAPlayableLength(CAPlayableLength::Eighth) );
	} else
	if ( _sampleNames[maxScoreIdx] == "barline" ) {
		mainWin->musElementFactory()->setMusElementType( CAMusElement::Barline );
	} else
	if ( _sampleNames[maxScoreIdx] == "clefg" ) {
		mainWin->musElementFactory()->setMusElementType( CAMusElement::Clef );
		mainWin->musElementFactory()->setClef( CAClef::Treble );
	} else
	if ( _sampleNames[maxScoreIdx] == "cleff" ) {
		mainWin->musElementFactory()->setMusElementType( CAMusElement::Clef );
		mainWin->musElementFactory()->setClef( CAClef::Bass );
	}

	mainWin->insertMusElementAt( centerCoords, mainWin->currentScoreViewPort() );
}

/*!
	Crops image to exactly fit the content.
 */
QImage CAStylusCtl::cropImage( QImage &in ) {
	int minX=-1, minY=-1, maxX=-1, maxY=-1;

	for (int i=0; i<in.height(); i++) {
		for (int j=0; j<in.width(); j++) {
			if (in.pixelIndex( j, i )==1) {
				if ( minX==-1 || minX > j ) {
					minX = j;
				}
				if ( minY==-1 || minY > i ) {
					minY = i;
				}
				if ( maxX==-1 || maxX < j ) {
					maxX = j;
				}
				if ( maxY==-1 || maxY < j ) {
					maxY = i;
				}
			}
		}
	}

	if ( minX!=-1 && minY!=-1 && maxX!=-1 && maxY!=-1 ) {
		return in.copy( minX, minY, maxX-minX, maxY-minY );
	} else {
		return in.copy();
	}
}

/*!
	Cross correlation between input image \a in and sample images (codebook).
	Returns list of scores for the first throught the last sample.

	\sa _samples
 */
QList<int> CAStylusCtl::corr( QImage& in ) {
	QList<int> scores;

	for (int i=0; i<_samples.size(); i++) {
		int score=0;

		for (int y=0; y<in.height(); y++) {
			for (int x=0; x<in.width(); x++) {
				if ( in.pixelIndex( x, y ) == _samples[i].pixelIndex( x, y ) ) {
					score++;
				}
			}
		}

		scores << score;
	}

	return scores;
}
