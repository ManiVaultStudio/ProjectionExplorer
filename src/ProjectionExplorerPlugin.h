#pragma once

#include <ViewPlugin.h>

#include "InputEventHandler.h"
#include "Lens.h"

#include "Widgets/ScatterplotWidget.h"
#include "Explanation/ExplanationModel.h"
#include "Actions/SettingsAction.h"

#include <Dataset.h>
#include <PointData/PointData.h>
#include <widgets/DropWidget.h>

/** All plugin related classes are in the ManiVault plugin namespace */
using namespace mv::plugin;

/** Drop widget used in this plugin is located in the ManiVault gui namespace */
using namespace mv::gui;

/** Dataset reference used in this plugin is located in the ManiVault util namespace */
using namespace mv::util;

/**
 * Projection Explorer
 *
 * This plugin implements the explanatory tools described in the thesis 
 * Interactive Explanation of High-dimensional Data Projects, Thijssen et al. 2022
 * and allows for interactive explanation of dimensionality reduction embeddings
 * in terms of the dimensions of the dataset.
 *
 * @authors J. Thijssen
 */
class ProjectionExplorerPlugin : public ViewPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param factory Pointer to the plugin factory
     */
    ProjectionExplorerPlugin(const PluginFactory* factory);

    /** Destructor */
    ~ProjectionExplorerPlugin() override = default;
    
    /** This function is called by the core after the view plugin has been created */
    void init() override;

private:
    void initializeDropWidget();

    /** Invoked when the selection of the projection dataset changes */
    void onProjectionSelectionChanged();
    /** Invoked when the left-mouse button is pressed and the cursor moved */
    void onMouseDragged(mv::Vector2f cursorPos);

    bool eventFilter(QObject* target, QEvent* event) Q_DECL_OVERRIDE;

protected:
    DropWidget*             _dropWidget;                /** Widget for drag and drop behavior */

    // Widgets
    ScatterplotWidget*      _scatterplotWidget;         /** Widget for plotting the projection points */
    SettingsAction          _settingsAction;

    // Data
    mv::Dataset<Points>     _projectionDataset;         /** Points smart pointer */
    std::vector<uint32_t>   _localToGlobalIndices;

    // Explanation
    Explanation::Model      _explanationModel;

    // Events
    InputEventHandler       _inputEventHandler;         /** Handles mouse and keyboard events */
};

/**
 * Plugin factory class
 *
 * Note: Factory does not need to be altered (merely responsible for generating new plugins when requested)
 */
class ProjectionExplorerPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.uu.ProjectionExplorer"
                      FILE  "ProjectionExplorerPlugin.json")

public:

    /** Default constructor */
    ProjectionExplorerPluginFactory() {}

    /** Destructor */
    ~ProjectionExplorerPluginFactory() override {}
    
    /** Creates an instance of the example view plugin */
    ViewPlugin* produce() override;

    /** Returns the data types that are supported by the example view plugin */
    mv::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
