#include "ScatterplotWidget.h"

#include <graphics/Vector2f.h>
#include <graphics/Bounds.h>

#include <util/Exception.h>

#include <QPainter>

using namespace mv;

namespace
{
    /** Find the rectangular bounds of the 2D dataset provided */
    Bounds getDataBounds(const std::vector<Vector2f>& data)
    {
        Bounds bounds = Bounds::Max;

        for (const Vector2f& point : data)
        {
            bounds.setLeft(std::min(point.x, bounds.getLeft()));
            bounds.setRight(std::max(point.x, bounds.getRight()));
            bounds.setBottom(std::min(point.y, bounds.getBottom()));
            bounds.setTop(std::max(point.y, bounds.getTop()));
        }

        return bounds;
    }
}

ScatterplotWidget::ScatterplotWidget() :
    _pointRenderer()
{

}

ScatterplotWidget::~ScatterplotWidget()
{
}

void ScatterplotWidget::setData(const std::vector<Vector2f>& data)
{
    Bounds dataBounds = getDataBounds(data);
    dataBounds.expand(0.1f);

    _pointRenderer.setBounds(dataBounds);

    _pointRenderer.setData(data);

    update();
}

void ScatterplotWidget::onWidgetInitialized()
{
    _pointRenderer.init();
}

void ScatterplotWidget::onWidgetResized(int w, int h)
{
    _pointRenderer.resize(QSize(w, h));
}

void ScatterplotWidget::onWidgetRendered()
{
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    try {
        QPainter painter;

        // Begin mixed OpenGL/native painting
        if (!painter.begin(this))
            throw std::runtime_error("Unable to begin painting");

        painter.beginNativePainting();
        {
            _pointRenderer.render();
        }
        painter.endNativePainting();

        QBrush brush;
        brush.setColor(Qt::red);
        brush.setStyle(Qt::Dense3Pattern);

        painter.setBrush(brush);
        painter.fillRect(0, 0, 100, 100, brush);

        painter.end();
    }
    catch (std::exception& e)
    {
        util::exceptionMessageBox("Rendering failed", e);
    }
    catch (...) {
        util::exceptionMessageBox("Rendering failed");
    }
}

void ScatterplotWidget::onWidgetCleanup()
{
    _pointRenderer.destroy();
}
