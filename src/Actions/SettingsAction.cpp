#include "SettingsAction.h"

#include "ProjectionExplorerPlugin.h"

#include "PointData/PointData.h"

#include <QMenu>

using namespace mv::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _plugin(dynamic_cast<ProjectionExplorerPlugin*>(parent)),
    _currentDatasetAction(this, "CurrentDataset"),
    _lensRadiusAction(this, "Lens Radius", 1, 100, 30),
    _generateClustersAction(this, "Generate Clusters"),
    _globalRadiusAction(this, "Global Radius", 0.01, 0.2, 0.12, 2),
    _colorOptionAction(this, "Number of colors", { "20", "40", "60", "100" }, "20")
{
    setConnectionPermissionsToForceNone();

    connect(&_currentDatasetAction, &DatasetPickerAction::datasetPicked, this, [this]() {
        _plugin->onNewProjectionLoaded();
    });

    connect(&_generateClustersAction, &TriggerAction::triggered, this, [this]() {
        _plugin->generateClusterDataset();
    });

    connect(&_lensRadiusAction, &IntegralAction::valueChanged, this, [this](int32_t value) {
        _plugin->getExplanationModel().getLens().radius = value;
    });

    connect(&_globalRadiusAction, &DecimalAction::valueChanged, this, [this](float value) {
        _plugin->getExplanationModel().getValueMethod().setGlobalNeighbourhoodRadius(value);
    });

    connect(&_colorOptionAction, &OptionAction::currentIndexChanged, this, [this](const int32_t& currentIndex) {
        _plugin->getExplanationModel().getColorMapping().switchPalettes(currentIndex);
    });
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu();

    return menu;
}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _currentDatasetAction.fromParentVariantMap(variantMap);

    // Load position dataset
    auto positionDataset = _currentDatasetAction.getCurrentDataset();

    if (positionDataset.isValid())
    {
        mv::Dataset pickedDataset = mv::data().getDataset(positionDataset.getDatasetId());
        _plugin->getProjectionDataset() = pickedDataset;
        _plugin->onNewProjectionLoaded();
    }
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _currentDatasetAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
