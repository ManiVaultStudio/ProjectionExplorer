#include "SettingsAction.h"

#include "Application.h"
#include "ScatterplotPlugin.h"
#include "PointData/PointData.h"

#include <QMenu>

using namespace mv::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent)),
    _renderModeAction(this, "Render Mode"),
    _positionAction(this, "Position"),
    _coloringAction(this, "Coloring"),
    _subsetAction(this, "Subset"),
    _selectionAction(this, "Selection"),
    _plotAction(this, "Plot"),
    _exportAction(this, "Export to image/video"),
    _miscellaneousAction(this, "Miscellaneous"),
    _datasetsAction(this, "Datasets")
{
    setConnectionPermissionsToForceNone();

    _renderModeAction.initialize(_scatterplotPlugin);
    _plotAction.initialize(_scatterplotPlugin);
    _subsetAction.initialize(_scatterplotPlugin);
    _selectionAction.initialize(_scatterplotPlugin);
    _exportAction.initialize(_scatterplotPlugin);

    const auto updateEnabled = [this]() {
        setEnabled(_scatterplotPlugin->getPositionDataset().isValid());
    };

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateEnabled);

    updateEnabled();

    _exportAction.setIcon(mv::Application::getIconFont("FontAwesome").getIcon("camera"));
    _exportAction.setDefaultWidgetFlags(TriggerAction::Icon);
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu();

    menu->addMenu(_renderModeAction.getContextMenu());
    menu->addMenu(_plotAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_positionAction.getContextMenu());
    menu->addMenu(_coloringAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_subsetAction.getContextMenu());
    menu->addMenu(_selectionAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_miscellaneousAction.getContextMenu());

    return menu;
}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _plotAction.fromParentVariantMap(variantMap);
    _positionAction.fromParentVariantMap(variantMap);
    _coloringAction.fromParentVariantMap(variantMap);
    _renderModeAction.fromParentVariantMap(variantMap);
    _selectionAction.fromParentVariantMap(variantMap);
    _miscellaneousAction.fromParentVariantMap(variantMap);
    _datasetsAction.fromParentVariantMap(variantMap);
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _datasetsAction.insertIntoVariantMap(variantMap);
    _renderModeAction.insertIntoVariantMap(variantMap);
    _plotAction.insertIntoVariantMap(variantMap);
    _positionAction.insertIntoVariantMap(variantMap);
    _coloringAction.insertIntoVariantMap(variantMap);
    _selectionAction.insertIntoVariantMap(variantMap);
    _miscellaneousAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
