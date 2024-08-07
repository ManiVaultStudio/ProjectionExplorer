#include "ScatterplotWidget.h"

#include <graphics/Vector2f.h>
#include <util/Exception.h>
#include <util/Timer.h>

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

ScatterplotWidget::ScatterplotWidget(Explanation::Model& explanationModel) :
    _explanationModel(explanationModel),
    _pointRenderer()
{

}

ScatterplotWidget::~ScatterplotWidget()
{
}

void ScatterplotWidget::setData(const std::vector<Vector2f>& data)
{
    _dataBounds = getDataBounds(data);

    _dataBounds.ensureMinimumSize(1e-07f, 1e-07f);
    _dataBounds.makeSquare();
    _dataBounds.expand(0.1f);

    _pointRenderer.setBounds(_dataBounds);
    _pointRenderer.setPointSize(1.0f);
    _pointRenderer.setData(data);

    update();
}

void ScatterplotWidget::setSelection(const std::vector<char>& highlights, const std::int32_t& numSelectedPoints)
{
    _pointRenderer.setHighlights(highlights, numSelectedPoints);
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
    Timer t("Render");
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    //qDebug() << "Repaint Widget";
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

        // Draw lens
        Lens& lens = _explanationModel.getLens();
        // Draw local neighbourhood circle
        QPen pen;
        pen.setColor(QColor(255, 0, 0, 255));
        pen.setStyle(Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(lens.position.x - lens.radius, lens.position.y - lens.radius, lens.radius * 2, lens.radius * 2);

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
