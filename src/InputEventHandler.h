#pragma once

#include <graphics/Vector2f.h>

#include <QObject>
#include <QPointF>
#include <QTimer>

class QEvent;

class InputEventHandler : public QObject
{
    Q_OBJECT
public:
    void onEvent(QEvent* event);
    QPointF getMousePos() { return mousePos; }

signals:
    void mouseDragged(mv::Vector2f cursorPos);

private:
    bool _leftMouseButtonPressed = false;
    QPointF mousePos;
    QTimer* _mouseDragTimer;
};
