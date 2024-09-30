#pragma once

#include "Explanation/ExplanationModel.h"

#include "Widgets/ScatterplotWidget.h"
#include "Widgets/ExplanationWidget.h"
#include "Actions/SettingsAction.h"

#include <widgets/DropWidget.h>

#include <QVBoxLayout>

class ProjectionExplorerPlugin;

class UserInterface
{
public:
    UserInterface(ProjectionExplorerPlugin* plugin, Explanation::Model& explanationModel);

    QVBoxLayout* getLayout() { return _layout; }

    void init();

public:
    ScatterplotWidget* getScatterplotWidget() { return _scatterplotWidget; }
    ExplanationWidget* getExplanationWidget() { return _explanationWidget; }
    DropWidget* getDropWidget() { return _dropWidget; }

private:
    void initializeDropWidget();

private:
    ProjectionExplorerPlugin*   _plugin;

    QVBoxLayout*                _layout;

    // Widgets
    QWidget*                    _centralWidget;
    DropWidget*                 _dropWidget;                /** Widget for drag and drop behavior */
    ScatterplotWidget*          _scatterplotWidget;         /** Widget for plotting the projection points */
    ExplanationWidget*          _explanationWidget;         /** Widget for showing the local explanation histograms */
    SettingsAction              _settingsAction;
};
