/*!
	Copyright (c) 2008-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/resourceview.h"
#include "canorus.h"
#include "control/resourcectl.h"
#include "score/document.h"
#include "score/resource.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStringList>

/*!
	\class CAResourceView
	\brief Tree view of all the resources inside the document

	This widget shows the resources stored inside the document in a
	tree-view style. Pass the document to the constructor and call
	rebuildUi() to refresh the GUI.

	This widget also allows user to remove or rename the resources.
 */

/*!
	Default constructor.

	Pass the document \a doc. The view automatically gathers the resources
	stored inside the document and shows them.
 */
CAResourceView::CAResourceView(CADocument* doc, QWidget* parent)
    : QTreeWidget(parent)
    , _document(doc)
{
    setColumnCount(2);
    setHeaderLabels(QStringList() << tr("Name") << tr("Linked"));
    setWindowTitle(tr("Document Resources"));

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(on_itemChanged(QTreeWidgetItem*, int)));

    rebuildUi();
}

CAResourceView::~CAResourceView()
{
}

void CAResourceView::rebuildUi()
{
    clear();
    _items.clear();

    QList<CAResource*> sItems;
    for (int i = 0; i < selectedItems().size(); i++) {
        if (_items[selectedItems()[i]]) {
            sItems << _items[selectedItems()[i]];
        }
    }

    if (document()) {
        QTreeWidgetItem* doc = new QTreeWidgetItem(QStringList() << tr("Document") << "");
        doc->setIcon(0, QIcon("images:document/document.svg"));
        addTopLevelItem(doc);

        for (int i = 0; i < document()->resourceList().size(); i++) {
            QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << document()->resourceList()[i]->name() << (document()->resourceList()[i]->isLinked() ? tr("yes") : tr("no")));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
            _items[item] = document()->resourceList()[i];
            doc->addChild(item);
        }
    }

    expandAll();

    for (int i = 0; i < sItems.size(); i++) {
        if (_items.key(sItems[i])) {
            _items.key(sItems[i])->setSelected(true);
        }
    }
}

void CAResourceView::showEvent(QShowEvent*)
{
    QList<CAMainWin*> mainWins = CACanorus::findMainWin(document());

    for (int i = 0; i < mainWins.size(); i++) {
        mainWins[i]->resourceViewAction()->setChecked(true);
    }
}

void CAResourceView::closeEvent(QCloseEvent*)
{
    QList<CAMainWin*> mainWins = CACanorus::findMainWin(document());

    for (int i = 0; i < mainWins.size(); i++) {
        mainWins[i]->resourceViewAction()->setChecked(false);
    }
}

void CAResourceView::contextMenuEvent(QContextMenuEvent* e)
{
    QList<QTreeWidgetItem*> selection = selectedItems();

    if (selection.size() && _items[selection[0]]) {
        CAResource* resource = _items[selection[0]];

        QList<QAction*> actions;
        QAction* rename = new QAction(tr("Rename"), this);
        actions << rename;

        QAction* saveAs = nullptr;
        if (!resource->isLinked()) {
            saveAs = new QAction(tr("Save as..."), this);
            actions << saveAs;
        }

        QAction* remove = new QAction(tr("Remove"), this);
        actions << remove;

        QAction* selectedAction = QMenu::exec(actions, e->globalPos());
        if (selectedAction) {
            if (selectedAction == rename) {
                editItem(selection[0], 0);
            } else if (selectedAction == remove && QMessageBox::question(this, tr("Confirm deletion"), tr("Do you want to remove resource \"%1\"?\n\nDeletion cannot be undone!").arg(resource->name()), QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes) {
                CAResourceCtl::deleteResource(resource);
                CACanorus::rebuildUI(document());
            } else if (selectedAction == saveAs) {
                QString path = QFileDialog::getSaveFileName();
                if (!path.isEmpty()) {
                    resource->copy(path);
                }
            }
        }
    }
}

void CAResourceView::on_itemChanged(QTreeWidgetItem* i, int)
{
    if (_items[i]) {
        _items[i]->setName(i->text(0));
    }

    rebuildUi();
}
