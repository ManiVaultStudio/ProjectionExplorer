#include "ScatterplotWidget.h"

#include <QPainter>

ScatterplotWidget::ScatterplotWidget()
{

}

void ScatterplotWidget::initializeGL()
{
    initializeOpenGLFunctions();
}

void ScatterplotWidget::resizeGL(int w, int h)
{

}

void ScatterplotWidget::paintGL()
{
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    QPainter painter;

    painter.begin(this);

    QBrush brush;
    brush.setColor(Qt::red);
    brush.setStyle(Qt::Dense3Pattern);

    painter.setBrush(brush);
    painter.fillRect(0, 0, 100, 100, brush);

    painter.end();
}

void ScatterplotWidget::cleanup()
{

}

ScatterplotWidget::~ScatterplotWidget()
{
    disconnect(QOpenGLWidget::context(), &QOpenGLContext::aboutToBeDestroyed, this, &ScatterplotWidget::cleanup);
    cleanup();
}
