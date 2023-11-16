#include "SubsetAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <PointData/PointData.h>

#include <Application.h>

#include <QMenu>

using namespace mv;
using namespace mv::gui;

SubsetAction::SubsetAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _subsetNameAction(this, "Subset name"),
    _sourceDataAction(this, "Source data"),
    _createSubsetAction(this, "Create subset")
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("crop"));
    setConnectionPermissionsToForceNone(true);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_subsetNameAction);
    addAction(&_sourceDataAction);
    addAction(&_createSubsetAction);

    _subsetNameAction.setToolTip("Name of the subset");
    _createSubsetAction.setToolTip("Create subset from selected data points");

    const auto updateReadOnly = [this]() -> void {
        _createSubsetAction.setEnabled(!_subsetNameAction.getString().isEmpty());
    };

    updateReadOnly();

    connect(&_subsetNameAction, &StringAction::stringChanged, this, updateReadOnly);
}

void SubsetAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    connect(&_createSubsetAction, &QAction::triggered, this, [this]() {
        _scatterplotPlugin->createSubset(_sourceDataAction.getCurrentIndex() == 1, _subsetNameAction.getString());
    });

    const auto onCurrentDatasetChanged = [this]() -> void {
        if (!_scatterplotPlugin->getPositionDataset().isValid())
            return;

        const auto datasetGuiName = _scatterplotPlugin->getPositionDataset()->text();

        QStringList sourceDataOptions;

        if (!datasetGuiName.isEmpty()) {
            const auto sourceDatasetGuiName = _scatterplotPlugin->getPositionDataset()->getSourceDataset<Points>()->text();

            sourceDataOptions << QString("From: %1").arg(datasetGuiName);

            if (sourceDatasetGuiName != datasetGuiName)
                sourceDataOptions << QString("From: %1 (source data)").arg(sourceDatasetGuiName);
        }

        _sourceDataAction.setOptions(sourceDataOptions);
        _sourceDataAction.setEnabled(sourceDataOptions.count() >= 2);
    };

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, onCurrentDatasetChanged);

    onCurrentDatasetChanged();

    const auto updateReadOnly = [this]() {
        const auto positionDataset          = _scatterplotPlugin->getPositionDataset();
        const auto numberOfSelectedPoints   = positionDataset.isValid() ? positionDataset->getSelectionSize() : 0;
        const auto hasSelection             = numberOfSelectedPoints >= 1;

        setEnabled(_scatterplotPlugin->getScatterplotWidget().getRenderMode() == ScatterplotWidget::SCATTERPLOT && hasSelection);
    };

    updateReadOnly();

    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, updateReadOnly);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataSelectionChanged, this, updateReadOnly);
}

QMenu* SubsetAction::getContextMenu()
{
    auto menu = new QMenu("Subset");

    menu->addAction(&_createSubsetAction);
    menu->addAction(&_sourceDataAction);

    return menu;
}

void SubsetAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _subsetNameAction.fromParentVariantMap(variantMap);
}

QVariantMap SubsetAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _subsetNameAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
