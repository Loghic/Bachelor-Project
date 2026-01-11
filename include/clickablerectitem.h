#ifndef CLICKABLERECTITEM_H
#define CLICKABLERECTITEM_H

#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QPointF>

class ClickableRectItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    ClickableRectItem(const QString &deviceId, qreal width = 100, qreal height = 60);

    QString deviceId() const { return m_deviceId; }
    void setSelectedStyle(bool selected);

    qreal getWidth() const { return m_width; }
    qreal getHeight() const { return m_height; }

signals:
    void deviceClicked(const QString &deviceId);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;


private:
    QString m_deviceId;
    bool isSelected = false;
    qreal m_width;
    qreal m_height;
    QPointF pressPos;
};

#endif // CLICKABLERECTITEM_H
