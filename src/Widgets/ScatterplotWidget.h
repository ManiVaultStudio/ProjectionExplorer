#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

class ScatterplotWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    ScatterplotWidget();
    ~ScatterplotWidget();

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();
};
