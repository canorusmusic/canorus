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
		void splitVertically(CAViewPort *v = 0);
		void splitHorizontally(CAViewPort *v = 0);
		void unsplit(CAViewPort *);
		int getViewPortsRows() {return layout_->rowCount();}
		int getViewPortsColumns() {return layout_->columnCount();}

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
		QList<CAViewPort *> viewPorts_;	//list of all visible viewports (multiple viewports happen when you use split view etc.)
		QGridLayout *layout_;	//layout of the viewports widgets
		CAViewPort *lastUsedViewPort_;	//the last viewport user interacted with
};
#endif

