#pragma once

#include "RenderModeAction.h"
#include "PlotAction.h"
#include "PositionAction.h"
#include "ColoringAction.h"
#include "SubsetAction.h"
#include "SelectionAction.h"
#include "MiscellaneousAction.h"
#include "ExportAction.h"
#include "DatasetsAction.h"

using namespace mv::gui;

class ScatterplotPlugin;

class SettingsAction : public GroupAction
{
public:
    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SettingsAction(QObject* parent, const QString& title);

    /**
     * Get action context menu
     * @return Pointer to menu
     */
    QMenu* getContextMenu();

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

public: // Action getters

    RenderModeAction& getRenderModeAction() { return _renderModeAction; }
    PositionAction& getPositionAction() { return _positionAction; }
    ColoringAction& getColoringAction() { return _coloringAction; }
    SubsetAction& getSubsetAction() { return _subsetAction; }
    SelectionAction& getSelectionAction() { return _selectionAction; }
    PlotAction& getPlotAction() { return _plotAction; }
    ExportAction& getExportAction() { return _exportAction; }
    MiscellaneousAction& getMiscellaneousAction() { return _miscellaneousAction; }
    DatasetsAction& getDatasetsAction() { return _datasetsAction; }

protected:
    ScatterplotPlugin* _scatterplotPlugin;         /** Pointer to scatter plot plugin */

    RenderModeAction            _renderModeAction;
    PositionAction              _positionAction;
    ColoringAction              _coloringAction;
    SubsetAction                _subsetAction;
    SelectionAction             _selectionAction;
    PlotAction                  _plotAction;
    ExportAction                _exportAction;
    MiscellaneousAction         _miscellaneousAction;
    DatasetsAction              _datasetsAction;            /** Action for picking dataset(s) */
};
