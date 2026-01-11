#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QMap>
#include <QString>
#include <QList>
#include <QPointF>
#include <QSettings>
#include <QGraphicsScene>
#include "clickablerectitem.h"

struct DeviceInfo {
    QString connectionType;
    QString connectionVersion;
    QString gatewayHost;
    int gatewayPort;
    QString ip;
    QString deviceXmlPath;
    int port;
    QString uri;
};

struct DeviceEntry {
    QString id;
    QString uri;
};

class DeviceManager {
public:
    DeviceManager();

    bool loadDeviceListFromXml(
        const QString& xmlPath,
        QList<DeviceEntry>& deviceList,
        QMap<QString, DeviceInfo>& deviceInfoMap,
        QString& errorMessage
        );

    QMap<QString, QList<DeviceEntry>> groupAndSortDevices(const QList<DeviceEntry>& devices);

    ClickableRectItem* createDeviceBlock(const QString& deviceId, int posX, int posY,
                                         QMap<QString, QPointF>& defaultPositions,
                                         QMap<QString, QPointF>& initialPositions);

    void saveDevicePositions(const QMap<QString, ClickableRectItem*>& deviceItems);

    DeviceInfo parseUri(const QString& uri);
};

#endif // DEVICEMANAGER_H
