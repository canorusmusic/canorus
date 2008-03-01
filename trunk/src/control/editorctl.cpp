/*!
        Copyright (c) 2007, Reinhard Katzmann, Canorus development team
        
        All Rights Reserved. See AUTHORS for a complete list of authors.
        
        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

//#include <Python.h> must be called before standard headers inclusion. See http://docs.python.org/api/includes.html
#ifdef USE_PYTHON
#include <Python.h>
#endif

#include <QtGui>
#include <QSlider>
#include <QComboBox>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPoint>
#include <QKeyEvent>
#include <QString>
#include <QTextStream>
#include <QXmlInputSource>
#include <QComboBox>
#include <QThread>
#include <QMessageBox>
#include <iostream>

#include "ui/settingsdialog.h"
#include "ui/propertiesdialog.h"

#include "interface/playback.h"
#include "interface/engraver.h"
#include "interface/pluginmanager.h"
#include "interface/mididevice.h"
#include "interface/rtmididevice.h"

#include "widgets/viewport.h"
#include "widgets/viewportcontainer.h"
#include "widgets/scoreviewport.h"
#include "widgets/sourceviewport.h"

#include "drawable/drawablecontext.h"
#include "drawable/drawablestaff.h"
#include "drawable/drawablelyricscontext.h"
#include "drawable/drawablemuselement.h"
#include "drawable/drawablenote.h"

#include "canorus.h"
#include "core/editorctl.h"
#include "core/settings.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/functionmarkingcontext.h"
#include "core/lyricscontext.h"
#include "core/clef.h"
#include "core/keysignature.h"
#include "core/note.h"
#include "core/slur.h"
#include "core/rest.h"
#include "core/voice.h"
#include "core/barline.h"
#include "core/timesignature.h"
#include "core/syllable.h"
#include "core/functionmarking.h"
#include "core/muselementfactory.h"
#include "core/mimedata.h"
#include "core/undo.h"

#include "scripting/swigruby.h"
#include "scripting/swigpython.h"

#include "export/lilypondexport.h"
#include "export/canorusmlexport.h"
#include "import/lilypondimport.h"
#include "import/canorusmlimport.h"

CAEditorCtl::CAEditorCtl()
{
	// Initialize internal UI properties
	_mode = SelectMode;
	_playback = 0;
	_animatedScroll = true;
	_lockScrollPlayback = false;

	_iVoiceNum = 0;
	_iMaxVoiceNum = 20;
	_bSelectMode = true;
	
	_musElementFactory = new CAMusElementFactory();
}

void CAEditorCtl::newDocument() {
	// clear GUI before clearing the data part!
	clearUI();
	
	// clear the data part
	if ( document() && (CACanorus::mainWinCount(document()) == 1) ) {
		CACanorus::undo()->deleteUndoStack( document() ); 
		delete document();
	}
	
	setDocument(new CADocument());
	CACanorus::undo()->createUndoStack( document() );
	restartTimeEditedTime();
	
#ifdef USE_PYTHON
	QList<PyObject*> argsPython;
	argsPython << CASwigPython::toPythonObject(document(), CASwigPython::Document);
	CASwigPython::callFunction(CACanorus::locateResource("scripts/newdocument.py").at(0), "newDefaultDocument", argsPython);
#endif
	
	// call local rebuild only because no other main windows share the new document
	rebuildUI( true, 0 );
	
	// select the first context automatically
	if ( document()->sheetCount() && document()->sheetAt(0)->contextCount() )
		currentScoreViewPort()->selectContext( document()->sheetAt(0)->contextAt(0) );
}

void CAEditorCtl::addSheet(CASheet *s) {
	CAScoreViewPort *v = new CAScoreViewPort(s, 0);
	initViewPort( v );
	
	CAViewPortContainer *vpc = new CAViewPortContainer( 0 );
	vpc->addViewPort( v );
	_viewPortContainerList << vpc;
	_sheetMap[vpc] = s;
	
	setCurrentViewPortContainer( vpc );
}

/*!
	Deletes all viewports (and their drawable content), disconnects all signals and resets all
	buttons and modes.
	
	This function deletes the current editor window's GUI only (drawable elements). All the data
	classes (staffs, notes, rests) should stay intact. Use delete document() to free the data
	part of Canorus as well.
*/
void CAEditorCtl::clearUI() {
	// Delete all viewports
	while(!_viewPortList.isEmpty())
		delete _viewPortList.takeFirst();
	_viewPortList.clear();
	_sheetMap.clear();
	setCurrentViewPort( 0 );
}

/*!
	Adds a new voice to the staff.
*/
int CAEditorCtl::newVoice_triggered() {
	CAStaff *staff = currentStaff();
	int voiceNumber = staff->voiceCount()+1;
	CANote::CAStemDirection stemDirection;
	if ( voiceNumber == 1 )
		stemDirection = CANote::StemNeutral;
	else {
		staff->voiceAt(0)->setStemDirection( CANote::StemUp );
		stemDirection = CANote::StemDown;
	}
	
	CACanorus::undo()->createUndoCommand( document(), QObject::tr("new voice", "undo") );
	if (staff) {
		staff->addVoice(new CAVoice( staff->name() + QObject::tr("Voice%1").arg( staff->voiceCount()+1 ), staff, stemDirection, voiceNumber ));
		staff->synchronizeVoices();
	}
	
	CACanorus::undo()->pushUndoCommand();
	CACanorus::rebuildUI(document(), currentSheet());
	return staff->voiceCount();
}

void CAEditorCtl::hiddenRest_toggled( bool checked ) {
	if ( mode()==InsertMode )
		musElementFactory()->setRestType( checked ? CARest::Hidden : CARest::Normal );
	else if (mode()==SelectMode || mode()==EditMode) {
		CAScoreViewPort *v = currentScoreViewPort();
		if ( v && v->selection().size() ) {
			CARest *rest = dynamic_cast<CARest*>(v->selection().at(0)->musElement());
			if ( rest ) {
				rest->setRestType( checked ? CARest::Hidden : CARest::Normal );
				CACanorus::rebuildUI( document(), currentSheet() );
			}
		}
	}
}

/*!
	Removes the current voice from the staff and deletes its contents.
*/
int CAEditorCtl::removeVoice_triggered() {
	int iVNum = -1;
	CAVoice *voice = currentVoice();
	if (voice) {
		// Last voice cannot be deleted
		if (voice->staff()->voiceCount()==1) {
			int ret = QMessageBox::critical(
				this, QObject::tr("Canorus"),
				QObject::tr("Cannot delete the last voice in the staff!")
			);
			return iVNum;
		}
		
		int ret = QMessageBox::warning(
			this, QObject::tr("Canorus"),
			QObject::tr("Are you sure do you want to delete voice\n%1 and all its notes?").arg(voice->name()),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No);
		
		if (ret == QMessageBox::Yes) {
			CACanorus::undo()->createUndoCommand( document(), QObject::tr("voice removal", "undo") );
			currentScoreViewPort()->clearSelection();
			iVNum = voice->staff()->voiceCount()-1;
			delete voice; // also removes voice from the staff
			CACanorus::undo()->pushUndoCommand();
			CACanorus::rebuildUI(document(), currentSheet());
		}
	}
	return iVNum;
}

/*!
	Removes the current context from the sheet and all its contents.
*/
void CAEditorCtl::removeContext_triggered() {
	CAContext *context = currentContext();
	if (context) {
		int ret = QMessageBox::warning(
			this, QObject::tr("Canorus"),
			QObject::tr("Are you sure do you want to delete context\n%1 and all its contents?").arg(context->name()),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No);
		
		if (ret == QMessageBox::Yes) {
			CACanorus::undo()->createUndoCommand( document(), QObject::tr("context removal", "undo") );
			CASheet *sheet = context->sheet();
			sheet->removeContext(context);
			CACanorus::undo()->pushUndoCommand();			
			CACanorus::rebuildUI(document(), currentSheet());
			delete context;
		}
	}
}

/*!
	Rebuilds the GUI from data.
	
	This method is called eg. when multiple viewports share the same data and a change has been made (eg. a
	note pitch has changed or a new element added). ViewPorts content is repositioned and redrawn (CAEngraver
	creates CADrawable elements for every score viewport, sources are updated in source viewports etc.).
	
	\a sheet argument is a pointer to the data sheet where the change occured. This way only viewports showing
	the given sheet are updated which speeds up the process.
	If \a sheet argument is null, all viewports are rebuilt, but the viewports contents, number and locations
	remain the same.
	
	If \a repaint is True (default) the rebuilt viewports are also repainted. If False, viewports content is
	only created but not yet drawn. This is useful when multiple operations which could potentially change the
	content are to happen and we want to actually draw it only at the end.
*/
bool CAEditorCtl::rebuildUI(CASheet *sheet, bool repaint) {
	if (rebuildUILock()) return false;
	
	setRebuildUILock( true );
	if (document()) {
		for (int i=0; i<_viewPortList.size(); i++) {
			if (sheet && _viewPortList[i]->viewPortType()==CAViewPort::ScoreViewPort &&
			    static_cast<CAScoreViewPort*>(_viewPortList[i])->sheet()!=sheet)
				continue;
			
			_viewPortList[i]->rebuild();
			
			if (_viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort)
				static_cast<CAScoreViewPort*>(_viewPortList[i])->checkScrollBars();
				
			if (repaint)
				_viewPortList[i]->repaint();
		}
	} else {
		clearUI();
	}
	setRebuildUILock( false );
	return true;
}

/*!
	Rebuilds the GUI from data.
	
	This method is called eg. when multiple viewports share the same data and a change has been made (eg. a
	note pitch has changed or a new element added). ViewPorts content is repositioned and redrawn (CAEngraver
	creates CADrawable elements for every score viewport, sources are updated in source viewports etc.).
	
	This method in comparison to CAEditorWin::rebuildUI(CASheet *s, bool repaint) rebuilds the whole GUI from
	scratch and creates new viewports for the sheets. This method is called for example when a new document
	is created or opened.
	
	If \a repaint is True (default) the rebuilt viewports are also repainted. If False, viewports content is
	only created but not yet drawn. This is useful when multiple operations which could potentially change the
	content are to happen and we want to actually draw it only at the end.
*/
bool CAEditorCtl::rebuildUI( bool repaint, int curIndex ) {
	if (rebuildUILock()) return false;
	
	setRebuildUILock( true );
	if (document()) {		
		// save the current state of viewports
		QList<QRect> worldCoordsList;
		for (int i=0; i<_viewPortList.size(); i++)
			if (_viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort)
				worldCoordsList << static_cast<CAScoreViewPort*>(_viewPortList[i])->worldCoords();
		
		clearUI();
		for (int i=0; i<document()->sheetCount(); i++) {
			addSheet(document()->sheetAt(i));
			
			// restore the current state of viewports
			if ( _viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort &&
			     i < worldCoordsList.size() )
				static_cast<CAScoreViewPort*>(_viewPortList[i])->setWorldCoords(worldCoordsList[i]);
		}
		
		for (int i=0; i<_viewPortList.size(); i++) {
			_viewPortList[i]->rebuild();
			
			if (_viewPortList[i]->viewPortType() == CAViewPort::ScoreViewPort)
				static_cast<CAScoreViewPort*>(_viewPortList[i])->checkScrollBars();
				
			if (repaint)
				_viewPortList[i]->repaint();
		}		
	} else {
		clearUI();
	}
	setRebuildUILock( false );
	return true;
}

/*!
	Returns the currently selected context in the current view port or 0 if no contexts are selected.
*/
CAContext *CAEditorCtl::currentContext() {
	if ( currentViewPort() &&
	     (currentViewPort()->viewPortType() == CAViewPort::ScoreViewPort) &&
	     (static_cast<CAScoreViewPort*>(currentViewPort())->currentContext())
	   ) {
		return static_cast<CAScoreViewPort*>(currentViewPort())->currentContext()->context();
	} else
		return 0;
}

/*!
	Returns the pointer to the currently active voice or 0, if All voices are selected or the current context is not a staff at all.
*/
CAVoice *CAEditorCtl::currentVoice() {
	CAStaff *staff = currentStaff();
	if (staff) {
		if ( _iVoiceNum &&
		     _iVoiceNum <= staff->voiceCount())
			return staff->voiceAt( _iVoiceNum - 1);
	}
	
	return 0;
}

/*!
	Sets the current mode and updates the GUI and toolbars.
*/
void CAEditorCtl::setMode(CAMode mode) {
_mode = mode;

switch (mode) {
	case SelectMode: {
		for (int i=0; i<_viewPortList.size(); i++) {
			if ( currentScoreViewPort() ) {
				if (!currentScoreViewPort()->playing())
					currentScoreViewPort()->unsetBorder();
				currentScoreViewPort()->setShadowNoteVisible(false);
				if (currentScoreViewPort()->syllableEditVisible())
					currentScoreViewPort()->removeSyllableEdit();
				musElementFactory()->setMusElementType( CAMusElement::Undefined );
				_iVoiceNum = 0;
				_viewPortList[i]->repaint();
			}
		}
		break;
	}
	case InsertMode: {
		QPen p;
		p.setColor(Qt::blue);
		p.setWidth(3);
		
		if ( currentScoreViewPort() )
			currentScoreViewPort()->setShadowNoteVisible((musElementFactory()->musElementType() == CAMusElement::Note) ? true : false); /// \todo Set other mouse cursors

		for (int i=0; i<_viewPortList.size(); i++) {
			if (_viewPortList[i]->viewPortType()==CAViewPort::ScoreViewPort) {
				if (!((CAScoreViewPort*)_viewPortList[i])->playing())
					((CAScoreViewPort*)_viewPortList[i])->setBorder(p);
				((CAScoreViewPort*)_viewPortList[i])->repaint();
			}
		}

		break;
	}
	case EditMode: {
		QPen p;
		p.setColor(Qt::red);
		p.setWidth(3);
		for (int i=0; i<_viewPortList.size(); i++) {
			if (_viewPortList[i]->viewPortType()==CAViewPort::ScoreViewPort) {
				CAScoreViewPort *sv = static_cast<CAScoreViewPort*>(_viewPortList[i]);
				if (!sv->playing())
					(sv->setBorder(p));
				
				sv->setShadowNoteVisible(false);
				sv->repaint();
			}
		}
		
		if (currentScoreViewPort()) {
			if (currentScoreViewPort()->selection().size() &&
			    currentScoreViewPort()->selection().front()->drawableMusElementType()==CADrawableMusElement::DrawableSyllable
			) {
				currentScoreViewPort()->createSyllableEdit(currentScoreViewPort()->selection().front());
			} else {
				currentScoreViewPort()->removeSyllableEdit();
			}
		}
	}
}	// switch (mode)
if ( currentScoreViewPort() && !currentScoreViewPort()->syllableEditVisible() ||
     !currentScoreViewPort() && currentViewPort() )
	currentViewPort()->setFocus();
}

/*!
	Processes the mouse press event \a e with world coordinates \a coords of the score viewport \a v.
	Any action happened in any of the viewports are always linked to these main window slots.
	
	\sa CAScoreViewPort::mousePressEvent(), scoreViewPortMouseMove(), scoreViewPortWheel(), scoreViewPortKeyPress()
*/
void CAEditorCtl::scoreViewPortMousePress(QMouseEvent *e, const QPoint coords, CAScoreViewPort *v,
		                                  bool bSyllable, bool bPlayable) {
	setCurrentViewPort( v );
	currentViewPortContainer()->setCurrentViewPort( currentViewPort() );
	
	CADrawableContext *prevContext = v->currentContext();
	v->selectCElement(coords.x(), coords.y());
		
	QList<CADrawableMusElement*> l = v->musElementsAt( coords.x(), coords.y() );
	CADrawableMusElement *newlySelectedElement=0;
	int idx=-1;
	
	if (l.size() > 0) { // multiple elements can share the same coordinates
		if ( (v->selection().size() > 0) && (!v->selection().contains(l.front())) ) {
			if (e->modifiers()!=Qt::ShiftModifier)
				v->clearSelection();
			v->addToSelection( newlySelectedElement = l[0] );      // if the previous selection was not a single element or if the new list doesn't contain the selection set the first element in the available list to the selection
		} else {
			if (e->modifiers()==Qt::ShiftModifier && v->selection().size()) {
				v->removeFromSelection(l[0]); // shift used on an already selected element - toggle selection
			} else {
				idx = (v->selection().size()?l.indexOf(v->selection().front()):-1);
				v->clearSelection();
				v->addToSelection( newlySelectedElement = l[((++idx < l.size()) ? idx : 0)] ); // if there are two or more elements with the same coordinates, select the next one (behind it). This way, you can click multiple times on the same place and you'll always select the other element.
			}
		}
	} else if (e->modifiers()==Qt::NoModifier) { // no elements at that coordinates
		v->clearSelection();
	}
	
	// always select the context the current element belongs to
	if ( newlySelectedElement && mode()!=InsertMode )
		v->setCurrentContext( newlySelectedElement->drawableContext() );
	
	if ( v->currentContext() && prevContext != v->currentContext() && mode()!=InsertMode) {	// new context was selected
		// voice number widget
		if (v->currentContext()->context()->contextType() == CAContext::Staff) {
			_iVoiceNum = 0;
			_iMaxVoiceNum = static_cast<CAStaff*>(v->currentContext()->context())->voiceCount();
		}
	} else if ( prevContext != v->currentContext() && bPlayable ) { // but insert playable mode is active and context should remain the same
		v->setCurrentContext( prevContext );
	}
	
	switch ( mode() ) {
		case SelectMode:
		case EditMode: {
			v->clearSelectionRegionList();
			
			if ( v->selection().size() ) {
				CADrawableMusElement *dElt = v->selection().front();
				CAMusElement *elt = dElt->musElement();
				if (!elt) break;
				
				if ( mode()==EditMode && dElt->drawableMusElementType()==CADrawableMusElement::DrawableSyllable ) {
					v->createSyllableEdit( dElt );
				}
				
				// debug
				std::cout << "drawableMusElement: " << dElt << ", x,y=" << dElt->xPos() << "," << dElt->yPos() << ", w,h=" << dElt->width() << "," << dElt->height() << ", dContext=" << dElt->drawableContext() << std::endl;
				std::cout << "musElement: " << elt << ", timeStart=" << elt->timeStart() << ", timeEnd=" << elt->timeEnd() << ", dContext = " << v->selection().front()->drawableContext() << ", context=" << elt->context();
				if (elt->isPlayable()) {
					std::cout << ", voice=" << ((CAPlayable*)elt)->voice() << ", voiceNr=" << ((CAPlayable*)elt)->voice()->voiceNumber() << ", idxInVoice=" << ((CAPlayable*)elt)->voice()->indexOf(elt);
					std::cout << ", voiceStaff=" << ((CAPlayable*)elt)->voice()->staff();
					if (elt->musElementType()==CAMusElement::Note)
						std::cout << ", pitch=" << ((CANote*)elt)->pitch();
				}
				std::cout << std::endl;
			} else {
				v->removeSyllableEdit();
			}
			
			break;
		}
		case InsertMode: {
			// Insert context
			if (uiContextType->isChecked()) {
				// Add new Context
				CAContext *newContext;
				CADrawableContext *dupContext = v->nearestUpContext(coords.x(), coords.y());
				switch(uiContextType->currentId()) {
					case CAContext::Staff: {
						CACanorus::undo()->createUndoCommand( document(), QObject::tr("new staff", "undo"));
						v->sheet()->insertContextAfter(
							dupContext?dupContext->context():0,
							newContext = new CAStaff(
								QObject::tr("Staff%1").arg(v->sheet()->staffCount()+1),
								v->sheet()
							)
						);
						static_cast<CAStaff*>(newContext)->addVoice();
						break;
					}
					case CAContext::LyricsContext: {
						CACanorus::undo()->createUndoCommand( document(), QObject::tr("new lyrics context", "undo"));
						
						int stanza=1;
						if (dupContext && dupContext->context() && dupContext->context()->contextType()==CAContext::LyricsContext)
							stanza = static_cast<CALyricsContext*>(dupContext->context())->stanzaNumber()+1;
							
						v->sheet()->insertContextAfter(
							dupContext?dupContext->context():0,
							newContext = new CALyricsContext(
								QObject::tr("LyricsContext%1").arg(v->sheet()->contextCount()+1),
								1,
								(v->sheet()->voiceList().size()?v->sheet()->voiceList().at(0):0)
							)
						);
						
						break;
					}
					case CAContext::FunctionMarkingContext: {
						CACanorus::undo()->createUndoCommand( document(), QObject::tr("new function marking context", "undo"));
						v->sheet()->insertContextAfter(
							dupContext?dupContext->context():0,
							newContext = new CAFunctionMarkingContext(
								QObject::tr("FunctionMarkingContext%1").arg(v->sheet()->contextCount()+1),
								v->sheet()
							)
						);
						break;
					}
				}
				CACanorus::undo()->pushUndoCommand();
				CACanorus::rebuildUI(document(), v->sheet());
				
				v->selectContext(newContext);
				if (newContext->contextType()==CAContext::Staff) {
					_iMaxVoiceNum = 1;
					_iVoiceNum = 0;
				}
				_bSelectMode = !_bSelectMode;
				v->repaint();
				break;
			} else
			// Insert Syllable
			if (bSyllable) {
				if ( v->currentContext() && v->currentContext()->context() &&
					 v->currentContext()->context()->contextType()==CAContext::LyricsContext && v->selection().size() &&
					 v->selection().front()->drawableMusElementType()==CADrawableMusElement::DrawableSyllable ) {
					CADrawableLyricsContext *dlc = static_cast<CADrawableLyricsContext*>(v->currentContext());
					v->createSyllableEdit( v->selection().front() );
				} else {
					v->removeSyllableEdit();
				}
				
				break;
			} else
			// Insert music element
			if (bPlayable) {
				// Add Note/Rest
				if (e->button()==Qt::RightButton && musElementFactory()->musElementType()==CAMusElement::Note)
					// place a rest when using right mouse button and note insertion is selected
					musElementFactory()->setMusElementType( CAMusElement::Rest );
			}
			
			insertMusElementAt( coords, v );
			
			if ( musElementFactory()->musElementType()==CAMusElement::Rest )
			     musElementFactory()->setMusElementType( CAMusElement::Note );
			
			break;
		}
	}
	
	CAPluginManager::action("onScoreViewPortClick", document(), 0, 0, this);
	v->repaint();
}

/*!
	Processes the mouse move event \a e with coordinates \a coords of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::mouseMoveEvent(), scoreViewPortMousePress(), scoreViewPortWheel(), scoreViewPortKeyPress()
*/
QString CAEditorCtl::scoreViewPortMouseMove(QMouseEvent *e, QPoint coords, CAScoreViewPort *c) {
	QString oMessage;
	if ( (mode() == InsertMode && musElementFactory()->musElementType() == CAMusElement::Note) ) {
		CADrawableStaff *s;
		if (c->currentContext()?(c->currentContext()->drawableContextType() == CADrawableContext::DrawableStaff):0)
			s = (CADrawableStaff*)c->currentContext(); 
		else
			return oMessage;

		if ( musElementFactory()->musElementType() == CAMusElement::Note || 
             musElementFactory()->musElementType() == CAMusElement::Rest) {
			c->setShadowNoteVisible(true);
        }
        
		// calculate the musical pitch out of absolute world coordinates and the current clef
		int pitch = s->calculatePitch(coords.x(), coords.y());
		
		// write into the main window's status bar the note pitch name
		int iNoteAccs = s->getAccs(coords.x(), pitch) + musElementFactory()->noteExtraAccs();
		musElementFactory()->setNoteAccs( iNoteAccs );
		oMessage = CANote::generateNoteName(pitch, iNoteAccs);
		c->setShadowNoteAccs(iNoteAccs);
		c->repaint();
	} else
	if ( mode()!=InsertMode  && e->buttons()==Qt::LeftButton ) { // multiple selection
		c->clearSelectionRegionList();
		int x=c->lastMousePressCoords().x(), y=c->lastMousePressCoords().y(),
		    w=coords.x()-c->lastMousePressCoords().x(), h=coords.y()-c->lastMousePressCoords().y();
		if (w<0) { x+=w; w*=(-1); } // user selected from right to left
		if (h<0) { y+=h; h*=(-1); } // user selected from bottom to top
		QRect selectionRect( x, y, w, h );
		
		QList<CADrawableContext*> dcList = c->findContextsInRegion( selectionRect );
		for (int i=0; i<dcList.size(); i++) {
			QList<CADrawableMusElement*> musEltList = dcList[i]->findInRange( selectionRect.x(), selectionRect.x() + selectionRect.width() );
			for (int j=0; j<musEltList.size(); j++)
				if (musEltList[j]->drawableMusElementType()==CADrawableMusElement::DrawableSlur)
					musEltList.removeAt(j--);
			
			if (musEltList.size()) {
				c->addSelectionRegion( QRect(musEltList.front()->xPos(), dcList[i]->yPos(),
				                             musEltList.back()->xPos()+musEltList.back()->width()-musEltList.front()->xPos(), dcList[i]->height()) );
			}
		}
		c->repaint();
	}
	return oMessage;
}

/*!
	Processes the mouse move event \a e with coordinates \a coords of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::mouseReleaseEvent(), scoreViewPortMousePress(), scoreViewPortMouseMove(), scoreViewPortWheel(), scoreViewPortKeyPress()
*/
void CAEditorCtl::scoreViewPortMouseRelease(QMouseEvent *e, QPoint coords, CAScoreViewPort *c) {
	if ( mode() != InsertMode  && c->lastMousePressCoords()!=coords ) { // area was selected
		c->clearSelectionRegionList();
		
		if (e->modifiers()==Qt::NoModifier)
			c->clearSelection();
		
		int x=c->lastMousePressCoords().x(), y=c->lastMousePressCoords().y(),
		    w=coords.x()-c->lastMousePressCoords().x(), h=coords.y()-c->lastMousePressCoords().y();
		if (w<0) { x+=w; w*=(-1); } // user selected from right to left
		if (h<0) { y+=h; h*=(-1); } // user selected from bottom to top
		QRect selectionRect( x, y, w, h );
		
		QList<CADrawableContext*> dcList = c->findContextsInRegion( selectionRect );
		for (int i=0; i<dcList.size(); i++) {
			QList<CADrawableMusElement*> musEltList = dcList[i]->findInRange( selectionRect.x(), selectionRect.x() + selectionRect.width() );
			if ( c->selectedVoice() && dcList[i]->context()!=c->selectedVoice()->staff() )
				continue;
			
			for (int j=0; j<musEltList.size(); j++)
				if (c->selectedVoice() && musEltList[j]->musElement()->isPlayable() && static_cast<CAPlayable*>(musEltList[j]->musElement())->voice()!=c->selectedVoice() ||
				    (!musEltList[j]->isSelectable()) ||
				    (musEltList[j]->drawableMusElementType()==CADrawableMusElement::DrawableSlur)
				)
					musEltList.removeAt(j--);
			c->addToSelection(musEltList);
		}
		c->repaint();
	}
}

/*!
	Processes the mouse wheel event \a e with coordinates \a coords of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::wheelEvent(), scoreViewPortMousePress(), scoreViewPortMouseMove(), scoreViewPortKeyPress()
*/
void CAEditorCtl::scoreViewPortWheel(QWheelEvent *e, QPoint coords, CAScoreViewPort *sv) {
	setCurrentViewPort( sv );
	
	int val;
	switch (e->modifiers()) {
		case Qt::NoModifier:			//scroll horizontally
			sv->setWorldX( sv->worldX() - (int)((0.5*e->delta()) / sv->zoom()), _animatedScroll );
			break;
		case Qt::AltModifier:			//scroll horizontally, fast
			sv->setWorldX( sv->worldX() - (int)(e->delta() / sv->zoom()), _animatedScroll );
			break;
		case Qt::ShiftModifier:			//scroll vertically
			sv->setWorldY( sv->worldY() - (int)((0.5*e->delta()) / sv->zoom()), _animatedScroll );
			break;
		case 0x0A000000://SHIFT+ALT		//scroll vertically, fast
			sv->setWorldY( sv->worldY() - (int)(e->delta() / sv->zoom()), _animatedScroll );
			break;
		case Qt::ControlModifier:		//zoom
			if (e->delta() > 0)
				sv->setZoom( sv->zoom()*1.1, coords.x(), coords.y(), _animatedScroll );
			else
				sv->setZoom( sv->zoom()/1.1, coords.x(), coords.y(), _animatedScroll );
			
			break;
	}
	
	sv->repaint();
}

/*!
	Processes the key press event \a e of the score viewport \a v.
	Any action happened in any of the viewports are always linked to its main window slots.
	
	\sa CAScoreViewPort::keyPressEvent(), scoreViewPortMousePress(), scoreViewPortMouseMove(), scoreViewPortWheel()
*/
void CAEditorCtl::scoreViewPortKeyPress(QKeyEvent *e, CAScoreViewPort *v) {
	setCurrentViewPort( v );
	
	switch (e->key()) {
		// Music editing keys
		case Qt::Key_Right: {
			// select next music element
			v->selectNextMusElement( e->modifiers()==Qt::ShiftModifier );
			v->repaint();
			break;
		}
		
		case Qt::Key_Left: {
			// select previous music element
			v->selectPrevMusElement( e->modifiers()==Qt::ShiftModifier );
			v->repaint();
			break;
		}
		
		case Qt::Key_B: {
			// place a barline
			CADrawableContext *drawableContext;
			drawableContext = v->currentContext();
			
			if ( (!drawableContext) || (drawableContext->context()->contextType() != CAContext::Staff) )
				return;
			
			CAStaff *staff = static_cast<CAStaff*>(drawableContext->context());
			CAMusElement *right = 0;
			if (!v->selection().isEmpty())
				right = staff->next(v->selection().back()->musElement());
			
			CACanorus::undo()->createUndoCommand( document(), QObject::tr("insert barline", "undo") );
			CABarline *bar = new CABarline(
				CABarline::Single,
				staff,
				0
			);
			
			if (currentVoice()) {
				currentVoice()->insert( right, bar ); // insert the barline in all the voices, timeStart is set
			} else {
				if ( right && right->isPlayable() )
					static_cast<CAPlayable*>(right)->voice()->insert( right, bar );
				else
					staff->voiceAt(0)->insert( right, bar );
			}
			
			staff->synchronizeVoices();
			
			CACanorus::undo()->pushUndoCommand();
			CACanorus::rebuildUI(document(), v->sheet());
			v->selectMElement(bar);
			v->repaint();
			break;
		}
		
		case Qt::Key_Up: {
			if (mode() == SelectMode) {	// select the upper music element
				v->selectUpMusElement();
				v->repaint();
			} else if ((mode() == InsertMode) || (mode() == EditMode)) {
				bool rebuild=false;
				if (v->selection().size())
					CACanorus::undo()->createUndoCommand( document(), QObject::tr("rise note", "undo") );
				
				for (int i=0; i<v->selection().size(); i++) {
					CADrawableMusElement *elt = v->selection().at(i);
					
					// pitch note for one step higher
					if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
						CANote *note = (CANote*)elt->musElement();
						note->setPitch(note->pitch()+1);
						CACanorus::undo()->pushUndoCommand();
						rebuild = true;
					}
				}
				if (rebuild)
					CACanorus::rebuildUI(document(), currentSheet());
			}
			break;
		}
		
		case Qt::Key_Down: {
			if (mode() == SelectMode) {	// select the upper music element
				v->selectUpMusElement();
				v->repaint();
			} else if ((mode() == InsertMode) || (mode() == EditMode)) {
				bool rebuild = false;
				if (v->selection().size())
					CACanorus::undo()->createUndoCommand( document(), QObject::tr("lower note", "undo") );
				
				for (int i=0; i<v->selection().size(); i++) {
					CADrawableMusElement *elt = v->selection().at(i);
					
					// pitch note for one step higher
					if (elt->drawableMusElementType() == CADrawableMusElement::DrawableNote) {
						CANote *note = (CANote*)elt->musElement();
						note->setPitch(note->pitch()-1);
						CACanorus::undo()->pushUndoCommand();
						rebuild = true;
					}
				}
				CACanorus::rebuildUI(document(), currentSheet());
			}
			break;
		}
		
		case Qt::Key_Plus: {
			if (mode()==InsertMode) {
				musElementFactory()->addNoteExtraAccs(1); musElementFactory()->addNoteAccs(1);
				v->setDrawShadowNoteAccs(musElementFactory()->noteExtraAccs()!=0);
				v->setShadowNoteAccs(musElementFactory()->noteAccs());
				v->repaint();
			} else if (mode()==EditMode) {
				if (!v->selection().isEmpty()) {
					CAMusElement *elt = v->selection().front()->musElement();
					if (elt->musElementType()==CAMusElement::Note) {
						CACanorus::undo()->createUndoCommand( document(), QObject::tr("add sharp", "undo") );
						((CANote*)elt)->setAccidentals(((CANote*)elt)->accidentals()+1);
						CACanorus::undo()->pushUndoCommand();
						CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
					}
				}
			}
			break;
		}
		
		case Qt::Key_Minus: {
			if (mode()==InsertMode) {
				musElementFactory()->subNoteExtraAccs(1); musElementFactory()->subNoteAccs(1);
				v->setDrawShadowNoteAccs(musElementFactory()->noteExtraAccs()!=0);
				v->setShadowNoteAccs(musElementFactory()->noteAccs());
				v->repaint();
			} else if (mode()==EditMode) {
				if (!v->selection().isEmpty()) {
					CAMusElement *elt = v->selection().front()->musElement();
					if (elt->musElementType()==CAMusElement::Note) {
						CACanorus::undo()->createUndoCommand( document(), QObject::tr("add flat", "undo") );
						((CANote*)elt)->setAccidentals(((CANote*)elt)->accidentals()-1);
						CACanorus::undo()->pushUndoCommand();
						CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
					}
				}
			}
			break;
		}
		
		case Qt::Key_Period: {
			if (mode()==InsertMode) {
				musElementFactory()->addPlayableDotted( 1 );
				currentScoreViewPort()->setShadowNoteDotted( musElementFactory()->playableDotted() );
				v->repaint();
			} else if (mode()==EditMode) {
				if (!((CAScoreViewPort*)v)->selection().isEmpty()) {
					CACanorus::undo()->createUndoCommand( document(), QObject::tr("set dotted", "undo") );
					CAMusElement *elt = ((CAScoreViewPort*)v)->selection().front()->musElement();
					
					if (elt->isPlayable()) {
						int diff;
						if (elt->musElementType()==CAMusElement::Note) {
							int i;
							for (i=0; i<((CANote*)elt)->getChord().size(); i++) {
								diff = ((CANote*)elt)->getChord().at(i)->setDotted((((CAPlayable*)elt)->dotted()+1)%4);
							}
							elt = ((CANote*)elt)->getChord().last();
						} else if (elt->musElementType()==CAMusElement::Rest)
							diff = ((CAPlayable*)elt)->setDotted((((CAPlayable*)elt)->dotted()+1)%4);
						 
						static_cast<CAPlayable*>(elt)->voice()->staff()->synchronizeVoices();
						CACanorus::undo()->pushUndoCommand();
						CACanorus::rebuildUI(document(), ((CANote*)elt)->voice()->staff()->sheet());
					}
				}
			}
			break;
		}
		
		case Qt::Key_Delete:
		case Qt::Key_Backspace:
			deleteSelection( v, e->modifiers()==Qt::ShiftModifier, true );
			break;
		
		// Mode keys
		case Qt::Key_Escape:
			if (mode()==SelectMode) {
				v->clearSelection();
				v->setCurrentContext( 0 );
			}
			_iVoiceNum = 0;
			break;
	}
}

/*!
	This method places the currently prepared music element in CAMusElementFactory to the staff or
	voice, dependent on the music element type and the viewport coordinates.
*/
void CAEditorCtl::insertMusElementAt(const QPoint coords, CAScoreViewPort *v) {
	CADrawableContext *drawableContext = v->currentContext();
	
	CAStaff *staff=0;
	CADrawableStaff *drawableStaff = 0;
	if (drawableContext) {
		drawableStaff = dynamic_cast<CADrawableStaff*>(drawableContext);
		staff = dynamic_cast<CAStaff*>(drawableContext->context());
	}
	
	CADrawableMusElement *drawableRight = v->nearestRightElement(coords.x(), coords.y());
	
	CAMusElement *right=0;
	if ( drawableRight )
		right = drawableRight->musElement();
	
	bool success=false;
	
	if (!drawableContext)
		return;
	
	CACanorus::undo()->createUndoCommand( document(), QObject::tr("insertion of music element", "undo") );
	
	switch ( musElementFactory()->musElementType() ) {
		case CAMusElement::Clef: {
			if (staff)
				success = musElementFactory()->configureClef( staff , right );
			break;
		}
		case CAMusElement::KeySignature: {
			if ( staff )
				success = musElementFactory()->configureKeySignature( staff, right );
			break;
		}
		case CAMusElement::TimeSignature: {
			if ( staff )
				success = musElementFactory()->configureTimeSignature( staff, right );
			break;
		}
		case CAMusElement::Barline: {
			if ( staff )
				success = musElementFactory()->configureBarline( staff, right );
			break;
		}
		case CAMusElement::Note: { // Do we really need to do all that here??
			CAVoice *voice = currentVoice();
			
			if ( !voice )
				break;
			
			CADrawableMusElement *left = v->nearestLeftElement( coords.x(), coords.y(), voice ); // use nearestLeft search because it searches left borders
			CADrawableMusElement *dright = v->nearestRightElement( coords.x(), coords.y(), voice );
			
			if ( left && left->musElement() && left->musElement()->musElementType() == CAMusElement::Note &&
			     left->xPos() <= coords.x() && (left->width() + left->xPos() >= coords.x()) ) {
				// user clicked inside x borders of the note - add a note to the chord
				if ( voice->containsPitch( drawableStaff->calculatePitch(coords.x(), coords.y()), left->musElement()->timeStart() ) )
					break;	//user clicked on an already placed note or wanted to place illegal length (not the one the chord is of) - return and do nothing
				
				success = musElementFactory()->configureNote( drawableStaff->calculatePitch(coords.x(), coords.y()), voice, left->musElement(), true );
			} else {
				success = musElementFactory()->configureNote( drawableStaff->calculatePitch(coords.x(), coords.y()), voice, dright?dright->musElement():0, false );
			}
			
			if ( success ) {
				musElementFactory()->setNoteExtraAccs( 0 );
				v->setDrawShadowNoteAccs( false ); 
				v->setShadowNoteDotted( musElementFactory()->playableDotted() );
			}
			break;
		}
		case CAMusElement::Rest: {
			int iVoiceNum = _iVoiceNum-1<0?0:_iVoiceNum-1;
			CAVoice *voice = 0;
			if (staff)
				voice = staff->voiceAt( iVoiceNum );
			if (voice)
				success = musElementFactory()->configureRest( voice, right );
			if (success)
				v->setShadowNoteDotted( musElementFactory()->playableDotted() );
			break;
		}
		case CAMusElement::Slur: {
			// Insert tie, slur or phrasing slur
			if ( v->selection().size() ) { // start note has to always be selected
				CANote *noteStart = (currentScoreViewPort()->selection().front()->musElement()?dynamic_cast<CANote*>(currentScoreViewPort()->selection().front()->musElement()):0);
				CANote *noteEnd = (currentScoreViewPort()->selection().back()->musElement()?dynamic_cast<CANote*>(currentScoreViewPort()->selection().back()->musElement()):0);
				
				// Insert Tie
				if ( noteStart && musElementFactory()->slurType()==CASlur::TieType ) {
					noteEnd = 0; // find a fresh next note
					QList<CANote*> noteList = noteStart->voice()->getNoteList();
					
					if ( noteStart->tieStart() ) {
						break; // return, if the tie already exists
					} else {
						// create a new tie
						for (int i=0; i<noteList.count() && noteList[i]->timeStart()<=noteStart->timeEnd(); i++) {
							if ( noteList[i]->timeStart()==noteStart->timeEnd() && noteList[i]->pitch()==noteStart->pitch() ) {
								noteEnd = noteList[i];
								break;
							}
						}
					}
					success = musElementFactory()->configureSlur( staff, noteStart, noteEnd );
				} else
				// Insert slur or phrasing slur
				if ( noteStart && noteEnd && noteStart != noteEnd && (musElementFactory()->slurType()==CASlur::SlurType || musElementFactory()->slurType()==CASlur::PhrasingSlurType) ) {
					if (noteStart->isPartOfTheChord()) noteStart = noteStart->getChord().at(0);
					if (noteEnd->isPartOfTheChord()) noteEnd = noteEnd->getChord().at(0);
					QList<CANote*> noteList = noteStart->voice()->getNoteList();
					int end = noteList.indexOf(noteEnd);
					for (int i=noteList.indexOf(noteStart); i<=end; i++)
						if ( musElementFactory()->slurType()==CASlur::SlurType && (noteList[i]->slurStart() || noteList[i]->slurEnd()) ||
						     musElementFactory()->slurType()==CASlur::PhrasingSlurType && (noteList[i]->phrasingSlurStart() || noteList[i]->phrasingSlurEnd()) )
							return;
					
					if (musElementFactory()->slurType()==CASlur::SlurType && (noteStart->slurStart() || noteEnd->slurEnd()) ||
					    musElementFactory()->slurType()==CASlur::PhrasingSlurType && (noteStart->phrasingSlurStart() || noteEnd->phrasingSlurEnd()))
						break; // return, if the slur already exist
					success = musElementFactory()->configureSlur( staff, noteStart, noteEnd );
				}
			}
			break;
		}
		case CAMusElement::FunctionMarking: {
			// Insert function marking
			if (drawableContext->context()->contextType()==CAContext::FunctionMarkingContext) {
				CAFunctionMarkingContext *fmc = static_cast<CAFunctionMarkingContext*>(drawableContext->context());
				CADrawableMusElement *dLeft = v->nearestLeftElement(coords.x(), coords.y(), false);
				int timeStart = 0;
				if ( dLeft ) // find the nearest left element from the cursor
					timeStart = dLeft->musElement()->timeStart();
				QList<CAPlayable*> chord = currentSheet()->getChord(timeStart);
				int timeLength = chord.size()?chord[0]->timeLength():256;
				for (int i=0; i<chord.size(); i++) // find the shortest note in the chord
					if (chord[i]->timeLength()-(timeStart-chord[i]->timeStart())<timeLength)
						timeLength = chord[i]->timeLength()-(timeStart-chord[i]->timeStart());
				
				success = musElementFactory()->configureFunctionMarking( fmc, timeStart, timeLength );
			}
			break;
		}
	}
	
	if (success) {
		if (staff)
			staff->synchronizeVoices();
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI(document(), v->sheet());
		v->selectMElement( musElementFactory()->musElement() );
		musElementFactory()->cloneMusElem(); // Clones the current musElement so it doesn't conflict with the added musElement
	}
}

/*!
	Called when playback is finished or interrupted by the user.
	It stops the playback, closes ports etc.
*/
void CAEditorCtl::playbackFinished() {
	_playback->disconnect();
	//delete _playback;	/// \todo crashes on application close, if deleted! Is this ok? -Matevz
	uiPlayFromSelection->setChecked(false);
	static_cast<CAScoreViewPort*>(_playbackViewPort)->setPlaying(false);

	_repaintTimer->stop();
	_repaintTimer->disconnect();	/// \todo crashes, if disconnected sometimes. -Matevz
	delete _repaintTimer;			/// \todo crashes, if deleted. -Matevz
	CACanorus::midiDevice()->closeOutputPort();
	
	static_cast<CAScoreViewPort*>(_playbackViewPort)->clearSelection();
	static_cast<CAScoreViewPort*>(_playbackViewPort)->addToSelection( _prePlaybackSelection );
	static_cast<CAScoreViewPort*>(_playbackViewPort)->unsetBorder();
	_prePlaybackSelection.clear();

	setMode( mode() );
}

/*!
	Connected with the play button which starts the playback.
*/
void CAEditorCtl::playFromSelection_toggled(bool checked) {
	if (checked && currentScoreViewPort()) {
		CACanorus::midiDevice()->openOutputPort( CACanorus::settings()->midiOutPort() );
		_repaintTimer = new QTimer();
		_repaintTimer->setInterval(100);
		_repaintTimer->start();
		//connect(_repaintTimer, SIGNAL(timeout()), this, SLOT(on_repaintTimer_timeout())); //TODO: timeout is connected directly to repaint() directly. This should be optimized in the future -Matevz
		connect( _repaintTimer, SIGNAL(timeout()), this, SLOT( onRepaintTimerTimeout() ) );
		
		_playback = new CAPlayback(currentScoreViewPort(), CACanorus::midiDevice());
		if ( currentScoreViewPort()->selection().size() && currentScoreViewPort()->selection().at(0)->musElement() )
			_playback->setInitTimeStart( currentScoreViewPort()->selection().at(0)->musElement()->timeStart() );
		
		connect(_playback, SIGNAL(finished()), this, SLOT(playbackFinished()));
		
		QPen p;
		p.setColor(Qt::green);
		p.setWidth(3);
		
		_playbackViewPort = currentViewPort();
		currentScoreViewPort()->setBorder(p);
		currentScoreViewPort()->setPlaying(true);	// set the deadlock for borders
		
		// Remember old selection
		_prePlaybackSelection = currentScoreViewPort()->selection();
		currentScoreViewPort()->clearSelection();
		
		_playback->start();
	} else {
		_playback->stop();
	}
}

/*!
	Links the newly created viewport with the main window:
		- Adds the viewport to the viewport list
		- Sets the icon, focus policy and sets the focus.
		- Sets the currentViewPort but not currentViewPortContainer
*/
void CAEditorCtl::initViewPort(CAViewPort *v) {
	_viewPortList << v;
	QList<QString> paths = CACanorus::locateResource("images/clogosm.png");
	if ( !paths.size() )
		paths = CACanorus::locateResource("ui/images/clogosm.png");
	
	if ( paths.size() )
		v->setWindowIcon(QIcon( paths[0] ));

	v->setFocusPolicy(Qt::ClickFocus);
	v->setFocus();
	setCurrentViewPort(v);
	setMode(mode());	// updates the new viewport border settings	
}

/*!
	Opens a document with the given absolute file name.
	The previous document will be lost.
	
	Returns a pointer to the opened document or null if opening the document has failed.
*/
CADocument *CAEditorCtl::openDocument(const QString& fileName) {
	CACanorusMLImport open;
	open.setStreamFromFile( fileName );
	open.importDocument();
	while ( open.isRunning() );
	
	if( open.importedDocument() ) {
		open.importedDocument()->setFileName(fileName);
		return openDocument( open.importedDocument() );
	} else
		return 0;
}

/*!
	Saves the current document to a given absolute \a fileName.
	This function is usually called when the filename was entered and save document dialog
	successfully closed.
	If \a recovery is False (default), it changes the timeEdit, fileName and some other document
	properties before saving it.
	
	Returns True, if the save was complete; False otherwise.
*/
bool CAEditorCtl::saveDocument( QString fileName ) {
	document()->setTimeEdited( document()->timeEdited() + _timeEditedTime );
	document()->setDateLastModified( QDateTime::currentDateTime() );
	CACanorus::restartTimeEditedTimes( document() );
	
	CACanorusMLExport save;
	save.setStreamToFile( fileName );
	save.exportDocument( document() );
	while ( save.isRunning() );
	
	if ( save.exportedDocument() ) {
		document()->setFileName( fileName );
	}
	
	return save.exportedDocument();
}

/*!
	Called when a user changes the current voice number.
*/
void CAEditorCtl::voiceNum_valChanged(int voiceNr) {
	if ( currentScoreViewPort() ) {
		if ( voiceNr &&
		     currentScoreViewPort()->currentContext() &&
		     currentScoreViewPort()->currentContext()->context()->contextType() == CAContext::Staff
		   )
			currentScoreViewPort()->setSelectedVoice( static_cast<CAStaff*>(currentScoreViewPort()->currentContext()->context())->voiceAt(voiceNr-1) );
		else
			currentScoreViewPort()->setSelectedVoice(0);
		
		currentScoreViewPort()->repaint();
	}
}

/*!
	Changes the number of accidentals.
*/
void CAEditorCtl::keySig_activated( int row ) {
	signed char accs = qRound((row-14.5) / 2);
	CAKeySignature::CAMajorMinorGender gender = (row%2)==0 ? CAKeySignature::Major : CAKeySignature::Minor;
	
	if (mode()==InsertMode) {
		musElementFactory()->setKeySigNumberOfAccs( accs );
		musElementFactory()->setKeySigGender( gender );
	} else
	if ( mode()==EditMode && currentScoreViewPort() && currentScoreViewPort()->selection().size() ) {
		QList<CADrawableMusElement*> list = currentScoreViewPort()->selection();
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change key signature", "undo") );
		
		for ( int i=0; i<list.size(); i++ ) {
			CAKeySignature *keySig = dynamic_cast<CAKeySignature*>(list[i]->musElement());
			CAFunctionMarking *fm = dynamic_cast<CAFunctionMarking*>(list[i]->musElement());
			
			if ( keySig ) {
				keySig->setKeySignatureType( CAKeySignature::MajorMinor, accs, gender );
			}
			
			if ( fm ) {
				fm->setKey( CAKeySignature::keySignatureToString( accs, gender ) );
			}
		}
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI(document(), currentSheet());
	}
}

/*!
	Changes the offset of the clef.
*/
void CAEditorCtl::clefOffset_valueChanged( int newOffset ) {
	if ( oldUiClefOffsetValue==0 && qAbs(newOffset)==1 ) {
		uiClefOffset->setValue( (newOffset/qAbs(newOffset))*2 );
		return;
	} else
	if ( qAbs(oldUiClefOffsetValue)==2 && qAbs(newOffset)==1 ) {
		uiClefOffset->setValue( 0 );
		return;
	}
	
	oldUiClefOffsetValue=newOffset;
	if (mode()==InsertMode) {
		musElementFactory()->setClefOffset( newOffset );
	} else if ( mode()==EditMode ) {
		CAScoreViewPort *v = currentScoreViewPort();
		if ( v && v->selection().size() ) {
			CACanorus::undo()->createUndoCommand( document(), QObject::tr("change clef offset", "undo") );		
			CAClef *clef = dynamic_cast<CAClef*>(v->selection().at(0)->musElement());
			
			if ( clef ) {
				clef->setOffset( CAClef::offsetFromReadable(newOffset) );
				CACanorus::undo()->pushUndoCommand();
				CACanorus::rebuildUI(document(), currentSheet());
			}
		}
	}
}

/*!
	Gets the current voice and sets its name.
*/
void CAEditorCtl::voiceName_returnPressed() {
	CAVoice *voice = currentVoice();
	if (voice) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change voice name", "undo") );
		CACanorus::undo()->pushUndoCommand();
		voice->setName(uiVoiceName->text());
	}
}

void CAEditorCtl::insertPlayable_toggled(bool checked) {
	if (checked) {
		if (!_iVoiceNum)
			_iVoiceNum = 1; // select the first voice if none selected
		
		musElementFactory()->setMusElementType( CAMusElement::Note );
		setMode( InsertMode );
	}
}

/*!
	Function called when user types the text of the syllable or hits control keys.
	
	The following behaviour possible:
		- alphanumeric keys are pressed - writes text
		- spacebar is pressed - creates the current syllable and jumps to the next syllable
		- return is pressed - creates the current syllable and hides the syllable edit widget
		- left key is pressed - if cursorPosition()==0, jumps to the previous syllable
		- right key is pressed - if cursorPosition()==length(), same as spacebar
		- escape key is pressed - hides the syllable edit and cancels any changes to syllable
	
*/
void CAEditorCtl::onSyllableEditKeyPressEvent(QKeyEvent *e, CASyllableEdit *syllableEdit) {
	if (!currentScoreViewPort()) return;
	
	CAScoreViewPort *v = currentScoreViewPort();
	CADrawableContext *dContext = v->currentContext();
	if (!dContext || !dContext->context() || dContext->context()->contextType()!=CAContext::LyricsContext)
		return;
	
	CASyllable *syllable=0;
	if ( v->selection().size() && v->selection().front()->drawableMusElementType()==CADrawableMusElement::DrawableSyllable )
		syllable = static_cast<CASyllable*>(v->selection().front()->musElement());
	
	if (!syllable) v->removeSyllableEdit();
	
	QString text = syllableEdit->text().simplified(); // remove any trailing whitespaces
		
	bool hyphen = false;
	if (text.right(1)=="-") { hyphen = true; text.chop(1); }
		
	bool melisma = false;
	if (text.right(1)=="_") { melisma = true; text.chop(1); }
		
	CAVoice *voice = 0; /// \todo GUI for syllable specific associated voice - current is the default lyrics context's one
		
	CALyricsContext *context = static_cast<CALyricsContext*>(dContext->context());
	
	// create or edit syllable
	if ( e->key()==Qt::Key_Space  ||
	     e->key()==Qt::Key_Return ||
	     e->key()==Qt::Key_Right && syllableEdit->cursorPosition()==syllableEdit->text().size() ||
	     (e->key()==Qt::Key_Left || e->key()==Qt::Key_Backspace) && syllableEdit->cursorPosition()==0 || 
	     CACanorus::settings()->finaleLyricsBehaviour() && e->key()==Qt::Key_Minus
	   ) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("lyrics edit", "undo") );	     
		syllable->setText(text);
		syllable->setHyphenStart(hyphen);
		syllable->setMelismaStart(melisma);
		
		v->removeSyllableEdit();
		
		CAVoice *voice = (syllable->associatedVoice()?syllable->associatedVoice():context->associatedVoice());
		CAMusElement *nextSyllable = 0;
		if (syllable) {
			if (e->key()==Qt::Key_Space || e->key()==Qt::Key_Right || e->key()==Qt::Key_Return) { // next right note
				nextSyllable = syllable->lyricsContext()->next(syllable);
			} else  if (e->key()==Qt::Key_Left || e->key()==Qt::Key_Backspace) {                  // next left note
				nextSyllable = syllable->lyricsContext()->previous(syllable);
			} else if (e->key()==Qt::Key_Minus) {
				syllable->setHyphenStart(true);
				nextSyllable = syllable->lyricsContext()->next(syllable);
			}
			
			CACanorus::undo()->pushUndoCommand();
			CACanorus::rebuildUI( document(), currentSheet() );
			if (nextSyllable) {
				CADrawableMusElement *dNextSyllable = v->selectMElement(nextSyllable);
				v->createSyllableEdit( dNextSyllable );
				if ( e->key()==Qt::Key_Space || e->key()==Qt::Key_Right || e->key()==Qt::Key_Return )
					v->syllableEdit()->setCursorPosition(0); // go to the beginning if moving to the right next syllable
			}
		}
	}
	
	// escape key - cancel
	if (e->key()==Qt::Key_Escape) {
		v->removeSyllableEdit();
	}
}

void CAEditorCtl::FMFunction_toggled( bool checked, int buttonId ) {
	if ( mode()==InsertMode ) {
		musElementFactory()->setFMFunction( static_cast<CAFunctionMarking::CAFunctionType>( buttonId * (buttonId<0?-1:1) ));
		musElementFactory()->setFMFunctionMinor( buttonId<0 );
	} else
	if ( mode()==EditMode && currentScoreViewPort() && currentScoreViewPort()->selection().size()) {
		CAScoreViewPort *v = currentScoreViewPort();
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change function", "undo") );
		
		for ( int i=0; i<v->selection().size(); i++ ) {
			CAFunctionMarking *fm = dynamic_cast<CAFunctionMarking*>( v->selection().at(i)->musElement() );
			
			if ( fm ) {
				fm->setFunction( static_cast<CAFunctionMarking::CAFunctionType>( buttonId * (buttonId<0?-1:1) ));
				fm->setMinor( buttonId<0 );
			}
		}
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI( document(), currentSheet() );
	}
}

void CAEditorCtl::FMChordArea_toggled(bool checked, int buttonId) {
	if ( mode()==InsertMode ) {
		musElementFactory()->setFMChordArea( static_cast<CAFunctionMarking::CAFunctionType>( buttonId * (buttonId<0?-1:1) ));
		musElementFactory()->setFMChordAreaMinor( buttonId<0 );
	} else
	if ( mode()==EditMode && currentScoreViewPort() && currentScoreViewPort()->selection().size()) {
		CAScoreViewPort *v = currentScoreViewPort();
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change chord area", "undo") );
		
		for ( int i=0; i<v->selection().size(); i++ ) {
			CAFunctionMarking *fm = dynamic_cast<CAFunctionMarking*>( v->selection().at(i)->musElement() );
			
			if ( fm ) {
				fm->setChordArea( static_cast<CAFunctionMarking::CAFunctionType>( buttonId * (buttonId<0?-1:1) ));
				fm->setChordAreaMinor( buttonId<0 );
			}
		}
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI( document(), currentSheet() );
	}
}

void CAEditorCtl::FMTonicDegree_toggled(bool checked, int buttonId) {
	if ( mode()==InsertMode ) {
		musElementFactory()->setFMTonicDegree( static_cast<CAFunctionMarking::CAFunctionType>( buttonId * (buttonId<0?-1:1) ));
		musElementFactory()->setFMTonicDegreeMinor( buttonId<0 );
	} else
	if ( mode()==EditMode && currentScoreViewPort() && currentScoreViewPort()->selection().size()) {
		CAScoreViewPort *v = currentScoreViewPort();
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change tonic degree", "undo") );
		
		for ( int i=0; i<v->selection().size(); i++ ) {
			CAFunctionMarking *fm = dynamic_cast<CAFunctionMarking*>( v->selection().at(i)->musElement() );
			
			if ( fm ) {
				fm->setTonicDegree( static_cast<CAFunctionMarking::CAFunctionType>( buttonId * (buttonId<0?-1:1) ));
				fm->setTonicDegreeMinor( buttonId<0 );
			}
		}
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI( document(), currentSheet() );
	}
}

void CAEditorCtl::FMEllipse_toggled( bool checked ) {
	if ( mode()==InsertMode ) {
		musElementFactory()->setFMEllipse( checked );
	} else
	if ( mode()==EditMode && currentScoreViewPort() && currentScoreViewPort()->selection().size()) {
		CAScoreViewPort *v = currentScoreViewPort();
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("set/unset ellipse", "undo") );
		
		for ( int i=0; i<v->selection().size(); i++ ) {
			CAFunctionMarking *fm = dynamic_cast<CAFunctionMarking*>( v->selection().at(i)->musElement() );
			
			if ( fm ) {
				fm->setEllipse( checked );
			}
		}
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI( document(), currentSheet() );
	}
}

void CAEditorCtl::slurType_toggled( bool checked, int buttonId ) {
	// remember previous muselement type so we can return to previous state after
	CAMusElement::CAMusElementType prevMusEltType =
		musElementFactory()->musElementType();
	
	// Read currently selected entry from tool button menu
	CASlur::CASlurType slurType =
		static_cast<CASlur::CASlurType>(buttonId);
	
	musElementFactory()->setMusElementType( CAMusElement::Slur );
	
	// New clef type
	musElementFactory()->setSlurType( slurType );
	
	insertMusElementAt( QPoint(0,0), currentScoreViewPort() ); // inserts a slur or tie and quits the insert mode
	
	musElementFactory()->setMusElementType( prevMusEltType );
}

void CAEditorCtl::clefType_toggled(bool checked, int buttonId) {
	if ( checked ) {
		// Read currently selected entry from tool button menu
		CAClef::CAPredefinedClefType clefType =
			static_cast<CAClef::CAPredefinedClefType>(buttonId);
			
		musElementFactory()->setMusElementType( CAMusElement::Clef );
		
		// New clef type
		musElementFactory()->setClef( clefType );
		
		setMode( InsertMode );
	}
}

/*!
	Called when a user clicks "Commit" button in source viewport.
*/
void CAEditorCtl::sourceViewPortCommit(QString inputString, CASourceViewPort *v) {
	if (v->document()) {
		// CanorusML document source
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("commit CanorusML source", "undo") );
		
		clearUI(); 	// clear GUI before clearing the data part!
		if ( document() )
			delete document();
		
		CACanorus::undo()->pushUndoCommand();
		CACanorusMLImport open( inputString );
		open.importDocument();
		while ( open.isRunning() );
		
		if (open.importedDocument()) {
			CACanorus::undo()->changeDocument( document(), open.importedDocument() );
			setDocument( open.importedDocument() );
		}
		CACanorus::rebuildUI(document());
	} else
	if (v->voice()) {
		// LilyPond voice source
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("commit LilyPond source", "undo") );
		
		CALilyPondImport li( inputString );
		
		CAVoice *oldVoice = v->voice();
		
		QList<CAMusElement*> signList = oldVoice->getSignList();
		for (int i=0; i<signList.size(); i++)
			oldVoice->remove( signList[i] );  // removes signs from all voices
		
		li.setTemplateVoice( oldVoice );      // copy properties
		li.importVoice();
		while (li.isRunning());
		
		CAVoice *newVoice = li.importedVoice();
		delete oldVoice; // also removes voice from the staff
		
		newVoice->staff()->addVoice( newVoice );
		newVoice->staff()->synchronizeVoices();
		
		v->setVoice( newVoice );
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI(document(), newVoice->staff()->sheet());
	} else
	if (v->lyricsContext()) {
		// LilyPond lyrics source
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("commit LilyPond source", "undo") );
		
		CALilyPondImport li( inputString );
		
		li.importLyricsContext();
		while (li.isRunning());
		CALyricsContext *newLc = li.importedLyricsContext();
		CALyricsContext *oldLc = v->lyricsContext();
		newLc->cloneLyricsContextProperties( oldLc );
		if (newLc->associatedVoice()) {
			newLc->associatedVoice()->removeLyricsContext( oldLc );
			newLc->associatedVoice()->addLyricsContext(newLc);
		}
		newLc->sheet()->insertContextAfter( oldLc, newLc );
		newLc->sheet()->removeContext( oldLc );
		v->setLyricsContext( newLc );
		delete oldLc;
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI(document(), v->lyricsContext()->sheet());
	}
	
	setCurrentViewPort( v );
}

void CAEditorCtl::LilyPondSource_triggered() {
	CAContext *context = currentContext();
	if ( !context ) 
		return;
	
	CASourceViewPort *v=0;
	CAStaff *staff = 0;
	if (staff = currentStaff()) {
		int voiceNum = _iVoiceNum-1<0?0:_iVoiceNum-1;
		CAVoice *voice = staff->voiceAt( voiceNum );
		v = new CASourceViewPort(voice, 0);
	} else
	if (context->contextType()==CAContext::LyricsContext) {
		v = new CASourceViewPort(static_cast<CALyricsContext*>(context), 0);
	}
	
	initViewPort( v );
	currentViewPortContainer()->addViewPort( v );
}

/*!
	Adds a new score viewport to default viewport container.
*/
bool CAEditorCtl::scoreView_triggered() {
	CASheet* s = _sheetMap[currentViewPortContainer()];
	
	if ( currentViewPortContainer() && s ) {
		CAScoreViewPort *v = new CAScoreViewPort(s, 0);
		initViewPort( v );
		currentViewPortContainer()->addViewPort( v );
		v->rebuild();
		return true;
	}
	return false;
}

/*!
	Removes the sheet, all its contents and rebuilds the GUI.
*/
void CAEditorCtl::removeSheet_triggered() {
	CASheet *sheet = currentSheet();
	if (sheet) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("deletion of the sheet", "undo") );
		CACanorus::undo()->pushUndoCommand();
		document()->removeSheet(currentSheet());
		removeSheet(sheet);
		delete sheet;
	}
}

/*!
	Removes the sheet from the GUI and deletes the viewports and viewport containers and tabs
	pointing to the given \a sheet.
*/
CAViewPortContainer *CAEditorCtl::removeSheet( CASheet *sheet ) {
	CAViewPortContainer *vpc = _sheetMap.key(sheet);
	_sheetMap.remove(vpc);
	
	setCurrentViewPortContainer( static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget()) );
	setCurrentViewPort( static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget())?static_cast<CAViewPortContainer*>(uiTabWidget->currentWidget())->currentViewPort():0 );
	
	// remove other viewports pointing to the sheet
	QList<CAViewPort*> vpl = viewPortList();
	for (int i=0; i<vpl.size(); i++) {
		switch (vpl[i]->viewPortType()) {
			case CAViewPort::ScoreViewPort: {
				if (static_cast<CAScoreViewPort*>(vpl[i])->sheet()==sheet)
					delete vpl[i];
				break;
			}
			case CAViewPort::SourceViewPort: {
				CASourceViewPort *sv = static_cast<CASourceViewPort*>(vpl[i]);
				if (sv->voice() && sv->voice()->staff()->sheet()==sheet)
					delete vpl[i];
				else
				if (sv->lyricsContext() && sv->lyricsContext()->sheet()==sheet)
					delete vpl[i];
				
				break;
			}
		}
	}
	return vpc;
}

CASheet *CAEditorCtl::sheetName_returnPressed() {
	CASheet *sheet = currentSheet();
	if (sheet) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change sheet name", "undo") );
		CACanorus::undo()->pushUndoCommand();
		updateUndoRedoButtons();
		sheet->setName( uiSheetName->text() );
	}
	return sheet;
}

/*!
	Sets the current context name.
*/
void CAEditorCtl::contextName_returnPressed() {
	CAContext *context = currentContext();
	if (context) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change context name", "undo") );
		CACanorus::undo()->pushUndoCommand();
		updateUndoRedoButtons();
		context->setName(uiContextName->text());
	}
}

/*!
	Sets the stanza number of the current lyrics context.
*/
void CAEditorCtl::stanzaNumber_valueChanged(int stanzaNumber) {
	if (currentContext() && currentContext()->contextType()==CAContext::LyricsContext) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change stanza number", "undo") );
		if (static_cast<CALyricsContext*>(currentContext())->stanzaNumber()!=stanzaNumber)
			CACanorus::undo()->pushUndoCommand();
		static_cast<CALyricsContext*>(currentContext())->setStanzaNumber( stanzaNumber );
	}
}

/*!
	Sets the associated voice of the current lyrics context.
*/
void CAEditorCtl::associatedVoice_activated(int idx) {
	if (idx != -1 && currentContext() && currentContext()->contextType()==CAContext::LyricsContext) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change associated voice", "undo") );
		if (static_cast<CALyricsContext*>(currentContext())->associatedVoice()!=currentSheet()->voiceList().at( idx ))
			CACanorus::undo()->pushUndoCommand();
		static_cast<CALyricsContext*>(currentContext())->setAssociatedVoice( currentSheet()->voiceList().at( idx ) );
		CACanorus::rebuildUI( document(), currentSheet() ); // needs a rebuild if lyrics contexts are to be moved
	}
}

void CAEditorCtl::voiceStemDirection_toggled(bool checked, int direction) {
	CAVoice *voice = currentVoice();
	if (voice) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change voice stem direction", "undo") );
		if (voice->stemDirection()!=static_cast<CANote::CAStemDirection>(direction))
			CACanorus::undo()->pushUndoCommand();
		CACanorus::undo()->pushUndoCommand();
		voice->setStemDirection(static_cast<CANote::CAStemDirection>(direction));
		CACanorus::rebuildUI(document(), currentSheet());
	}
}

/*!
	Sets the currently selected note stem direction if in insert/edit mode or the music elements factory note stem direction if in insert mode.
*/
void CAEditorCtl::noteStemDirection_toggled(bool checked, int id) {
	CANote::CAStemDirection direction = static_cast<CANote::CAStemDirection>(id);
	if (mode()==InsertMode)
		musElementFactory()->setNoteStemDirection( direction );
	else if (mode()==SelectMode || mode()==EditMode) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("change note stem direction", "undo") );
		CAScoreViewPort *v = currentScoreViewPort();
		bool changed=false;
		for (int i=0; v && i<v->selection().size(); i++) {
			CANote *note = dynamic_cast<CANote*>(v->selection().at(i)->musElement());
			if ( note ) {
				note->setStemDirection( direction );
				changed=true;
			}
		}
		if (changed) {
			CACanorus::undo()->pushUndoCommand();
			CACanorus::rebuildUI(document(), currentSheet());
		}
	}
}

/*!
	Action on Edit->Cut.
*/
void CAEditorCtl::cut_triggered() {
	if ( currentScoreViewPort() ) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("cut", "undo") );
		copySelection( currentScoreViewPort() );
		deleteSelection( currentScoreViewPort(), false, false ); // and don't make undo as we already make it
		CACanorus::undo()->pushUndoCommand();
	}	
}

/*!
	Backend for Edit->Copy.
*/
void CAEditorCtl::copySelection( CAScoreViewPort *v ) {
	if ( v->selection().size() ) {
		QList<CAMusElement*> list;
		
		for (int i=0; i<v->selection().size(); i++) {
			if (v->selection().at(i)->musElement() && !list.contains( v->selection().at(i)->musElement())) {
				list << v->selection().at(i)->musElement()->clone(); // Add clones to clipboard. They are destroyed in CAMimeData destructor!
			}
		}
		
		QApplication::clipboard()->setMimeData( new CAMimeData(list) );
	}
}

/*!
	Backend for Delete.
*/
void CAEditorCtl::deleteSelection( CAScoreViewPort *v, bool deleteSyllable, bool doUndo ) {
	if ( v->selection().size() ) {
		if (doUndo)
			CACanorus::undo()->createUndoCommand( document(), QObject::tr("deletion of elements", "undo") );
		
		QSet<CAMusElement*> musElemSet;
		for (int i=0; i<v->selection().size(); i++)
			musElemSet << v->selection().at(i)->musElement();
		
		// cleans up the set - removes empty elements and elements which get deleted automatically (eg. slurs, if both notes are deleted)
		for (QSet<CAMusElement*>::iterator i=musElemSet.begin(); i!=musElemSet.end();) {
			if (!(*i))
				i = musElemSet.erase(i);
			else if ((*i)->musElementType()==CAMusElement::Slur && musElemSet.contains(static_cast<CASlur*>(*i)->noteStart()))
				i = musElemSet.erase(i);
			else if ((*i)->musElementType()==CAMusElement::Slur && musElemSet.contains(static_cast<CASlur*>(*i)->noteEnd()))
				i = musElemSet.erase(i);
			else
				i++;
		}
		
		for (QSet<CAMusElement*>::const_iterator i=musElemSet.constBegin(); i!=musElemSet.constEnd(); i++) {
			if ((*i)->musElementType()==CAMusElement::Note) {
				CANote *note = static_cast<CANote*>(*i);
				if (!note->isPartOfTheChord()) {
					for (int j=0; j<note->voice()->lyricsContextList().size(); j++) {
						CASyllable *removedSyllable =
							note->voice()->lyricsContextList().at(j)->removeSyllableAtTimeStart(note->timeStart());
						musElemSet.remove(removedSyllable);
					}
					note->voice()->staff()->synchronizeVoices();
				}
				note->voice()->remove( note );
				delete note;
			} else if ((*i)->musElementType()==CAMusElement::Rest) {
				static_cast<CARest*>(*i)->voice()->staff()->synchronizeVoices();
				delete *i;				
			} else if ((*i)->musElementType()==CAMusElement::Syllable) {
				if ( deleteSyllable ) {
					CALyricsContext *lc = static_cast<CALyricsContext*>((*i)->context()); 
					(*i)->context()->remove(*i);  // actually removes the syllable if SHIFT is pressed
					lc->repositSyllables();
				} else {
					static_cast<CASyllable*>(*i)->clear(); // only clears syllable's text
				}
			} else if ((*i)->musElementType()==CAMusElement::FunctionMarking) {
				if ( deleteSyllable ) {
					CAFunctionMarkingContext *fmc = static_cast<CAFunctionMarkingContext*>((*i)->context()); 
					(*i)->context()->remove(*i);  // actually removes the function if SHIFT is pressed
					fmc->repositFunctions();
				} else {
					static_cast<CAFunctionMarking*>(*i)->clear(); // only clears the function
				}
			} else {
				(*i)->context()->remove(*i);
			}
		}
		if (doUndo)
			CACanorus::undo()->pushUndoCommand();
		
		v->clearSelection();
		CACanorus::rebuildUI(document(), v->sheet());
	}	
}

/*!
	Backend for Edit->Paste.
*/
void CAEditorCtl::pasteAt( const QPoint coords, CAScoreViewPort *v ) {
	if ( QApplication::clipboard()->mimeData() &&
	     dynamic_cast<const CAMimeData*>(QApplication::clipboard()->mimeData()) &&
	     v->currentContext() &&
	     v->currentContext()->drawableContextType()==CADrawableContext::DrawableStaff ) {
		CACanorus::undo()->createUndoCommand( document(), QObject::tr("paste", "undo") );
		
		CAStaff *staff = static_cast<CAStaff*>( v->currentContext()->context() ); 
		CAVoice *voice = staff->voiceAt( _iVoiceNum ? _iVoiceNum-1 : _iVoiceNum );
		CADrawableMusElement *drawableRight = v->nearestRightElement(coords.x(), coords.y(), voice);
		CAMusElement *right = 0;
		if (drawableRight)
			right = drawableRight->musElement();
		
		QList<CAMusElement*> newEltList;
		QList<CAMusElement*> eltList = static_cast<const CAMimeData*>(QApplication::clipboard()->mimeData())->musElements();
		for ( int i=0; i<eltList.size(); i++ ) {
			CAMusElement *newElt;
			if ( eltList[i]->isPlayable() ) {
				newElt = static_cast<CAPlayable*>(eltList[i])->clone(voice);
				if ( eltList[i]->musElementType()==CAMusElement::Note &&
				     static_cast<CANote*>(eltList[i])->isPartOfTheChord() &&
				    !static_cast<CANote*>(eltList[i])->isFirstInTheChord() ) {
					voice->insert( right, newElt, true );
				} else {
					voice->insert( right, newElt, false );
				}
			} else {
				newElt = eltList[i]->clone();
				newElt->setContext( staff );
				newElt->setTimeStart( right?right->timeEnd():0);
				voice->insert( right, newElt );
			}
			newEltList << newElt;
		}
		
		staff->synchronizeVoices();
		
		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI( document(), currentSheet() );
		
		// select paste elements
		currentScoreViewPort()->clearSelection();
		for (int i=0; i<newEltList.size(); i++)
			currentScoreViewPort()->addToSelection( newEltList[i] );
		currentScoreViewPort()->repaint();
	}
}
