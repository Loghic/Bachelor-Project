#include "include/devicemanager.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QGraphicsTextItem>

DeviceManager::DeviceManager() {}

bool DeviceManager::loadDeviceListFromXml(const QString& xmlPath,
                                          QList<DeviceEntry>& deviceList,
                                          QMap<QString, DeviceInfo>& deviceInfoMap,
                                          QString& errorMessage) {
    QFile xmlFile(xmlPath);
    if (!xmlFile.open(QIODevice::ReadOnly)) {
        errorMessage = "Unable to open the XML file.";
        return false;
    }

    QXmlStreamReader xmlReader(&xmlFile);

    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.isStartElement() && xmlReader.name() == QStringLiteral("connection")) {
            QString id = xmlReader.attributes().value("id").toString();
            QString uri = xmlReader.attributes().value("uri").toString();
            QString addressTable = xmlReader.attributes().value("address_table").toString();

            if (!id.isEmpty() && !uri.isEmpty()) {
                deviceList.append({id, uri});
                DeviceInfo info = parseUri(uri);
                info.deviceXmlPath = addressTable;
                deviceInfoMap[id] = info;
            }
        }
    }

    xmlFile.close();

    if (xmlReader.hasError()) {
        errorMessage = "Error reading the XML file.";
        return false;
    }

    return true;
}

QMap<QString, QList<DeviceEntry>> DeviceManager::groupAndSortDevices(const QList<DeviceEntry>& devices) {
    QMap<QString, QList<DeviceEntry>> grouped;

    for (const auto& d : devices) {
        QString key = d.id.left(1).toUpper();
        grouped[key].append(d);
    }

    for (auto& list : grouped) {
        std::sort(list.begin(), list.end(), [](const DeviceEntry& a, const DeviceEntry& b) {
            return a.id.toLower() < b.id.toLower();
        });
    }

    return grouped;
}

ClickableRectItem* DeviceManager::createDeviceBlock(const QString& deviceId, int posX, int posY,
                                                    QMap<QString, QPointF>& defaultPositions,
                                                    QMap<QString, QPointF>& initialPositions) {
    ClickableRectItem* deviceBlock = new ClickableRectItem(deviceId, 100, 60);

    QSettings settings("CERN", "Register GUI");
    settings.beginGroup("DevicePositions");
    qreal savedX = settings.value(deviceId + "/x", posX).toDouble();
    qreal savedY = settings.value(deviceId + "/y", posY).toDouble();
    settings.endGroup();

    QPointF actualPos(savedX, savedY);
    deviceBlock->setPos(actualPos);

    if (!initialPositions.contains(deviceId)) initialPositions[deviceId] = actualPos;
    if (!defaultPositions.contains(deviceId)) defaultPositions[deviceId] = QPointF(posX, posY);

    QGraphicsTextItem* deviceName = new QGraphicsTextItem(deviceId, deviceBlock);
    deviceName->setDefaultTextColor(Qt::black);
    QRectF textRect = deviceName->boundingRect();
    deviceName->setPos((100 - textRect.width()) / 2, (60 - textRect.height()) / 2);

    return deviceBlock;
}

void DeviceManager::saveDevicePositions(const QMap<QString, ClickableRectItem*>& deviceItems) {
    QSettings settings("CERN", "Register GUI");
    settings.beginGroup("DevicePositions");
    settings.remove("");

    for (auto it = deviceItems.begin(); it != deviceItems.end(); ++it) {
        QPointF pos = it.value()->pos();
        settings.setValue(it.key() + "/x", pos.x());
        settings.setValue(it.key() + "/y", pos.y());
    }

    settings.endGroup();
}

DeviceInfo DeviceManager::parseUri(const QString& uri) {
    DeviceInfo info;

    QRegularExpression newRe(R"(^(.*?)(?:-(\d+\.\d+))?://([^:]+):(\d+)\?target=([\d\.]+):(\d+))");
    QRegularExpressionMatch newMatch = newRe.match(uri);
    info.uri = uri;
    if (newMatch.hasMatch()) {
        info.connectionType = newMatch.captured(1);
        info.connectionVersion = newMatch.captured(2);
        info.gatewayHost = newMatch.captured(3);
        info.gatewayPort = newMatch.captured(4).toInt();
        info.ip = newMatch.captured(5);
        info.port = newMatch.captured(6).toInt();
        return info;
    }

    QRegularExpression oldRe(R"(^([\w\-]+)(?:-(\d+\.\d+))?://([\d\.]+):(\d+))");
    QRegularExpressionMatch oldMatch = oldRe.match(uri);
    if (oldMatch.hasMatch()) {
        info.connectionType = oldMatch.captured(1);
        info.connectionVersion = oldMatch.captured(2);
        info.ip = oldMatch.captured(3);
        info.port = oldMatch.captured(4).toInt();
        return info;
    }

    qWarning() << "Failed to parse URI:" << uri;
    return info;
}
