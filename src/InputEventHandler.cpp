#include "InputEventHandler.h"

#include <QEvent>
#include <QMouseEvent>
#include <QThread>

using namespace mv;

void InputEventHandler::onEvent(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton)
        {
            _leftMouseButtonPressed = true;

            _mouseDragTimer = new QTimer(this);
            connect(_mouseDragTimer, &QTimer::timeout, this, [this]()
                {
                    emit mouseDragged(Vector2f(_currentMousePos.x(), _currentMousePos.y()));
                });
            _mouseDragTimer->start(20);
        }

        break;
    }
    case QEvent::MouseButtonRelease:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton)
        {
            _mouseDragTimer->stop();
            _leftMouseButtonPressed = false;
        }

        break;
    }
    case QEvent::MouseMove:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        // If the left mouse button isn't pressed then we don't care about the mouse moving
        if (!_leftMouseButtonPressed)
            break;

        _currentMousePos = mouseEvent->position();

        break;
    }
    }
}
