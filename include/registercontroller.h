#ifndef REGISTERCONTROLLER_H
#define REGISTERCONTROLLER_H

#include <exception>
#include <uhal/uhal.hpp>
#include <uhal/ConnectionManager.hpp>
#include <uhal/log/log.hpp>
#include <uhal/log/exception.hpp>

#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QWidget>
#include <QComboBox>

#include "registertreeviewhandler.h"
#include "devicemanager.h"

class RegisterController
{
public:
    RegisterController(QWidget* parentWidget, QTreeWidget* tData);
    ~RegisterController();

    void readNodes(uhal::HwInterface& hw);
    void writeNode(uhal::HwInterface& hw, const QString& reg, const QString& val);
    void performRead(bool resetFilters, bool clearTreeBeforeRead, QVector<QLineEdit*> filterEdits, QComboBox* cbDeviceDropdown, QMap<QString, DeviceInfo>& deviceInfoMap);
    void openWriteDialog(QTreeWidgetItem* item, QMap<QString, DeviceInfo>& deviceInfoMap, QComboBox* cbDeviceDropdown);
private: 
    void ensureCachedHw(const QString& deviceId, const QMap<QString, DeviceInfo>& deviceInfoMap);

    RegisterTreeViewHandler* registerTreeViewHandler;
    QWidget* parentWidget;

    uhal::HwInterface* cachedHw = nullptr;
    QString cachedDeviceId;

    struct NodeMetadata {
        uint32_t address;
        uint32_t mask;
        int permission;
    };

    QMap<std::string, NodeMetadata> nodeMetadataCache;

};

#endif // REGISTERCONTROLLER_H
