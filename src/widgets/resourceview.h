/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef RESOURCEVIEW_H_
#define RESOURCEVIEW_H_

#include <QMap>
#include <QTreeWidget>

class QWidget;
class CADocument;
class CAResource;

class CAResourceView : public QTreeWidget {
    Q_OBJECT

public:
    CAResourceView(CADocument* doc, QWidget* parent = nullptr);
    ~CAResourceView();

    void rebuildUi();

    void setDocument(CADocument* doc)
    {
        _document = doc;
        rebuildUi();
    }
    CADocument* document() { return _document; }

protected slots:
    void on_itemChanged(QTreeWidgetItem* i, int column);

private:
    void showEvent(QShowEvent*);
    void closeEvent(QCloseEvent*);
    void contextMenuEvent(QContextMenuEvent* e);

    CADocument* _document;
    QMap<QTreeWidgetItem*, CAResource*> _items;
};

#endif /* RESOURCEVIEW_H_ */
