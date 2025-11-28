#pragma once

#include <actions/GroupAction.h>
#include <actions/TriggerAction.h>
#include <actions/IntegralAction.h>
#include <actions/DecimalAction.h>
#include <actions/OptionAction.h>
#include <actions/DatasetPickerAction.h>

using namespace mv::gui;

class ProjectionExplorerPlugin;

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
    DatasetPickerAction& getCurrentDatasetAction() { return _currentDatasetAction; }
    TriggerAction& getGenerateClustersAction() { return _generateClustersAction; }
    IntegralAction& getLensRadiusAction() { return _lensRadiusAction; }
    DecimalAction& getGlobalRadiusAction() { return _globalRadiusAction; }
    OptionAction& getColorOptionAction() { return _colorOptionAction; }

protected:
    ProjectionExplorerPlugin*   _plugin;        /** Pointer to plugin class */

    DatasetPickerAction         _currentDatasetAction;
    IntegralAction              _lensRadiusAction;
    TriggerAction               _generateClustersAction;
    DecimalAction               _globalRadiusAction;
    OptionAction                _colorOptionAction;
};
