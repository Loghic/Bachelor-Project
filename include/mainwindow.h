#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <exception>
#include <uhal/uhal.hpp>
#include <uhal/ConnectionManager.hpp>
#include <uhal/log/log.hpp>
#include <uhal/log/exception.hpp>

#include <typeinfo>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QTabWidget>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QFile>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QMap>
#include <QMenuBar>
#include <QMenu>
#include <QGroupBox>
#include <QFormLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QStringList>
#include <QSet>
#include <QTreeWidget>
#include <QCloseEvent>
#include <QElapsedTimer>
#include <QFuture>

#include "clickablerectitem.h"
#include "draggableview.h"
#include "devicemanager.h"
#include "registercontroller.h"
#include "registertreeviewhandler.h"

class MainWindow : public QWidget {
    Q_OBJECT

private:
    void filterDeviceDropdown(const QString &text);

    void setupMenuBar();
    void setupConnectionTab(int tabIndex);
    void setupDataTab();
    void setupLoggedUser();
    void setupDeviceInfo();
    void pauseContinuousRead();

public:
    explicit MainWindow(int tabIndex = 0, const QString &username = "", QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void selectDeviceFromScene(const QString &deviceId);

private slots:
    void onReadButtonClicked();
    void readNodes(uhal::HwInterface& hw);
    void writeNode(uhal::HwInterface& hw, const QString& reg, const QString& val);

    void openWriteDialog(QTreeWidgetItem* item);
    void onPermissionCellClicked(QTreeWidgetItem* item, int column);

    void logout();
    void browseXmlFile();
    void useSettings();
    void saveSettings();

    void toggleContinuousRead();
    void stopContinuousRead();
    void switchToDataTab();
    void switchToConnectionTab();
    void performRead(bool resetFilters, bool clearTreeBeforeRead);
    void onReadTimerTimeout();


    void closeEvent(QCloseEvent *event) override;

    void loadDeviceListFromXml();

    void resetZoom();
    void fitToScreen();
    void zoomIn();
    void zoomOut();
    void restoreDefaultLayout();
    void centerOnSelected();
    void resetToLoadedLayout();

private:
    QTabWidget *tWidget;
    QWidget *wConnection, *wData, *wUserInfo;
    QGridLayout *glConn, *glSettings;
    QVBoxLayout *vlDataTab, *vlButton, *vlUser, *vlUsernInfo;
    QHBoxLayout *hlMain, *hlWriteButtons, *hlSettingsButtons, *hlLoggedUser;

    QMenuBar *menuBar;
    QMenu *viewMenu;
    QAction *actionResetView, *actionFitToScreen, *actionZoomIn,
        *actionZoomOut, *actionRestoreLayout, *actionCenterOnSelected, *actionResetToLoadedLayout;

    QGroupBox *gbLoggedUserInfo, *gbDeviceInfoBox, *gbSettingsBox;
    QFormLayout *formDeviceInfo;

    QLabel *lblDeviceDropdown, *lblConnetionFile, *lblReadSpeed, *lblLoggedIn, *lblUser;
    QComboBox *cbDeviceDropdown, *cbReadSpeed;
    QCheckBox *chbConnectionAccess;
    QPushButton *btnRead, *btnReadContinuously, *btnWrite, *btnPause, *btnStop, *btnBrowse, *btnUse, *btnSave, *btnLogout;
    QLineEdit *leSearchDevice, *leXmlPathInput, *leDeviceId, *leDeviceIp, *leDevicePort, *leDeviceConnType, *leDeviceGatewayHost, *leDeviceGatewayPort;
    QTreeWidget *tData;

    DraggableView *diagramView;    // Graphics View for diagram
    QGraphicsScene *diagramScene;  // Scene to hold the graphical items

    QMap<QString, QPointF> defaultDevicePositions;
    QMap<QString, QPointF> initialDevicePositions;

    QString strLoggedInUser;
    QStringList allDevices;

    QMap<QString, ClickableRectItem*> deviceItems;
    QMap<QString, DeviceInfo> deviceInfoMap;

    QVector<QLineEdit*> filterEdits;

    QSet<int> modifiedRows;

    // QTimer* readTimer;
    bool readingEnabled = false;
    QFuture<void> readFuture;

    bool dataTabSwitchedOnce = false;
    DeviceManager deviceManager;
    RegisterController* registerController;
    RegisterTreeViewHandler* registerTreeViewHandler;

    qint64 readCount = 0;
    QElapsedTimer readRateTimer;
    QLabel *lblReadCount;
};

#endif // MAINWINDOW_H
