/*****************************************************************************/
/*									     */
/* This program is free software; you can redistribute it and/or modify it   */
/* under the terms of the GNU General Public License as published by the     */ 
/* Free Software Foundation; version 2 of the License.	                     */
/*									     */
/* This program is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY WARRANTY; without even the implied warranty of                */ 
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  */
/* Public License for more details.                                          */
/*									     */
/* You should have received a copy of the GNU General Public License along   */
/* with this program; (See "LICENSE.GPL"). If not, write to the Free         */
/* Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA        */
/* 02111-1307, USA.				                             */
/*									     */
/*---------------------------------------------------------------------------*/
/*									     */
/*		Reinhard Katzmann, GERMANY			             */
/*		reinhard@suamor.de					     */
/*									     */
/*		Matevž Jekovec, SLOVENIA			             */
/*		matevz.jekovec@guest.arnes.si				     */
/*									     */
/*****************************************************************************/

#include "ui_mainwin.h"

#include "document.h"

enum CAMode {
	InsertMode,
	SelectMode,
	ReadOnlyMode,
	PlayMode
};

class CAViewPort;

class CAMainWin: public QMainWindow
{
	Q_OBJECT

public:
	CAMainWin(QMainWindow *oParent = 0);
	~CAMainWin() {};
	
	void newDocument();
	
	////////////////////////////////////////////////////
	//Sheet operations
	////////////////////////////////////////////////////
	void addSheet();
	
	/**
	 * Delete all viewports and its contents.
	 * Delete all signals.
	 * Release all buttons and modes.
	 * 
	 * WARNING! This function delets the UI only (drawable elements). All the data classes should stay intact. Use _document.clear() in order to clear the data part as well.
	 */
	void clearUI();

private slots:
	////////////////////////////////////////////////////
	//Menu bar actions
	////////////////////////////////////////////////////
	//File menu
	void on_actionNew_activated();
	void on_actionNew_sheet_activated();
	
	//Insert menu
	void on_actionNew_staff_activated();
	
	//View menu
	void on_action_Fullscreen_toggled(bool);
	
	//Window menu
	void on_actionSplit_horizontally_activated();
	void on_actionSplit_vertically_activated();
	void on_actionUnsplit_activated();
	void on_actionNew_viewport_activated();

	////////////////////////////////////////////////////
	//ViewPort actions
	////////////////////////////////////////////////////
	/**
	 * Process the mouse events of the children viewports.
	 * 
	 * @param e Mouse event which gets processed.
	 * @param coords Absolute world coordinates where the mouse cursor was at time of the event.
	 * @param v Pointer to viewport where the event happened.
	 */
	void viewPortMousePressEvent(QMouseEvent *e, QPoint coords, CAViewPort *v);
	
	/**
	 * Process the wheel events of the children viewports.
	 * 
	 * @param e Wheel event which gets processed.
	 * @param coords Absolute world coordinates where the mouse cursor was at time of the event.
	 * @param v Pointer to viewport where the event happened.
	 */
	void viewPortWheelEvent(QWheelEvent *e, QPoint coords, CAViewPort *v);

private:
#define _currentScrollWidget ((CAScrollWidget*)(oMainWin.tabWidget->currentWidget()))
	CAMode _currentMode;
	QList<CAViewPort *> _viewPortList;
	CAViewPort *_activeViewPort;	
    Ui::MainWindow oMainWin;
    CADocument _document;
};
