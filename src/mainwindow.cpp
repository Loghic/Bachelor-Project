#include "include/mainwindow.h"
#include "include/loginwindow.h"

#include <QElapsedTimer>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

MainWindow::MainWindow(int tabIndex, const QString &username, QWidget *parent)
    : QWidget(parent), strLoggedInUser(username) {

    tWidget = new QTabWidget(this);
    wConnection = new QWidget(this);
    wData = new QWidget(this);
    wUserInfo = new QWidget(this);

    // ======= UI setups =======
    setupMenuBar();
    setupConnectionTab(tabIndex);
    setupDataTab();
    setupLoggedUser();
    setupDeviceInfo();

    // ======= Main Layout =======
    hlMain = new QHBoxLayout(this);
    hlMain->setMenuBar(menuBar);
    hlMain->addWidget(tWidget);
    hlMain->addWidget(wUserInfo, 0, Qt::AlignTop);
    hlMain->setStretchFactor(tWidget, 2);
    hlMain->setStretchFactor(wUserInfo, 0);   

    setLayout(hlMain);
    setWindowTitle("Device Configuration");

    // ======= Load devices from XML =======
    loadDeviceListFromXml();

    // ======= Show maximized after full setup =======
    QTimer::singleShot(0, this, [this]() {
        this->showMaximized();
    });

    registerController = new RegisterController(this, tData);
    readingEnabled = false;
}

MainWindow::~MainWindow() {}

void MainWindow::logout() {
    readingEnabled = false;
    if (readFuture.isRunning())
        readFuture.waitForFinished();

    // Prompt the user with a confirmation dialog for logout
    int response = QMessageBox::question(this, "Logout", "Are you sure you want to logout?",
                                         QMessageBox::Yes | QMessageBox::No);

    // Proceed only if the user confirms
    if (response == QMessageBox::Yes) {
        this->close(); // Close the current main window

        // Create and show the login window
        LoginWindow *loginWindow = new LoginWindow();

        // If login is accepted, show a new main window instance
        if (loginWindow->exec() == QDialog::Accepted) {
            // Create a new MainWindow with the username from the login
            MainWindow *mainWin = new MainWindow(0, loginWindow->getUsername());
            mainWin->show();
        }
    }
}

void MainWindow::setupMenuBar()
{
    // Create menu bar and "View" menu
    menuBar = new QMenuBar(this);
    viewMenu = new QMenu("View", this);
    menuBar->addMenu(viewMenu);

    // Create actions
    actionResetView = new QAction("Reset Zoom", this);
    actionFitToScreen = new QAction("Fit to Screen", this);
    actionZoomIn = new QAction("Zoom In", this);
    actionZoomOut = new QAction("Zoom Out", this);
    actionRestoreLayout = new QAction("Restore Default Layout", this);
    actionCenterOnSelected = new QAction("Center on Selected", this);
    actionResetToLoadedLayout = new QAction("Reset to Loaded Layout", this);

    // Set shortcuts
    actionZoomIn->setShortcut(QKeySequence("Ctrl++"));
    actionZoomOut->setShortcut(QKeySequence("Ctrl+-"));
    actionResetView->setShortcut(QKeySequence("Ctrl+R"));
    actionFitToScreen->setShortcut(QKeySequence("Ctrl+F"));
    actionCenterOnSelected->setShortcut(QKeySequence("CTRL+E"));
    actionResetToLoadedLayout->setShortcut(QKeySequence("Ctrl+L"));

    // Add actions to menu
    viewMenu->addActions({
        actionResetView,
        actionFitToScreen,
        actionZoomIn,
        actionZoomOut,
        actionRestoreLayout,
        actionCenterOnSelected,
        actionResetToLoadedLayout
    });

    // Connect signals to slots
    connect(actionResetView, &QAction::triggered, this, &MainWindow::resetZoom);
    connect(actionFitToScreen, &QAction::triggered, this, &MainWindow::fitToScreen);
    connect(actionZoomIn, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(actionZoomOut, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(actionRestoreLayout, &QAction::triggered, this, &MainWindow::restoreDefaultLayout);
    connect(actionCenterOnSelected, &QAction::triggered, this, &MainWindow::centerOnSelected);
    connect(actionResetToLoadedLayout, &QAction::triggered, this, &MainWindow::resetToLoadedLayout);
}

void MainWindow::setupConnectionTab(int tabIndex)
{
    // ======= Setup Layout =======
    glConn = new QGridLayout(wConnection);
    int row = 0; // Used to track layout row positioning

    // ======= Diagram View (Graphics Scene) =======
    diagramScene = new QGraphicsScene(this);
    diagramView = new DraggableView(diagramScene);
    diagramView->setRenderHint(QPainter::Antialiasing);
    glConn->addWidget(diagramView, row, 0, 2, 4);
    row += 2;

    // ======= Device Search (Regex Supported) =======
    leSearchDevice = new QLineEdit(this);
    leSearchDevice->setPlaceholderText("Search devices (regex supported)");
    glConn->addWidget(leSearchDevice, row, 0, 1, 4);
    ++row;

    connect(leSearchDevice, &QLineEdit::textChanged, this, &MainWindow::filterDeviceDropdown);

    // ======= Device Dropdown =======
    lblDeviceDropdown = new QLabel("Choose device:", this);
    cbDeviceDropdown = new QComboBox(this);

    connect(cbDeviceDropdown, &QComboBox::currentTextChanged,
            this, &MainWindow::selectDeviceFromScene);

    glConn->addWidget(lblDeviceDropdown, row, 0);
    glConn->addWidget(cbDeviceDropdown, row, 1, 1, 3);
    ++row;

    // ======= XML File Path Input =======
    QSettings settings;

    lblConnetionFile = new QLabel("Connection XML Path:", this);
    leXmlPathInput = new QLineEdit(this);
    leXmlPathInput->setPlaceholderText("Enter or browse XML file path");
    leXmlPathInput->setText(settings.value("xmlPath", "").toString());

    btnBrowse = new QPushButton("Browse", this);
    connect(btnBrowse, &QPushButton::clicked, this, &MainWindow::browseXmlFile);

    glConn->addWidget(lblConnetionFile, row, 0, 1, 1);
    glConn->addWidget(leXmlPathInput, row, 1, 1, 2);
    glConn->addWidget(btnBrowse, row, 3, 1, 1);
    ++row;

    // ======= Read Speed Dropdown =======
    lblReadSpeed = new QLabel("Continuous Read Interval:", this);
    cbReadSpeed = new QComboBox(this);
    cbReadSpeed->addItem("10 ms", 10);
    cbReadSpeed->addItem("20 ms", 20);
    cbReadSpeed->addItem("50 ms", 50);
    cbReadSpeed->addItem("100 ms", 100);
    cbReadSpeed->addItem("500 ms", 500);
    cbReadSpeed->addItem("1 s", 1000);
    cbReadSpeed->addItem("2 s", 2000);
    cbReadSpeed->addItem("3 s", 3000);
    cbReadSpeed->addItem("1 min", 60000);
    cbReadSpeed->addItem("2 min", 120000);

    int savedSpeed = settings.value("readSpeed", 1000).toInt();
    int index = cbReadSpeed->findData(savedSpeed);
    if (index != -1) cbReadSpeed->setCurrentIndex(index);

    glConn->addWidget(lblReadSpeed, row, 0);
    glConn->addWidget(cbReadSpeed, row, 1, 1, 3);
    ++row;

    // ======= Read Buttons =======
    btnRead = new QPushButton("Read", this);
    btnReadContinuously = new QPushButton("Read Continuously", this);

    connect(btnRead, &QPushButton::clicked, this, &MainWindow::onReadButtonClicked);
    connect(btnReadContinuously, &QPushButton::clicked, this, &MainWindow::toggleContinuousRead);

    glConn->addWidget(btnRead, row, 0, 1, 2);
    glConn->addWidget(btnReadContinuously, row, 2, 1, 2);

    // ======= Finalize Layout and Tab =======
    wConnection->setLayout(glConn);
    tWidget->addTab(wConnection, "Connection");
    tWidget->setCurrentIndex(tabIndex);
}

void MainWindow::setupDataTab()
{
    // ======= Main Vertical Layout for Data Tab =======
    vlDataTab = new QVBoxLayout(wData);

    // ======= 1. Filter Row =======
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QStringList headers = {"node", "addr", "dec", "hex", "mask", "permission"};

    for (const QString& header : headers) {
        QLineEdit* edit = new QLineEdit();
        edit->setPlaceholderText("Filter " + header);
        edit->setClearButtonEnabled(true);
        filterLayout->addWidget(edit);
        filterEdits.append(edit);

        // Connect each filter field to the filtering logic
        connect(edit, &QLineEdit::textChanged, this, [=]() {
            registerTreeViewHandler->filterTree(filterEdits);
        });
    }

    vlDataTab->addLayout(filterLayout);  // Add filter row to the top of the tab

    // ======= 2. Data Table (QTreeWidget) =======
    tData = new QTreeWidget(wData);
    tData->setColumnCount(6);
    tData->setHeaderLabels({"node", "addr", "dec", "hex", "mask", "permission"});
    tData->header()->setStretchLastSection(true);
    tData->header()->setSectionResizeMode(QHeaderView::Stretch);
    tData->setAlternatingRowColors(true);
    tData->setSortingEnabled(true);
    tData->setSelectionBehavior(QAbstractItemView::SelectRows);
    tData->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

    // Connect table events to handlers
    registerTreeViewHandler = new RegisterTreeViewHandler(tData);
    // connect(tData, &QTreeWidget::itemChanged, this, &MainWindow::on_tree_item_changed);
    connect(tData, &QTreeWidget::itemClicked, this, &MainWindow::onPermissionCellClicked);


    vlDataTab->addWidget(tData);  // Add the table below the filters

    // ======= 3. Stop Button for Continuous Read =======
    hlWriteButtons = new QHBoxLayout();
    btnStop = new QPushButton("Stop", this);
    btnStop->setEnabled(false); // Initially disabled

    connect(btnStop, &QPushButton::clicked, this, &MainWindow::stopContinuousRead);
    lblReadCount = new QLabel("Reads: 0", this);
    hlWriteButtons->addWidget(lblReadCount);

    hlWriteButtons->addWidget(btnStop);

    // Stretch: lblReadCount gets minimal space (0), btnStop expands (1)
    hlWriteButtons->setStretch(0, 0); // lblReadCount
    hlWriteButtons->setStretch(1, 1); // btnStop
    vlDataTab->addLayout(hlWriteButtons); // Add button below the table

    // ======= Finalize Tab Setup =======
    wData->setLayout(vlDataTab);
    tWidget->addTab(wData, "Data");
}

void MainWindow::setupLoggedUser()
{
    // ======= Main container widget and layout =======
    vlUsernInfo = new QVBoxLayout(wUserInfo); // Main vertical layout

    // ======= Group box to display user info =======
    gbLoggedUserInfo = new QGroupBox("Logged User info:", this);
    vlUser = new QVBoxLayout();
    hlLoggedUser = new QHBoxLayout(wUserInfo);

    // ======= User info labels =======
    lblLoggedIn = new QLabel("User:", this);
    lblLoggedIn->setObjectName("lblLoggedIn");

    lblUser = new QLabel(strLoggedInUser, this);
    lblUser->setObjectName("lblUser");

    // Add labels side by side
    hlLoggedUser->addWidget(lblLoggedIn);
    hlLoggedUser->addWidget(lblUser);

    // Add label layout to vertical layout
    vlUser->addLayout(hlLoggedUser);

    // ======= Logout Button =======
    btnLogout = new QPushButton("Logout", this);
    btnLogout->setObjectName("btnLogout");

    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::logout);

    // Add logout button with top alignment
    vlUser->addWidget(btnLogout, 0, Qt::AlignTop);

    // ======= Finalize Group Box and Parent Layout =======
    gbLoggedUserInfo->setLayout(vlUser);
    vlUsernInfo->addWidget(gbLoggedUserInfo);
}

void MainWindow::setupDeviceInfo()
{
    // ======= Group box and layout for displaying device information =======
    gbDeviceInfoBox = new QGroupBox("Selected Device Info", this);
    formDeviceInfo = new QFormLayout(gbDeviceInfoBox);

    // ======= Read-only QLineEdits for device fields =======
    leDeviceId = new QLineEdit(this);
    leDeviceIp = new QLineEdit(this);
    leDeviceGatewayHost = new QLineEdit(this);
    leDeviceGatewayPort = new QLineEdit(this);
    leDevicePort = new QLineEdit(this);
    leDeviceConnType = new QLineEdit(this);

    // Make all device fields read-only
    leDeviceId->setReadOnly(true);
    leDeviceIp->setReadOnly(true);
    leDeviceGatewayHost->setReadOnly(true);
    leDeviceGatewayPort->setReadOnly(true);
    leDevicePort->setReadOnly(true);
    leDeviceConnType->setReadOnly(true);

    // Set object names for stylesheet access or identification
    leDeviceId->setObjectName("DeviceInfoField");
    leDeviceIp->setObjectName("DeviceInfoField");
    leDevicePort->setObjectName("DeviceInfoField");
    leDeviceConnType->setObjectName("DeviceInfoField");
    leDeviceGatewayHost->setObjectName("DeviceInfoField");
    leDeviceGatewayPort->setObjectName("DeviceInfoField");

    // ======= Add label-field pairs to the form layout =======
    formDeviceInfo->addRow("Device ID:", leDeviceId);
    formDeviceInfo->addRow("IP Address:", leDeviceIp);
    formDeviceInfo->addRow("GatewayHost:", leDeviceGatewayHost);
    formDeviceInfo->addRow("GatewayPort:", leDeviceGatewayPort);
    formDeviceInfo->addRow("Port:", leDevicePort);
    formDeviceInfo->addRow("Connection:", leDeviceConnType);

    // ======= Layout styling and spacing =======
    formDeviceInfo->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formDeviceInfo->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
    formDeviceInfo->setHorizontalSpacing(15);
    formDeviceInfo->setVerticalSpacing(8);

    gbDeviceInfoBox->setLayout(formDeviceInfo);

    // ======= Add to main user info layout =======
    vlUsernInfo->addWidget(gbDeviceInfoBox);

    // Spacing and padding for entire user info section
    vlUsernInfo->setSpacing(15);
    vlUsernInfo->setContentsMargins(10, 10, 10, 10);

    wUserInfo->setLayout(vlUsernInfo);
}

void MainWindow::filterDeviceDropdown(const QString &text)
{
    // Create a case-insensitive regular expression from the input text
    QRegularExpression regex(text, QRegularExpression::CaseInsensitiveOption);

    // Clear the current dropdown items
    cbDeviceDropdown->clear();

    // Filter the list of all available devices using the regex
    for (const QString &device : allDevices) {
        if (regex.match(device).hasMatch()) {
            cbDeviceDropdown->addItem(device); // Add matching device to dropdown
        }
    }

    // If at least one device matches, select the first one by default
    if (cbDeviceDropdown->count() > 0) {
        cbDeviceDropdown->setCurrentIndex(0);
        selectDeviceFromScene(cbDeviceDropdown->currentText());
    }
}

void MainWindow::stopContinuousRead()
{
    pauseContinuousRead();
    dataTabSwitchedOnce = false;
    switchToConnectionTab();
}

void MainWindow::pauseContinuousRead()
{
    readingEnabled = false;
    btnReadContinuously->setText("Read Continuously");
    btnStop->setEnabled(false);
}

void MainWindow::switchToConnectionTab()
{
    int dataTabIndex = tWidget->indexOf(wConnection);
    if (dataTabIndex != -1) {
        tWidget->setCurrentIndex(dataTabIndex);
    }
}

void MainWindow::switchToDataTab()
{
    int dataTabIndex = tWidget->indexOf(wData);
    if (dataTabIndex != -1) {
        tWidget->setCurrentIndex(dataTabIndex);
    }
}

void MainWindow::onReadTimerTimeout()
{

    QElapsedTimer timer;
    timer.start();

    bool resetFilters = false;
    bool clearTreeBeforeRead = false;
    performRead(resetFilters, clearTreeBeforeRead);

    qint64 elapsed = timer.elapsed();  // In milliseconds
    qDebug() << "performRead took:" << elapsed << "ms";
}

void MainWindow::performRead(bool resetFilters, bool clearTreeBeforeRead)
{
    if (registerController){
        registerController->performRead(resetFilters, clearTreeBeforeRead, filterEdits, cbDeviceDropdown, deviceInfoMap);

    }
}

void MainWindow::onReadButtonClicked()
{
    dataTabSwitchedOnce = false;  // Reset for manual reads
    switchToDataTab();
    bool resetFilters = true;
    bool clearTreeBeforeRead = true;
    performRead(resetFilters, clearTreeBeforeRead); // Reset filters on manual read
}

void MainWindow::toggleContinuousRead() {
    if (readingEnabled) {
        stopContinuousRead();
    } else {
        int interval = cbReadSpeed->currentData().toInt();
        if (interval <= 0) interval = 1000;

        if (!dataTabSwitchedOnce) {
            switchToDataTab();
            dataTabSwitchedOnce = true;
        }

        btnReadContinuously->setText("Stop Reading");
        btnStop->setEnabled(true);
        dataTabSwitchedOnce = true;

        readingEnabled = true;
        readCount = 0;
        readRateTimer.start();  // Start measuring read rate

        readFuture = QtConcurrent::run([this, interval]() {
            QElapsedTimer loopTimer;
            loopTimer.start();

            while (readingEnabled) {
                if (loopTimer.elapsed() >= interval) {
                    loopTimer.restart();
                    readCount++;

                    QMetaObject::invokeMethod(this, [this]() {
                            QElapsedTimer timer;
                            timer.start();
                            performRead(false, false);
                            qint64 elapsed = timer.elapsed();
                            qDebug() << "performRead took:" << elapsed << "ms";

                            // Calculate and show reads per second every 1 sec
                            if (readRateTimer.elapsed() >= 1000) {
                                double rps = static_cast<double>(readCount) / (readRateTimer.elapsed() / 1000.0);
                                lblReadCount->setText(QString("Reads/sec: %1").arg(rps, 0, 'f', 2));
                                readRateTimer.restart();
                                readCount = 0;
                            }
                        }, Qt::QueuedConnection);
                }

                QThread::msleep(1);
            }
        });
    }
}

void MainWindow::openWriteDialog(QTreeWidgetItem* item)
{
    if (registerController){
        registerController->openWriteDialog(item, deviceInfoMap, cbDeviceDropdown);
    }
}

void MainWindow::readNodes(uhal::HwInterface& hw)
{
    if (registerController){
        registerController->readNodes(hw);
    }
}

void MainWindow::writeNode(uhal::HwInterface& hw, const QString& reg, const QString& val)
{
    if (registerController){
        registerController->writeNode(hw, reg, val);
    }
}

void MainWindow::onPermissionCellClicked(QTreeWidgetItem* item, int column)
{
    // Ignore clicks on the node name column (column 0)

    if (column == 0) return;

    // Check if the clicked item has write permission
    QString permission = item->text(5);
    if (permission == "Write") {
        // Open the write dialog for this register
        openWriteDialog(item);
    }
}

void MainWindow::browseXmlFile() {
    // Open a file dialog to let the user select an XML file
    QString fileName = QFileDialog::getOpenFileName(this, "Select XML File", QString(), "XML Files (*.xml);;All Files (*)");

    // If a file was selected, update the input field
    if (!fileName.isEmpty()) {
        leXmlPathInput->setText(fileName);
    }

    // Apply the new settings after selecting the file
    useSettings();
}

void MainWindow::useSettings() {
    // Load and apply device list from the currently selected XML path
    loadDeviceListFromXml();
}

void MainWindow::saveSettings() {
    // Save user preferences (XML path and read speed) using QSettings

    QSettings settings;

    // Save current XML file path
    settings.setValue("xmlPath", leXmlPathInput->text());

    // Save currently selected read speed (from combobox userData)
    int selectedSpeed = cbReadSpeed->currentData().toInt();
    settings.setValue("readSpeed", selectedSpeed);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Called when the main window is closing
    readingEnabled = false;
    if (readFuture.isRunning())
        readFuture.waitForFinished();

    // Save the device widget positions and other user settings
    deviceManager.saveDevicePositions(deviceItems);
    saveSettings();

    // Call base class implementation to ensure normal close event handling
    QWidget::closeEvent(event);
}

void MainWindow::loadDeviceListFromXml() {
    QString xmlPath = leXmlPathInput->text();
    QList<DeviceEntry> deviceList;
    QString errorMsg;

    if (!deviceManager.loadDeviceListFromXml(xmlPath, deviceList, deviceInfoMap, errorMsg)) {
        QMessageBox::warning(this, "XML Error", errorMsg);
        return;
    }

    auto groupedDevices = deviceManager.groupAndSortDevices(deviceList);

    cbDeviceDropdown->clear();
    diagramScene->clear();
    deviceItems.clear();
    defaultDevicePositions.clear();
    initialDevicePositions.clear();
    allDevices.clear();

    const int maxPerRow = 8, spacingX = 150, spacingY = 100, groupSpacingY = 30;
    int currentY = 0;

    for (const QString& groupKey : groupedDevices.keys()) {
        const QList<DeviceEntry>& entries = groupedDevices[groupKey];
        int rowOffset = 0, col = 0;

        for (const auto& entry : entries) {
            cbDeviceDropdown->addItem(entry.id);
            int posX = (col % maxPerRow) * spacingX;
            int posY = currentY + rowOffset * spacingY;

            ClickableRectItem* item = deviceManager.createDeviceBlock(
                entry.id, posX, posY, defaultDevicePositions, initialDevicePositions
                );
            deviceItems[entry.id] = item;
            connect(item, &ClickableRectItem::deviceClicked, this, &MainWindow::selectDeviceFromScene);
            diagramScene->addItem(item);

            col++;
            if (col % maxPerRow == 0) rowOffset++;
        }

        currentY += ((entries.size() + maxPerRow - 1) / maxPerRow) * spacingY + groupSpacingY;
    }

    for (int i = 0; i < cbDeviceDropdown->count(); ++i) {
        allDevices << cbDeviceDropdown->itemText(i);
    }
}

void MainWindow::selectDeviceFromScene(const QString &deviceId)
{
    // Find the device in the dropdown list and select it if found
    int index = cbDeviceDropdown->findText(deviceId);
    if (index != -1) {
        cbDeviceDropdown->setCurrentIndex(index);
    }

    // Update the visual selection style for device blocks in the scene
    for (auto it = deviceItems.begin(); it != deviceItems.end(); ++it) {
        // Highlight the selected device block, reset others
        it.value()->setSelectedStyle(it.key() == deviceId);
    }

    // If device info is available for the selected device, display it
    if (deviceInfoMap.contains(deviceId)) {
        const DeviceInfo &info = deviceInfoMap[deviceId];

        // Populate UI fields with device details
        leDeviceId->setText(deviceId);
        leDeviceIp->setText(info.ip);
        leDevicePort->setText(QString::number(info.port));
        leDeviceGatewayHost->setText(info.gatewayHost);
        leDeviceGatewayPort->setText(QString::number(info.gatewayPort));

        // Build connection type string including version if present
        QString connStr = info.connectionType;
        if (!info.connectionVersion.isEmpty()) {
            connStr += "-" + info.connectionVersion;
        }
        leDeviceConnType->setText(connStr);
    }
}

void MainWindow::resetZoom()
{
    // Reset any transformations applied to the diagram view,
    // returning it to its original zoom
    diagramView->resetTransform();
}

void MainWindow::fitToScreen()
{
    // If there are no items in the scene, no need to fit
    if (diagramScene->items().isEmpty())
        return;

    QRectF boundingRect;
    // Calculate bounding rectangle that contains all items in the scene
    for (QGraphicsItem *item : diagramScene->items()) {
        // Union the current bounding rectangle with each item's bounding rect
        boundingRect |= item->sceneBoundingRect();
    }

    // Add padding around the bounding rect for some breathing room
    const int padding = 20;
    boundingRect.adjust(-padding, -padding, padding, padding);

    // Fit the view to the bounding rectangle while keeping aspect ratio
    diagramView->fitInView(boundingRect, Qt::KeepAspectRatio);
}

void MainWindow::zoomIn()
{
    // Scale up the view by 20% to zoom in
    diagramView->scale(1.2, 1.2);
}

void MainWindow::zoomOut()
{
    // Scale down the view by 20% to zoom out (inverse of zoom in)
    diagramView->scale(0.8, 0.8);
}

void MainWindow::restoreDefaultLayout()
{
    // Reset all device blocks to their default saved positions
    for (const QString& deviceId : deviceItems.keys()) {
        if (defaultDevicePositions.contains(deviceId)) {
            deviceItems[deviceId]->setPos(defaultDevicePositions[deviceId]);
        }
    }

    // Remove any saved positions from QSettings so next launch uses default layout
    QSettings settings("CERN", "Register GUI");
    settings.beginGroup("DevicePositions");
    for (const QString& deviceId : deviceItems.keys()) {
        settings.remove(deviceId + "/x");
        settings.remove(deviceId + "/y");
    }
    settings.endGroup();
}

void MainWindow::resetToLoadedLayout() {
    // Restore device blocks to the positions loaded initially (from saved settings or defaults)
    for (const QString& deviceId : deviceItems.keys()) {
        if (initialDevicePositions.contains(deviceId)) {
            deviceItems[deviceId]->setPos(initialDevicePositions[deviceId]);
        }
    }
}

void MainWindow::centerOnSelected()
{
    // Reset zoom to default before centering
    resetZoom();

    // Get currently selected device from the dropdown
    QString selectedDeviceId = cbDeviceDropdown->currentText();
    if (!selectedDeviceId.isEmpty()) {
        // Find the corresponding graphics item and center the view on it
        QGraphicsItem *item = deviceItems[selectedDeviceId];
        if (item) {
            diagramView->centerOn(item);
        }
    }
}
