#include "ProjectionExplorerPlugin.h"

#include <event/Event.h>

#include <DatasetsMimeData.h>

#include <QDebug>
#include <QMimeData>

Q_PLUGIN_METADATA(IID "nl.uu.ProjectionExplorer")

using namespace mv;

ProjectionExplorerPlugin::ProjectionExplorerPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _projectionDataset(nullptr),
    _settingsAction(this, "SettingsAction"),
    _scatterplotWidget(new ScatterplotWidget())
{
    // This line is mandatory if drag and drop behavior is required
    _scatterplotWidget->setAcceptDrops(true);
}

void ProjectionExplorerPlugin::init()
{
    // Create layout
    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(_scatterplotWidget);

    // Apply the layout
    getWidget().setLayout(layout);

    // Initialize the drop regions
    initializeDropWidget();
    
    // Respond when the name of the dataset in the dataset reference changes
    connect(&_projectionDataset, &Dataset<Points>::guiNameChanged, this, [this]() {

        auto newDatasetName = _projectionDataset->getGuiName();

        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget->setShowDropIndicator(newDatasetName.isEmpty());
    });

    // Alternatively, classes which derive from mv::EventListener (all plugins do) can also respond to events
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&ProjectionExplorerPlugin::onDataEvent, this, std::placeholders::_1));
}

void ProjectionExplorerPlugin::initializeDropWidget()
{
    // Instantiate new drop widget
    _dropWidget = new DropWidget(_scatterplotWidget);

    // Set the drop indicator widget (the widget that indicates that the view is eligible for data dropping)
    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions
        {
            // A drop widget can contain zero or more drop regions
            DropWidget::DropRegions dropRegions;

            const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

            if (datasetsMimeData == nullptr)
                return dropRegions;

            if (datasetsMimeData->getDatasets().count() > 1)
                return dropRegions;

            const auto dataset = datasetsMimeData->getDatasets().first();
            const auto datasetGuiName = dataset->text();
            const auto datasetId = dataset->getId();
            const auto dataType = dataset->getDataType();
            const auto dataTypes = DataTypes({ PointType });

            // Check if the data type can be dropped
            if (!dataTypes.contains(dataType))
                dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);

            // Points dataset is about to be dropped
            if (dataType == PointType)
            {
                // Get points dataset from the core
                auto candidateDataset = mv::data().getDataset<Points>(datasetId);

                // Establish drop region description
                const auto description = QString("Visualize %1 explanations").arg(datasetGuiName);

                if (!_projectionDataset.isValid())
                {

                    // Load as point positions when no dataset is currently loaded
                    dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]()
                        {
                            _projectionDataset = candidateDataset;
                            // Extract 2-dimensional points from the data set based on the selected dimensions
                            std::vector<Vector2f> points;
                            _projectionDataset->extractDataForDimensions(points, 0, 1);
                            _scatterplotWidget->setData(points);
                            _explanationModel.setProjection(_projectionDataset);
                        });
                }
                else
                {
                    if (_projectionDataset != candidateDataset && candidateDataset->getNumDimensions() >= 2)
                    {

                        // The number of points is equal, so offer the option to replace the existing points dataset
                        dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]()
                            {
                                _projectionDataset = candidateDataset;
                            });
                    }
                }
            }

            return dropRegions;
        });
}

void ProjectionExplorerPlugin::onDataEvent(mv::DatasetEvent* dataEvent)
{
    // Get smart pointer to dataset that changed
    const auto changedDataSet = dataEvent->getDataset();

    const auto datasetGuiName = changedDataSet->getGuiName();

    // The data event has a type so that we know what type of data event occurred (e.g. data added, changed, removed, renamed, selection changes)
    switch (dataEvent->getType())
    {
        // Points dataset selection has changed
        case EventType::DatasetDataSelectionChanged:
        {
            // Cast the data event to a data selection changed event
            const auto dataSelectionChangedEvent = static_cast<DatasetDataSelectionChangedEvent*>(dataEvent);

            // Get the selection set that changed
            const auto& selectionSet = changedDataSet->getSelection<Points>();

            // Print to the console
            qDebug() << datasetGuiName << "selection has changed";

            break;
        }

        default:
            break;
    }
}

ViewPlugin* ProjectionExplorerPluginFactory::produce()
{
    return new ProjectionExplorerPlugin(this);
}

mv::DataTypes ProjectionExplorerPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;

    // This example analysis plugin is compatible with points datasets
    supportedTypes.append(PointType);

    return supportedTypes;
}

mv::gui::PluginTriggerActions ProjectionExplorerPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this]() -> ProjectionExplorerPlugin* {
        return dynamic_cast<ProjectionExplorerPlugin*>(plugins().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (numberOfDatasets >= 1 && PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<ProjectionExplorerPluginFactory*>(this), this, "Example", "View example data", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance();
        });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}
