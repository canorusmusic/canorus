#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QFrame>
#include <QScrollBar>
#include <QGridLayout>

#include "kdtree.h"
#include "viewport.h"

class CAScrollWidget : public QFrame {
	Q_OBJECT

	public:
		CAScrollWidget(QWidget *);
		void setZoom(float z, int x, int y);	//zoom to level
		void scroll(int x, int y);	//scroll the viewport to the given coordinates
		void scrollToElement(CADrawable *);

	signals:		
		void mousePressEvent(QMouseEvent *);
		void wheelEvent(QWheelEvent *);

	private slots:
		void scrollX(int x);
		void scrollY(int y);
		void processMouseEvent(QMouseEvent *);
		void processWheelEvent(QWheelEvent *);
		
	private:
		//events
		void paintEvent(QPaintEvent *);
		
		void checkScrollBars();

		CAKDTree musElements_;	//list of all elements

		//Widgets
		CAViewPort *canvas_;	//main canvas for drawing the music elements
		QScrollBar *hScrollBar_, *vScrollBar_;	//horizontal/vertical scrollbars
		bool vScrollVisible_, hScrollVisible_;	//are the vertical/horizontal scrollbars visible or not
		QGridLayout *layout_;	//layout of the main widget, vertical scrollbar on the left and horizontal scrollbar at the bottom
};
#endif

