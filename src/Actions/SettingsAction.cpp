#include "SettingsAction.h"

#include "PointData/PointData.h"

#include <QMenu>

using namespace mv::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title)
{
    setConnectionPermissionsToForceNone();
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
