#include "UserInterface.h"

#include "ProjectionExplorerPlugin.h"

#include <PointData/PointData.h>
#include <DatasetsMimeData.h>

#include <QMimeData>

UserInterface::UserInterface(ProjectionExplorerPlugin* plugin, Explanation::Model& explanationModel) :
    _plugin(plugin),
    _layout(new QVBoxLayout()),
    _centralWidget(new QWidget()),
    _primaryToolbarAction(plugin, "PrimaryToolbar"),
    _dropWidget(nullptr),
    _settingsAction(_plugin, "SettingsAction"),
    _scatterplotWidget(new ScatterplotWidget(explanationModel)),
    _explanationWidget(new ExplanationWidget(explanationModel))
{
    // This line is mandatory if drag and drop behavior is required
    _centralWidget->setAcceptDrops(true);

    _centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _centralWidget->setContentsMargins(0, 0, 0, 0);

    auto centralLayout = new QHBoxLayout();
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    //centralLayout->addWidget(_scatterplotWidget);
    centralLayout->addWidget(_explanationWidget);
    _centralWidget->setLayout(centralLayout);
    _layout->addWidget(_primaryToolbarAction.createWidget(&_plugin->getWidget()));
    _layout->addWidget(_centralWidget);
}

void UserInterface::init()
{
    // Initialize the drop regions
    initializeDropWidget();

    GroupAction* groupAction = new GroupAction(_plugin, "Numeric Group", true);
    //groupAction->addAction(&_settingsAction.getLensRadiusAction(), 3);
    //groupAction->addAction(&_settingsAction.getGlobalRadiusAction(), 3);
    groupAction->addAction(&_settingsAction.getColorOptionAction(), 1);

    _primaryToolbarAction.addAction(groupAction, 3, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getGenerateClustersAction(), 4, GroupAction::Horizontal);
}

void UserInterface::initializeDropWidget()
{
    // Instantiate new drop widget
    _dropWidget = new DropWidget(_centralWidget);

    // Set the drop indicator widget (the widget that indicates that the view is eligible for data dropping)
    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&_plugin->getWidget(), "No data loaded", "Drag an embedding of dataset from the data hierarchy and drop it here to explore its features..."));
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions
        {
            // A drop widget can contain zero or more drop regions
            DropWidget::DropRegions dropRegions;

            const auto datasetsMimeData = dynamic_cast<const mv::DatasetsMimeData*>(mimeData);

            if (datasetsMimeData == nullptr)
                return dropRegions;

            if (datasetsMimeData->getDatasets().count() > 1)
                return dropRegions;

            const auto dataset = datasetsMimeData->getDatasets().first();
            const auto datasetGuiName = dataset->text();
            const auto datasetId = dataset->getId();
            const auto dataType = dataset->getDataType();
            const auto dataTypes = mv::DataTypes({ PointType });

            // Check if the data type can be dropped
            if (!dataTypes.contains(dataType))
                dropRegions << new DropWidget::DropRegion(_plugin, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);

            // Points dataset is about to be dropped
            if (dataType == PointType)
            {
                // Get points dataset from the core
                auto candidateDataset = mv::data().getDataset<Points>(datasetId);

                // Establish drop region description
                const auto description = QString("Visualize %1 explanations").arg(datasetGuiName);

                // Check if data is an embedding, otherwise tell why it can't be dropped
                if (!candidateDataset->isDerivedData())
                {
                    dropRegions << new DropWidget::DropRegion(_plugin, "Incompatible data", "This data is not a projection/embedding.", "exclamation-circle", false);
                }
                else if (_plugin->getProjectionDataset() == candidateDataset)
                {
                    dropRegions << new DropWidget::DropRegion(_plugin, "Incompatible data", "This data is already loaded.", "exclamation-circle", false);
                }
                else
                {
                    dropRegions << new DropWidget::DropRegion(_plugin, "Point position", description, "map-marker-alt", true, [this, candidateDataset]()
                    {
                        _plugin->getProjectionDataset() = candidateDataset;
                        _plugin->onNewProjectionLoaded();
                    });
                }
            }

            return dropRegions;
        });
}
