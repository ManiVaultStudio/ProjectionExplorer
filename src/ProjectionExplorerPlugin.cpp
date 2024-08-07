#include "ProjectionExplorerPlugin.h"

#include <event/Event.h>
#include <graphics/Vector2f.h>

#include <DatasetsMimeData.h>

#include <QDebug>
#include <QMimeData>
#include <QPointF>

#include "util/Timer.h" ////////////

Q_PLUGIN_METADATA(IID "nl.uu.ProjectionExplorer")

using namespace mv;

ProjectionExplorerPlugin::ProjectionExplorerPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _projectionDataset(nullptr),
    _settingsAction(this, "SettingsAction"),
    _scatterplotWidget(new ScatterplotWidget(_explanationModel))
{
    // This line is mandatory if drag and drop behavior is required
    _scatterplotWidget->setAcceptDrops(true);

    getWidget().setFocusPolicy(Qt::ClickFocus);
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
    //_eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    //_eventListener.registerDataEventByType(PointType, std::bind(&ProjectionExplorerPlugin::onDataEvent, this, std::placeholders::_1));

    _scatterplotWidget->installEventFilter(this);

    connect(&_inputEventHandler, &InputEventHandler::mouseDragged, this, &ProjectionExplorerPlugin::onMouseDragged);

    // Update point selection when the position dataset data changes
    connect(&_projectionDataset, &Dataset<Points>::dataSelectionChanged, this, &ProjectionExplorerPlugin::onProjectionSelectionChanged);
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
                            _projectionDataset->getGlobalIndices(_localToGlobalIndices); // Save on time to recompute this every time in lens computation
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

void ProjectionExplorerPlugin::onProjectionSelectionChanged()
{
    if (!_projectionDataset.isValid())
        return;

    Timer t("Selection changed");
    auto selection = _projectionDataset->getSelection<Points>();

    std::vector<bool> selected;
    std::vector<char> highlights;

    _projectionDataset->selectedLocalIndices(selection->indices, selected);

    highlights.resize(_projectionDataset->getNumPoints(), 0);

    for (int i = 0; i < selected.size(); i++)
        highlights[i] = selected[i] ? 1 : 0;

    //qDebug() << "highlights:" << highlights.size();
    _scatterplotWidget->setSelection(highlights, static_cast<std::int32_t>(selection->indices.size()));

    _scatterplotWidget->update();
}

void ProjectionExplorerPlugin::onMouseDragged(Vector2f cursorPos)
{
    Timer t("Mouse drag");

    // Update the lens
    _explanationModel.getLens().position = cursorPos;
    const Lens& lens = _explanationModel.getLens();

    // Reserve space for the maximum number of points that can possibly fall within the lens selection
    std::vector<std::uint32_t> lensSelectionIndices;
    lensSelectionIndices.reserve(_projectionDataset->getNumPoints());

    const auto dataBounds = _scatterplotWidget->getBounds();
    const auto w = _scatterplotWidget->width();
    const auto h = _scatterplotWidget->height();
    const auto size = w < h ? w : h;
    const auto uvOffset = Vector2f((_scatterplotWidget->width() - size) / 2.0f, (_scatterplotWidget->height() - size) / 2.0f);

    DataMatrix& projection = _explanationModel.getProjection();
    float lensRadiusSqr = lens.radius * lens.radius;

    Vector2f lensPositionInDataSpace = lens.position - uvOffset;
    lensPositionInDataSpace /= Vector2f(size, size);
    lensPositionInDataSpace *= Vector2f(dataBounds.getWidth(), -dataBounds.getHeight());
    lensPositionInDataSpace += Vector2f(dataBounds.getLeft(), dataBounds.getTop());
    //    ((lens.position / size) - uvOffset) * Vector2f(dataBounds.getWidth(), dataBounds.getHeight()) + Vector2f()

    float lensRadiusInDataSpace = 30;
    lensRadiusInDataSpace /= size;
    lensRadiusInDataSpace *= dataBounds.getWidth(); // FIXME For now width should be same as height, but perhaps not always
    float lensRadiusInDataSpaceSqr = lensRadiusInDataSpace * lensRadiusInDataSpace;
    {
        Timer t("Inner mouse drag");
        // Loop over all points and establish whether they are selected or not
        for (int i = 0; i < projection.getNumRows(); i++) {
            Vector2f p(projection(i, 0), projection(i, 1));
            Vector2f diff = lensPositionInDataSpace - p;

            if (diff.sqrMagnitude() < lensRadiusInDataSpaceSqr)
                lensSelectionIndices.push_back(_localToGlobalIndices[i]);
        }
    }

    // Apply the selection indices
    _projectionDataset->setSelectionIndices(lensSelectionIndices);

    mv::Dataset<Points> selection = _projectionDataset->getSelection<Points>();

        // Notify others that the selection changed
        events().notifyDatasetDataSelectionChanged(_projectionDataset);

    //qDebug() << "Lens selection indices: " << lensSelectionIndices.size();
}

bool ProjectionExplorerPlugin::eventFilter(QObject* target, QEvent* event)
{
    _inputEventHandler.onEvent(event);

    return QObject::eventFilter(target, event);
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
