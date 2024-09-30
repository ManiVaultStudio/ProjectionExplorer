#include "UserInterface.h"

#include "ProjectionExplorerPlugin.h"

#include <PointData/PointData.h>
#include <DatasetsMimeData.h>

#include <QMimeData>

UserInterface::UserInterface(ProjectionExplorerPlugin* plugin, Explanation::Model& explanationModel) :
    _plugin(plugin),
    _layout(new QVBoxLayout()),
    _centralWidget(new QWidget()),
    _dropWidget(nullptr),
    _settingsAction(_centralWidget, "SettingsAction"),
    _scatterplotWidget(new ScatterplotWidget(explanationModel)),
    _explanationWidget(new ExplanationWidget(explanationModel))
{
    // This line is mandatory if drag and drop behavior is required
    _scatterplotWidget->setAcceptDrops(true);

    _centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _centralWidget->setContentsMargins(0, 0, 0, 0);

    auto centralLayout = new QHBoxLayout();
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(_scatterplotWidget);
    centralLayout->addWidget(_explanationWidget);
    _centralWidget->setLayout(centralLayout);
    _layout->addWidget(_centralWidget);
}

void UserInterface::init()
{
    // Initialize the drop regions
    initializeDropWidget();
}

void UserInterface::initializeDropWidget()
{
    // Instantiate new drop widget
    _dropWidget = new DropWidget(_scatterplotWidget);

    // Set the drop indicator widget (the widget that indicates that the view is eligible for data dropping)
    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&_plugin->getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));
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

                if (!_plugin->getProjectionDataset().isValid())
                {

                    // Load as point positions when no dataset is currently loaded
                    dropRegions << new DropWidget::DropRegion(_plugin, "Point position", description, "map-marker-alt", true, [this, candidateDataset]()
                        {
                            _plugin->getProjectionDataset() = candidateDataset;
                            _plugin->onNewProjectionLoaded();
                        });
                }
                else
                {
                    if (_plugin->getProjectionDataset() != candidateDataset && candidateDataset->getNumDimensions() >= 2)
                    {

                        // The number of points is equal, so offer the option to replace the existing points dataset
                        dropRegions << new DropWidget::DropRegion(_plugin, "Point position", description, "map-marker-alt", true, [this, candidateDataset]()
                            {
                                _plugin->getProjectionDataset() = candidateDataset;
                                _plugin->onNewProjectionLoaded();
                            });
                    }
                }
            }

            return dropRegions;
        });
}
