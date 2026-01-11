#include "include/registercontroller.h"

#include <QElapsedTimer>

RegisterController::RegisterController(QWidget* parentWidget, QTreeWidget* tData)
    : parentWidget(parentWidget)
{
    registerTreeViewHandler = new RegisterTreeViewHandler(tData);

}

RegisterController::~RegisterController()
{
    delete cachedHw;
    cachedHw = nullptr;
}

void RegisterController::readNodes(uhal::HwInterface& hw)
{
    std::vector<std::string> nodes = hw.getNodes();
    if (nodes.empty()) return;

    // --- Ensure metadata is cached ---
    for (const auto& node : nodes) {
        if (!nodeMetadataCache.contains(node)) {
            try {
                const auto& n = hw.getNode(node);
                nodeMetadataCache[node] = NodeMetadata{
                    .address = n.getAddress(),
                    .mask = n.getMask(),
                    .permission = static_cast<int>(n.getPermission())
                };
            } catch (const std::exception& e) {
                qDebug() << "[Meta] Error:" << e.what();
                continue;
            }
        }
    }

    // --- Batch all reads ---
    std::vector<uhal::ValWord<uint32_t>> values(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
        try {
            values[i] = hw.getNode(nodes[i]).read();
        } catch (const std::exception& e) {
            qDebug() << "[ReadQueue] Error:" << e.what();
        }
    }

    try {
        hw.dispatch();
    } catch (const std::exception& e) {
        qDebug() << "[Dispatch] Error:" << e.what();
        return;
    }

    // --- Update tree with read values ---
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];

        if (!nodeMetadataCache.contains(node)) continue;

        const NodeMetadata& meta = nodeMetadataCache[node];

        const uint32_t value = values[i].value();
        const QString dec_val = QString::number(value);
        const QString hex_val = QString("0x") + QString::number(value, 16).toUpper();
        const QString hex_addr = QString("0x") + QString::number(meta.address, 16).toUpper();
        const QString hex_mask = QString("0x") + QString::number(meta.mask, 16).toUpper();

        QString permission;
        switch (meta.permission) {
        case 1: permission = "R"; break;
        case 2: permission = "W"; break;
        case 3: permission = "R/W"; break;
        default: permission = "?"; break;
        }

        registerTreeViewHandler->populateTree(node, hex_addr, dec_val, hex_val, hex_mask, permission);
    }
}

void RegisterController::writeNode(uhal::HwInterface& hw, const QString& reg, const QString& val)
{
    // Check whether the hardware connection is alive by attempting a dummy read
    try{
        hw.getNode(reg.toStdString()).read();
        hw.dispatch();
    } catch(const uhal::exception::TransportLayerError& e){
        qDebug() << e.what();
        return;
    }

    // Retrieve the permission level of the register
    auto permission = hw.getNode(reg.toStdString()).getPermission();
    hw.dispatch();

    // Check if register is read-only
    if(static_cast<int>(permission) == 1){
        qDebug() << "Chosen register has only READ permission status.";
        return;
    }

    // Attempt to convert hex string input to a 32-bit unsigned intege
    bool ok;
    uint32_t write_value = val.toUInt(&ok, 16);

    if(ok){
        qDebug() << QString("Writing payload %1").arg(write_value);

        try{
            // Perform the write operation to the specified node
            hw.getNode(reg.toStdString()).write(write_value);
            hw.dispatch();  // Commit the write transaction
        } catch(const std::exception& e){
            qDebug() << e.what();  // Log hardware-level exception
            return;
        }
    } else{
        qDebug() << "Could not convert" << val << "to uint32_t";
        return;
    }
}

void RegisterController::openWriteDialog(QTreeWidgetItem* item, QMap<QString, DeviceInfo>& deviceInfoMap, QComboBox* cbDeviceDropdown)
{
    // Create a modal dialog for writing to a register
    QDialog dialog(parentWidget);
    dialog.setWindowTitle("Write Register");

    // Retrieve current register name and values from the tree item
    QString regName = item->text(0);
    QString currentDec = item->text(2);
    QString currentHex = item->text(3);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Display register name (read-only)
    QHBoxLayout* regLayout = new QHBoxLayout();
    QLabel* regLabel = new QLabel("Register:", &dialog);
    regLabel->setObjectName("lblDialog");
    regLayout->addWidget(regLabel);
    QLineEdit* regEdit = new QLineEdit(regName, &dialog);
    regEdit->setReadOnly(true);
    regEdit->setStyleSheet("background: #f0f0f0;");
    regLayout->addWidget(regEdit);
    layout->addLayout(regLayout);

    // Display current decimal value (read-only)
    QHBoxLayout* curDecLayout = new QHBoxLayout();
    QLabel* curDecLabel = new QLabel("Current Dec:", &dialog);
    curDecLabel->setObjectName("lblDialog");
    curDecLayout->addWidget(curDecLabel);
    QLineEdit* curDecEdit = new QLineEdit(currentDec, &dialog);
    curDecEdit->setReadOnly(true);
    curDecEdit->setStyleSheet("background: #f0f0f0;");
    curDecLayout->addWidget(curDecEdit);
    layout->addLayout(curDecLayout);

    // Display current hexadecimal value (read-only)
    QHBoxLayout* curHexLayout = new QHBoxLayout();
    QLabel* curHexLabel = new QLabel("Current Hex:", &dialog);
    curHexLabel->setObjectName("lblDialog");
    curHexLayout->addWidget(curHexLabel);
    QLineEdit* curHexEdit = new QLineEdit(currentHex, &dialog);
    curHexEdit->setReadOnly(true);
    curHexEdit->setStyleSheet("background: #f0f0f0;");
    curHexLayout->addWidget(curHexEdit);
    layout->addLayout(curHexLayout);

    // Input for new hexadecimal value
    QLabel* hexLabel = new QLabel("Hex value (e.g. FF):", &dialog);
    QLineEdit* hexInput = new QLineEdit(&dialog);
    hexInput->setPlaceholderText("Enter hex");

    // Input for new decimal value
    QLabel* decLabel = new QLabel("Decimal value:", &dialog);
    QLineEdit* decInput = new QLineEdit(&dialog);
    decInput->setPlaceholderText("Enter decimal");

    hexLabel->setObjectName("lblDialog");
    decLabel->setObjectName("lblDialog");

    layout->addWidget(hexLabel);
    layout->addWidget(hexInput);
    layout->addWidget(decLabel);
    layout->addWidget(decInput);

    // Sync decimal value when hexadecimal input changes
    QObject::connect(hexInput, &QLineEdit::textChanged, [&]() {
        bool ok = false;
        uint32_t val = hexInput->text().trimmed().toUInt(&ok, 16);
        if (ok) {
            decInput->blockSignals(true);
            decInput->setText(QString::number(val));
            decInput->blockSignals(false);
        }
    });

    // Sync hexadecimal value when decimal input changes
    QObject::connect(decInput, &QLineEdit::textChanged, [&]() {
        bool ok = false;
        uint32_t val = decInput->text().trimmed().toUInt(&ok, 10);
        if (ok) {
            hexInput->blockSignals(true);
            hexInput->setText(QString::number(val, 16).toUpper());
            hexInput->blockSignals(false);
        }
    });

    // Buttons for Write and Cancel actions
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* writeBtn = new QPushButton("Write", &dialog);
    QPushButton* cancelBtn = new QPushButton("Cancel", &dialog);
    cancelBtn->setObjectName("btnCancel");
    btnLayout->addStretch();
    btnLayout->addWidget(writeBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // Close dialog on Cancel
    QObject::connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    // Write to hardware on confirmation
    QObject::connect(writeBtn, &QPushButton::clicked, [&]() {
        QString decText = decInput->text().trimmed();
        bool ok = false;
        uint32_t val = decText.toUInt(&ok, 10);

        if (!ok) {
            QMessageBox::warning(&dialog, "Invalid Input", "Please enter a valid decimal or hex value.");
            return;
        }

        // Retrieve selected device from dropdown and validate
        QString selectedDeviceId = cbDeviceDropdown->currentText();
        if (!deviceInfoMap.contains(selectedDeviceId)) {
            QMessageBox::warning(&dialog, "Error", "Selected device not found.");
            return;
        }

        // Construct URI and XML path
        const DeviceInfo& info = deviceInfoMap[selectedDeviceId];
        QString device_path = info.deviceXmlPath;
        QString protocol = info.connectionType + "-" + info.connectionVersion;
        QString dev_xml = device_path.startsWith("file://") ? device_path : "file://" + device_path;
        QString uri;

        if (info.connectionType == "chtcp") {
            uri = protocol + "://" + info.gatewayHost + ":" + QString::number(info.gatewayPort)
                  + "?target=" + info.ip + ":" + QString::number(info.port);
        } else {
            uri = protocol + "://" + info.ip + ":" + QString::number(info.port);
        }

        try {
            // Connect to device and write the register value
            uhal::HwInterface hw = uhal::ConnectionManager::getDevice(selectedDeviceId.toStdString(),
                                                                      uri.toStdString(), dev_xml.toStdString());
            writeNode(hw, regName, QString::number(val, 16));

            // Update tree widget display
            item->setText(2, QString::number(val));
            item->setText(3, QString("0x%1").arg(val, 0, 16).toUpper());
            item->setData(2, Qt::UserRole, item->text(2));
            item->setData(3, Qt::UserRole, item->text(3));

            QMessageBox::information(&dialog, "Write Successful", "Value written successfully.");
            dialog.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dialog, "Write Error", e.what());
        }
    });

    // Show dialog modally
    dialog.exec();
}

void RegisterController::performRead(bool resetFilters,
                                     bool clearTreeBeforeRead,
                                     QVector<QLineEdit*> filterEdits,
                                     QComboBox* cbDeviceDropdown,
                                     QMap<QString, DeviceInfo>& deviceInfoMap)
{
    if (clearTreeBeforeRead) {
        registerTreeViewHandler->tData->clear();
        registerTreeViewHandler->nodeCache.clear();
        nodeMetadataCache.clear();  // clear metadata when tree is rebuilt
    }

    registerTreeViewHandler->tData->setSortingEnabled(false);

    if (resetFilters) {
        for (QLineEdit* filter : filterEdits) {
            if (filter) filter->clear();
        }
    }

    QString selectedDeviceId = cbDeviceDropdown->currentText();
    if (!deviceInfoMap.contains(selectedDeviceId)) {
        QMessageBox::warning(parentWidget, "Error", "Selected device not found.");
        return;
    }

    const DeviceInfo& info = deviceInfoMap[selectedDeviceId];
    QString device_path = info.deviceXmlPath;
    QString protocol = info.connectionType + "-" + info.connectionVersion;
    QString uri;

    if (!device_path.startsWith("file://"))
        device_path = "file://" + device_path;

    if (info.connectionType == "chtcp") {
        uri = protocol + "://" + info.gatewayHost + ":" + QString::number(info.gatewayPort)
              + "?target=" + info.ip + ":" + QString::number(info.port);
    } else {
        uri = protocol + "://" + info.ip + ":" + QString::number(info.port);
    }

    try {
        ensureCachedHw(selectedDeviceId, deviceInfoMap);
        readNodes(*cachedHw);
    } catch (const std::exception& e) {
        qDebug() << e.what();
        return;
    }

    registerTreeViewHandler->tData->setSortingEnabled(true);
}

void RegisterController::ensureCachedHw(const QString& deviceId, const QMap<QString, DeviceInfo>& deviceInfoMap)
{
    if (cachedHw && deviceId == cachedDeviceId) {
        return;  // Already cached
    }

    // If device changed, delete old interface
    delete cachedHw;
    cachedHw = nullptr;

    if (!deviceInfoMap.contains(deviceId)) {
        throw std::runtime_error("Device ID not found in map");
    }

    const DeviceInfo& info = deviceInfoMap[deviceId];

    QString device_path = info.deviceXmlPath;
    QString protocol = info.connectionType + "-" + info.connectionVersion;
    QString uri;

    if (!device_path.startsWith("file://")) {
        device_path = "file://" + device_path;
    }

    if (info.connectionType == "chtcp") {
        uri = protocol + "://" + info.gatewayHost + ":" + QString::number(info.gatewayPort)
              + "?target=" + info.ip + ":" + QString::number(info.port);
    } else {
        uri = protocol + "://" + info.ip + ":" + QString::number(info.port);
    }

    // Create and cache the new interface
    cachedHw = new uhal::HwInterface(
        uhal::ConnectionManager::getDevice(deviceId.toStdString(), uri.toStdString(), device_path.toStdString())
        );

    cachedDeviceId = deviceId;
}

