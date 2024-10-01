#include "SettingsAction.h"

#include "ProjectionExplorerPlugin.h"

#include "PointData/PointData.h"

#include <QMenu>

using namespace mv::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _plugin(dynamic_cast<ProjectionExplorerPlugin*>(parent)),
    _generateClustersAction(this, "Generate Clusters")
{
    setConnectionPermissionsToForceNone();

    connect(&_generateClustersAction, &TriggerAction::triggered, this, [this]() {
        _plugin->generateClusterDataset();
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

}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    return variantMap;
}
