#ifndef DRAGGABLEVIEW_H
#define DRAGGABLEVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPointF>
#include <QWheelEvent>
#include <QScrollBar>


class DraggableView : public QGraphicsView
{
    Q_OBJECT

public:
    DraggableView(QGraphicsScene *scene);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;


private:
    bool isDragging;
    QPoint lastPos;
    const qreal scaleFactor;
};

#endif // DRAGGABLEVIEW_H
