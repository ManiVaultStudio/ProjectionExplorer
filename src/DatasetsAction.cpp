#include "DatasetsAction.h"
#include "ScatterplotPlugin.h"

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <ColorData/ColorData.h>

#include <QMenu>

using namespace mv;
using namespace mv::gui;

DatasetsAction::DatasetsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _positionDatasetPickerAction(this, "Position"),
    _colorDatasetPickerAction(this, "Color")
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("database"));
    setToolTip("Manage loaded datasets for position and color");
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_positionDatasetPickerAction);
    addAction(&_colorDatasetPickerAction);

    _positionDatasetPickerAction.setFilterFunction([](const mv::Dataset<DatasetImpl>& dataset) -> bool {
        return dataset->getDataType() == PointType;
    });

    _colorDatasetPickerAction.setFilterFunction([](const mv::Dataset<DatasetImpl>& dataset) -> bool {
        return dataset->getDataType() == PointType || dataset->getDataType() == ColorType || dataset->getDataType() == ClusterType;
    });

    auto scatterplotPlugin = dynamic_cast<ScatterplotPlugin*>(parent->parent());

    if (scatterplotPlugin == nullptr)
        return;

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, scatterplotPlugin](Dataset<DatasetImpl> pickedDataset) -> void {
        scatterplotPlugin->getPositionDataset() = pickedDataset;
    });

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });
    
    connect(&_colorDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, scatterplotPlugin](Dataset<DatasetImpl> pickedDataset) -> void {
        scatterplotPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(pickedDataset);
    });
    
    connect(&scatterplotPlugin->getSettingsAction().getColoringAction(), &ColoringAction::currentColorDatasetChanged, this, [this](Dataset<DatasetImpl> currentColorDataset) -> void {
        _colorDatasetPickerAction.setCurrentDataset(currentColorDataset);
    });
}

void DatasetsAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicDatasetsAction = dynamic_cast<DatasetsAction*>(publicAction);

    Q_ASSERT(publicDatasetsAction != nullptr);

    if (publicDatasetsAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_positionDatasetPickerAction, &publicDatasetsAction->getPositionDatasetPickerAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_colorDatasetPickerAction, &publicDatasetsAction->getColorDatasetPickerAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void DatasetsAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_positionDatasetPickerAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_colorDatasetPickerAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void DatasetsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _positionDatasetPickerAction.fromParentVariantMap(variantMap);
    _colorDatasetPickerAction.fromParentVariantMap(variantMap);
}

QVariantMap DatasetsAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _positionDatasetPickerAction.insertIntoVariantMap(variantMap);
    _colorDatasetPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}