/*!
        Copyright (c) 2009, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef STYLUSCTL_H_
#define STYLUSCTL_H_

#include <QImage>
#include <QPoint>
#include <QList>
#include <QDir>

class CAMainWin;

class CAStylusCtl {
public:
	CAStylusCtl();
	virtual ~CAStylusCtl();
	void detectAndInsertElement( QImage&, QPoint, CAMainWin* );

private:
	void initSamples(QDir samplesDir);
	QImage cropImage( QImage& in );
	QList<int> corr( QImage& in );

	QList<QImage>  _samples;     // codebook
	QList<QString> _sampleNames; // name of sample files
};

#endif /* STYLUSCTL_H_ */
