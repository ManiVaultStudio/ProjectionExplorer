#pragma once

#include <ViewPlugin.h>

#include "util/PixelSelectionTool.h"
#include "Explanation/ExplanationModel.h"
#include "ExplanationWidget.h"

#include "Common.h"

#include "SettingsAction.h"

#include <ClusterData/ClusterData.h>
#include <actions/HorizontalToolbarAction.h>

using namespace mv::plugin;
using namespace mv::util;

class Points;

class ScatterplotWidget;

namespace mv
{
    class CoreInterface;
    class Vector2f;

    namespace gui {
        class DropWidget;
    }
}

class ScatterplotPlugin : public ViewPlugin
{
    Q_OBJECT
    
public:
    ScatterplotPlugin(const PluginFactory* factory);
    ~ScatterplotPlugin() override;

    void init() override;

    void onDataEvent(mv::DatasetEvent* dataEvent);

    /**
     * Load one (or more datasets in the view)
     * @param datasets Dataset(s) to load
     */
    void loadData(const Datasets& datasets) override;

    /** Get number of points in the position dataset */
    std::uint32_t getNumberOfPoints() const;

    void colorPointsByRanking();

private: // Initialization
    void initializeDropWidget();

protected slots:
    void neighbourhoodRadiusValueChanged(int value);
    void neighbourhoodRadiusSliderPressed();
    void neighbourhoodRadiusSliderReleased();
    void explanationMetricChanged();
    void datasetDimensionsChanged();

public:
    void createSubset(const bool& fromSourceData = false, const QString& name = "");

public: // Dimension picking
    void setXDimension(const std::int32_t& dimensionIndex);
    void setYDimension(const std::int32_t& dimensionIndex);

protected: // Data loading

    /** Invoked when the position points dataset changes */
    void positionDatasetChanged();

public: // Point colors

    /**
     * Load color from points dataset
     * @param points Smart pointer to points dataset
     * @param dimensionIndex Index of the dimension to load
     */
    void loadColors(const Dataset<Points>& points, const std::uint32_t& dimensionIndex);

    /**
     * Load color from clusters dataset
     * @param clusters Smart pointer to clusters dataset
     */
    void loadColors(const Dataset<Clusters>& clusters);

public: // Miscellaneous

    /** Get smart pointer to points dataset for point position */
    Dataset<Points>& getPositionDataset();

    /** Get smart pointer to source of the points dataset for point position (if any) */
    Dataset<Points>& getPositionSourceDataset();

    /** Use the pixel selection tool to select data points */
    void selectPoints();

public:

    /** Get reference to the scatter plot widget */
    ScatterplotWidget& getScatterplotWidget();

    SettingsAction& getSettingsAction() { return _settingsAction; }

private:
    void updateData();
    void calculatePositions(const Points& points);
    void updateSelection();
    void computeLensSelection(std::vector<std::uint32_t>& targetSelectionIndices);

    bool eventFilter(QObject* target, QEvent* event);

public: // Serialization

    /**
     * Load plugin from variant map
     * @param Variant map representation of the plugin
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    QVariantMap toVariantMap() const override;

private:
    Dataset<Points>                 _positionDataset;           /** Smart pointer to points dataset for point position */
    Dataset<Points>                 _positionSourceDataset;     /** Smart pointer to source of the points dataset for point position (if any) */
    std::vector<mv::Vector2f>     _positions;                 /** Point positions */
    unsigned int                    _numPoints;                 /** Number of point positions */
    
    
protected:
    ScatterplotWidget*          _scatterPlotWidget;
    mv::gui::DropWidget*      _dropWidget;
    SettingsAction              _settingsAction;

    ExplanationModel            _explanationModel;
    ExplanationWidget*          _explanationWidget;

    HorizontalToolbarAction    _primaryToolbarAction;
    HorizontalToolbarAction    _secondaryToolbarAction;

    float _selectionRadius;

    bool _lockSelection;
    std::vector<unsigned int>   _lockedSelection;

    QPoint _lastMousePos;

    bool _mousePressed = false;
};

// =============================================================================
// Factory
// =============================================================================

class ScatterplotPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.uu.ProjectionExplorer"
                      FILE  "ProjectionExplorer.json")
    
public:
    ScatterplotPluginFactory(void) {}
    ~ScatterplotPluginFactory(void) override {}

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    ViewPlugin* produce() override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
