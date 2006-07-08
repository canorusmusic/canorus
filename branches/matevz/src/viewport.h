#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QWidget>
#include <QPaintEvent>
#include <QScrollBar>
#include <QGridLayout>

#include "kdtree.h"

class CAViewPort : public QWidget {
	Q_OBJECT
	
	public:
		CAViewPort(QWidget *, CAKDTree *);
		//widget properties
		inline const int drawableWidth() {return canvas_->width();}
		inline const int drawableHeight() {return canvas_->height();}

		//scrollbars
		void setManualScroll(bool scroll) {allowManualScroll_ = scroll;}
		bool manualScroll() {return allowManualScroll_;}
		void checkScrollBars();

		//main actions
		void setWorldCoords(int x, int y, int w, int h, bool force=false);
		void setWorldCoords(QRect *r, bool force=false) {setWorldCoords(r->x(),r->y(),r->width(),r->height());}
		void setWorldCoords(int x, int y, bool force=false);
		void setCenterCoords(int x, int y, bool force=false);
		void setZoom(float z, int x=0, int y=0, bool force = false);
		void setWorldX(int x, bool force=false); 
		void setWorldY(int y, bool force=false);
		void setWorldWidth(int w, bool force=false);
		void setWorldHeight(int h, bool force=false);

		const int worldX() {return worldX_;}
		const int worldY() {return worldY_;}
		const int worldWidth() {return worldW_;}
		const int worldHeight() {return worldH_;}
		const float zoom() {return zoom_;}

		void setScrollBarsVisibility(char status);	//use the enum below for the status
		enum {ScrollBarAlwaysVisible, ScrollBarAlwaysHidden, ScrollBarShowIfNeeded};
	
	signals:
		void mousePressEvent(QMouseEvent *);
		void wheelEvent(QWheelEvent *);
		void CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *);
		void CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *);
	
	private slots:
		void processMousePressEvent(QMouseEvent *);
		void processWheelEvent(QWheelEvent *);
		void processVScrollBarEvent(int val);
		void processHScrollBarEvent(int val);
		void resizeEvent(QResizeEvent *);
		void paintEvent(QPaintEvent *);
		
	private:
		CAKDTree *musElements_;
		int worldX_, worldY_, worldW_, worldH_;
		int oldWorldX_, oldWorldY_, oldWorldW_, oldWorldH_;	//used for partial repaint
		float zoom_;

		//widgets
		QWidget *parent_;
		QGridLayout *layout_;	//grid layout for placing the scrollbars at the right and the bottom
		QWidget *canvas_;	//virtual canvas which represents the size of the drawable area
		QScrollBar *hScrollBar_, *vScrollBar_;	//horizontal/vertical scrollbars
		char scrollBarsVisible_; //are the scrollbars always visible/if needed/never
		bool allowManualScroll_; //does the scrollbars actually react on user actions - sometimes we only want the scrollbars to show the current location of the score and don't do anything
		bool holdRepaint_;	//dirty flag to prevent multiple repainting - use when you know what you're doing!
		bool checkScrollBarsDeadLock_;	//dirty flag to prevent recursive checkScrollBars() calls
		bool hScrollBarDeadLock_;	//dirty flag to prevent recursive scrollbar calls when its value is manually changed
		bool vScrollBarDeadLock_;	//dirty flag to prevent recursive scrollbar calls when its value is manually changed
};
#endif
