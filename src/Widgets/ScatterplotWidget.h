#pragma once

#include "OpenGLWidget.h"

#include "Explanation/ExplanationModel.h"

#include <renderers/PointRenderer.h>
#include <graphics/Bounds.h>

#include <QTimer>

namespace mv
{
    class Vector2f;
}
class Lens;

class ScatterplotWidget : public mv::gui::OpenGLWidget
{
    Q_OBJECT

public:
    ScatterplotWidget(Explanation::Model& explanationModel);
    ~ScatterplotWidget();

public:
    mv::Bounds getBounds() { return _dataBounds; }
    void setData(const std::vector<mv::Vector2f>& data);
    void setSelection(const std::vector<char>& selection, const std::int32_t& numSelectedPoints);

protected:
    void onWidgetInitialized()          override;
    void onWidgetResized(int w, int h)  override;
    void onWidgetRendered()             override;
    void onWidgetCleanup()              override;

private:
    Explanation::Model&     _explanationModel;      /** Reference to explanation model */

    mv::gui::PointRenderer  _pointRenderer;         /** Scatter plot renderer */

    mv::Bounds              _dataBounds;            /** Stored data bounds for lens computation */

};
