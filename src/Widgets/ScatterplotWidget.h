#pragma once

#include "OpenGLWidget.h"

#include "renderers/PointRenderer.h"

namespace mv
{
    class Vector2f;
}

class ScatterplotWidget : public mv::gui::OpenGLWidget
{
    Q_OBJECT

public:
    ScatterplotWidget();
    ~ScatterplotWidget();

public:
    void setData(const std::vector<mv::Vector2f>& data);

protected:
    void onWidgetInitialized()          override;
    void onWidgetResized(int w, int h)  override;
    void onWidgetRendered()             override;
    void onWidgetCleanup()              override;

private:
    mv::gui::PointRenderer           _pointRenderer;        /** Scatter plot renderer */
};
