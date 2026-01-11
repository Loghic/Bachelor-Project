#include <QGraphicsScene>
#include <QPainterPath>
#include <QPainter>
#include "include/clickablerectitem.h"

ClickableRectItem::ClickableRectItem(const QString &deviceId, qreal width, qreal height)
    : QGraphicsPathItem(), m_deviceId(deviceId), m_width(width), m_height(height)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);

    // Create a rounded rectangle using QPainterPath
    QPainterPath path;
    path.addRoundedRect(0, 0, m_width, m_height, 10, 10);

    setPath(path);
    setPen(QPen(Qt::black));
    setBrush(Qt::lightGray);
}

void ClickableRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressPos = event->scenePos();
    QGraphicsPathItem::mousePressEvent(event); // Allow default move/select handling
}

void ClickableRectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->scenePos() - pressPos).manhattanLength() < 5) {
        emit deviceClicked(m_deviceId); // Emit only if it's not a drag
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

void ClickableRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!isSelected)
        setBrush(QColor("#d0d0ff")); // light blue-ish on hover
    QGraphicsPathItem::hoverEnterEvent(event);
}

void ClickableRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!isSelected)
        setBrush(Qt::lightGray);
    QGraphicsPathItem::hoverLeaveEvent(event);
}

void ClickableRectItem::setSelectedStyle(bool selected)
{
    isSelected = selected;
    if (selected) {
        setBrush(QColor("#5ca0ff")); // blue-ish selected color
    } else {
        setBrush(Qt::lightGray);
    }
    update();
}

void ClickableRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPathItem::paint(painter, option, widget);

    if (isSelected) {
        QPen selectionPen(QColor("#2e66b7"));  // Blue-ish selection color
        selectionPen.setWidth(3);
        painter->setPen(selectionPen);

        QPainterPath path;
        path.addRoundedRect(0, 0, m_width, m_height, 10, 10);
        painter->drawPath(path);
    }
}

