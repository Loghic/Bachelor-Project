#include "include/registertreeviewhandler.h"

RegisterTreeViewHandler::RegisterTreeViewHandler(QTreeWidget* tree)
    : tData(tree)
{}

void RegisterTreeViewHandler::populateTree(const std::string& nodeStr,
                                           const QString& addr,
                                           const QString& dec,
                                           const QString& hex,
                                           const QString& mask,
                                           const QString& permission)
{
    QString fullPath = QString::fromStdString(nodeStr);

    // Lookup from cache
    QTreeWidgetItem* item = nodeCache.value(fullPath, nullptr);

    // If not cached, build path and create
    if (!item) {
        QStringList parts = fullPath.split('.');
        QTreeWidgetItem* parent = tData->invisibleRootItem();

        QString currentPath;
        for (int i = 0; i < parts.size(); ++i) {
            const QString& part = parts[i];
            currentPath += (i > 0 ? "." : "") + part;

            if (!nodeCache.contains(currentPath)) {
                QTreeWidgetItem* newItem = new QTreeWidgetItem();
                newItem->setText(0, fullPath);
                newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                parent->addChild(newItem);
                nodeCache[currentPath] = newItem;
            }

            parent = nodeCache[currentPath];
        }

        item = parent;
        item->setData(0, Qt::UserRole + 1, fullPath);

        // Set static fields only once on creation
        item->setText(1, addr);
        item->setText(4, mask);

        QString perm = (permission == "R" || permission == "Read") ? "Read" : "Write";
        item->setText(5, perm);

        if (perm == "Write") {
            item->setTextAlignment(5, Qt::AlignCenter);
            item->setBackground(5, QColor("#ADD8E6"));
            item->setForeground(5, QColor("#000000"));
            item->setFont(5, QFont("Segoe UI", 9, QFont::Bold));
        }

        // Non-editable
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }

    // Only update dec and hex values (the dynamic parts)
    if (item->text(2) != dec)
        item->setText(2, dec);
    if (item->text(3) != hex)
        item->setText(3, hex);

    // Store user data (for write dialog or tracking changes)
    if (item->data(2, Qt::UserRole).toString() != dec)
        item->setData(2, Qt::UserRole, dec);
    if (item->data(3, Qt::UserRole).toString() != hex)
        item->setData(3, Qt::UserRole, hex);
}


void RegisterTreeViewHandler::filterTree(const QVector<QLineEdit*>& filters)
{
    // Recursive lambda to apply filter to a tree item and its children
    std::function<bool(QTreeWidgetItem*)> filterItem = [&](QTreeWidgetItem* item) {
        bool match = true;

        // Check each column's filter against the item's text
        for (int col = 0; col < filters.size(); ++col) {
            QString filterText = filters[col]->text().trimmed();
            if (!filterText.isEmpty()) {
                // Use case-insensitive regex to match filter
                QRegularExpression regex(filterText, QRegularExpression::CaseInsensitiveOption);
                if (!regex.isValid() || !regex.match(item->text(col)).hasMatch()) {
                    match = false; // Stop checking further if one column doesn't match
                    break;
                }
            }
        }

        // Recursively apply filter to children
        bool hasVisibleChild = false;
        for (int i = 0; i < item->childCount(); ++i) {
            hasVisibleChild |= filterItem(item->child(i));
        }

        // An item is visible if it matches itself or has any visible children
        bool visible = match || hasVisibleChild;
        item->setHidden(!visible);

        // Expand matching parents automatically
        item->setExpanded(visible && item->childCount() > 0);

        return visible;
    };

    // Apply filter to all top-level items
    for (int i = 0; i < tData->topLevelItemCount(); ++i) {
        filterItem(tData->topLevelItem(i));
    }
}

void RegisterTreeViewHandler::onTreeItemChanged(QTreeWidgetItem* item, int column)
{
    // Only respond to edits in the decimal (col 2) or hex (col 3) columns
    if (column != 2 && column != 3) return;

    bool ok = false;
    QString newText = item->text(column);

    // If decimal column changed, update the corresponding hex value
    if (column == 2) {
        uint32_t val = newText.toUInt(&ok, 10); // Convert from base 10
        if (ok) {
            item->setText(3, QString("0x%1").arg(val, 0, 16)); // Update hex column
        }
    }
    // If hex column changed, update the corresponding decimal value
    else if (column == 3) {
        uint32_t val = newText.toUInt(&ok, 16); // Convert from base 16
        if (ok) {
            item->setText(2, QString::number(val));  // Update decimal column
        }
    }
}
