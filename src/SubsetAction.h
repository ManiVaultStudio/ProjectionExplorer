#pragma once

#include <actions/VerticalGroupAction.h>
#include <actions/StringAction.h>
#include <actions/TriggerAction.h>
#include <actions/OptionAction.h>

using namespace mv::gui;

class ScatterplotPlugin;

/**
 * Subset action class
 *
 * Action class for creating a subset
 *
 * @author Thomas Kroes
 */
class SubsetAction : public VerticalGroupAction
{
    Q_OBJECT

public:
    
    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SubsetAction(QObject* parent, const QString& title);

    /**
     * Initialize the selection action with \p scatterplotPlugin
     * @param scatterplotPlugin Pointer to scatterplot plugin
     */
    void initialize(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Get action context menu
     * @return Pointer to menu
     */
    QMenu* getContextMenu();

public: // Serialization

    /**
     * Load selection action from variant
     * @param Variant representation of the selection action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save selection action to variant
     * @return Variant representation of the selection action
     */
    QVariantMap toVariantMap() const override;

private:
    ScatterplotPlugin*  _scatterplotPlugin;     /** Pointer to scatter plot plugin */
    StringAction        _subsetNameAction;      /** String action for configuring the subset name */
    OptionAction        _sourceDataAction;      /** Option action for picking the source dataset */
    TriggerAction       _createSubsetAction;    /** Triggers the subset creation process */
};

Q_DECLARE_METATYPE(SubsetAction)

inline const auto subsetActionMetaTypeId = qRegisterMetaType<SubsetAction*>("SubsetAction");