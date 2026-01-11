#ifndef REGISTERTREEVIEWHANDLER_H
#define REGISTERTREEVIEWHANDLER_H

#include <QTreeWidget>
#include <QLineEdit>

class RegisterTreeViewHandler
{
public:
    explicit RegisterTreeViewHandler(QTreeWidget* tree);

    void onTreeItemChanged(QTreeWidgetItem* item, int column);
    void filterTree(const QVector<QLineEdit*>& filters);
    void populateTree(const std::string&, const QString&, const QString&, const QString&, const QString&, const QString&);
    QTreeWidgetItem* getOrCreateItem(QTreeWidgetItem* parent, const QString& name);


    QTreeWidget *tData;
    QMap<QString, QTreeWidgetItem*> nodeCache;
};

#endif // REGISTERTREEVIEWHANDLER_H
