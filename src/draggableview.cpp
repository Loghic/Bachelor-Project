#include "include/draggableview.h"

DraggableView::DraggableView(QGraphicsScene *scene)
    : QGraphicsView(scene), isDragging(false), scaleFactor(1.1)
{
    setDragMode(QGraphicsView::NoDrag);
}

void DraggableView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        isDragging = true;
        lastPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void DraggableView::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && (event->buttons() & Qt::RightButton)) {
        QPoint delta = event->pos() - lastPos;
        lastPos = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();

    }else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void DraggableView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && isDragging) {
        isDragging = false;
        unsetCursor();
        event->accept();
    }else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void DraggableView::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        // Zoom in
        scale(scaleFactor, scaleFactor);
    } else {
        // Zoom out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
    event->accept();
}
