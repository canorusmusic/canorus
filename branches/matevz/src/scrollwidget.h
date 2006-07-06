#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QFrame>
#include <QGridLayout>

#include "kdtree.h"
#include "viewport.h"

class CAScrollWidget : public QFrame {
	Q_OBJECT

	public:
		CAScrollWidget(QWidget *);
		void scrollToElement(CADrawable *);

	signals:		
		void mousePressEvent(QMouseEvent *);
		void wheelEvent(QWheelEvent *);

	private slots:
		void viewPortMousePressEvent(QMouseEvent *, QPoint coords, CAViewPort *);
		void viewPortWheelEvent(QWheelEvent *, QPoint coords, CAViewPort *);
		
	private:
		//events
		void paintEvent(QPaintEvent *);
		void resizeEvent(QResizeEvent *);
		
		void checkScrollBars();

		CAKDTree musElements_;	//list of all elements

		//Widgets
		CAViewPort *viewPort_;	//main canvas for drawing the music elements
		bool vScrollVisible_, hScrollVisible_;	//are the vertical/horizontal scrollbars visible or not
		QGridLayout *layout_;	//layout of the main widget, vertical scrollbar on the left and horizontal scrollbar at the bottom
};
#endif

