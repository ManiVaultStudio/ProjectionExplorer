#include "ProjectionExplorerPlugin.h"

#include "Widgets/DimensionPickingDialog.h"

#include <ClusterData/ClusterData.h>

#include <event/Event.h>
#include <graphics/Vector2f.h>

#include <QDebug>
#include <QPointF>
#include <QMap>

#include "util/Timer.h" ////////////
#include "Globals.h" /////////// Temp

Q_PLUGIN_METADATA(IID "nl.uu.ProjectionExplorer")

using namespace mv;

namespace
{
    mv::Datasets getPatchseqDatasets()
    {
        Datasets datasets = data().getAllDatasets();

        mv::Datasets patchseqDatasets;
        // Iterate over the children and find the ones of type cluster
        for (Dataset<DatasetImpl> dataset : datasets)
        {
            DataType type = dataset->getDataType();
            if (type == PointType && dataset->hasProperty("PatchSeqType") && dataset->getProperty("PatchSeqType") == "E" || dataset->getProperty("PatchSeqType") == "M")
            {
                auto* parentItem = dataHierarchy().getItem(dataset.getDatasetId());
                DataHierarchyItems childrenItems = dataHierarchy().getChildren(*parentItem);
                
                // Iterate over the children and find the ones of type cluster
                for (DataHierarchyItem* item : childrenItems)
                {
                    DataType type = item->getDataType();
                    if (type == PointType)
                    {
                        Dataset<Points> pointDataset = item->getDataset<Points>();
                        patchseqDatasets.push_back(pointDataset);
                    }
                }
            }
        }
        return patchseqDatasets;
    }
}

ProjectionExplorerPlugin::ProjectionExplorerPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _projectionDataset(nullptr),
    _userInterface(this, _explanationModel)
{
    getWidget().setFocusPolicy(Qt::ClickFocus);
}

void ProjectionExplorerPlugin::init()
{
    // Apply the layout
    getWidget().setLayout(_userInterface.getLayout());

    ui().init();
    
    // Respond when the name of the dataset in the dataset reference changes
    connect(&_projectionDataset, &Dataset<Points>::guiNameChanged, this, [this]() {

        auto newDatasetName = _projectionDataset->getGuiName();

        // Only show the drop indicator when nothing is loaded in the dataset reference
        ui().getDropWidget()->setShowDropIndicator(newDatasetName.isEmpty());
    });

    connect(&_inputEventHandler, &InputEventHandler::mouseDragged, this, &ProjectionExplorerPlugin::onMouseDragged);

    // Update point selection when the position dataset data changes
    connect(&_projectionDataset, &Dataset<Points>::dataSelectionChanged, this, &ProjectionExplorerPlugin::onProjectionSelectionChanged);

    _userInterface.getScatterplotWidget()->installEventFilter(this);

    // Load initial datasets
    _userInterface.getSettingsAction().getCurrentDatasetAction().setDatasets(getPatchseqDatasets());
}

void ProjectionExplorerPlugin::reset()
{
    _userInterface.getExplanationWidget()->reset();
}

void ProjectionExplorerPlugin::onPreNewProjectionLoaded()
{
    // Ask user which dimensions they want to load in the projection
    DimensionPickingDialog dimPickingDialog(&this->getWidget());
    dimPickingDialog.setModal(true);

    int ok = dimPickingDialog.exec();

    if (ok == QDialog::Accepted)
    {
        DIM1 = dimPickingDialog.getFirstDimension();
        DIM2 = dimPickingDialog.getSecondDimension();

        onNewProjectionLoaded();
    }
}

void ProjectionExplorerPlugin::onNewProjectionLoaded()
{
    qDebug() << "onNewProjectionSet";
    reset();

    _projectionDataset = _userInterface.getSettingsAction().getCurrentDatasetAction().getCurrentDataset();

    ui().getDropWidget()->setShowDropIndicator(!_projectionDataset.isValid());

    // Extract 2-dimensional points from the data set based on the selected dimensions
    std::vector<Vector2f> points;
    _projectionDataset->extractDataForDimensions(points, DIM1, DIM2);
    ui().getScatterplotWidget()->setData(points);
    _explanationModel.setProjection(_projectionDataset);
    _localToGlobalIndices.clear();
    _projectionDataset->getGlobalIndices(_localToGlobalIndices); // Save on time to recompute this every time in lens computation
    _explanationModel.computeExplanationMethod();

    // Compute the ranks of all the dimensions per point
    _explanationModel.computeDimensionRanks();
    DataMatrix& dimRanking = _explanationModel.getDimRanking();

    // Get vector of top ranked dimensions
    const std::vector<int>& topRankedDims = _explanationModel.getTopRankedDims();

    // Color points by dimension ranking
    const std::vector<QColor>& colorMapping = _explanationModel.getColorMapping().getColors();

    std::vector<Vector3f> colorData(topRankedDims.size());

    for (int i = 0; i < topRankedDims.size(); i++)
    {
        int dim = topRankedDims[i];
        float confidence = 1;

        if (dim < colorMapping.size())
        {
            QColor color = colorMapping[dim];

            colorData[i] = Vector3f(color.redF() * confidence, color.greenF() * confidence, color.blueF() * confidence);
        }
        else
            colorData[i] = Vector3f(1.0f * confidence, 0.2f * confidence, 0.2f * confidence);
    }

    ui().getScatterplotWidget()->setColors(colorData);

    ui().getExplanationWidget()->getHistogramChart().computeGlobalHistograms();

    _userInterface.getExplanationWidget()->update();
}

void ProjectionExplorerPlugin::generateClusterDataset()
{
    if (!_projectionDataset.isValid())
        return;

    // Color points by dimension ranking
    const std::vector<QColor>& colorMapping = _explanationModel.getColorMapping().getColors();

    // Get vector of top ranked dimensions
    const std::vector<int>& topRankedDims = _explanationModel.getTopRankedDims();

    mv::Dataset<Clusters> clusterData = mv::data().createDataset("Cluster", "TestClusters", _projectionDataset->getSourceDataset<Points>());

    QHash<QString, Cluster> clusters;
    for (int i = 0; i < _explanationModel.getColorMapping().getPalette().size(); i++)
    {
        const QColor& c = _explanationModel.getColorMapping().getPalette()[i];
        QString name = QString("%1%2%3").arg(c.red()).arg(c.green()).arg(c.blue());
        Cluster cluster(name, c);
        clusters[name] = cluster;
    }

    for (int i = 0; i < topRankedDims.size(); i++)
    {
        int dim = topRankedDims[i];
        float confidence = 1;

        if (dim < colorMapping.size())
        {
            QColor color = colorMapping[dim];
            QString name = QString("%1%2%3").arg(color.red()).arg(color.green()).arg(color.blue());

            auto& indices = clusters[name].getIndices();
            indices.push_back(i);
            clusters[name].setName(_explanationModel.getDataset().getDimensionNames()[dim]);
            clusters[name].setIndices(indices);
        }
    }
    clusterData->setClusters(clusters.values());
    events().notifyDatasetDataChanged(clusterData);
}

void ProjectionExplorerPlugin::onProjectionSelectionChanged()
{
    if (!_projectionDataset.isValid())
        return;
    qDebug() << "Selection changed";
    Timer t("Selection changed");
    auto selection = _projectionDataset->getSelection<Points>();

    std::vector<bool> selected;
    std::vector<char> highlights;

    {
        Timer t("Local selection");
        _projectionDataset->selectedLocalIndices(selection->indices, selected);
    }

    highlights.resize(_projectionDataset->getNumPoints(), 0);

    for (int i = 0; i < selected.size(); i++)
        highlights[i] = selected[i] ? 1 : 0;

    //qDebug() << "highlights:" << highlights.size();
    ui().getScatterplotWidget()->setSelection(highlights, static_cast<std::int32_t>(selection->indices.size()));

    if (selection->indices.size() > 0)
    {
        _explanationModel.computeSelectionDimensionRanks(selection->indices);
    }

    ui().getExplanationWidget()->getHistogramChart().setRanking(selection->indices);

    ui().getScatterplotWidget()->update();
    ui().getExplanationWidget()->getHistogramChart().update();

    //std::vector<float> dimRanking(_explanationModel.getDataset().getNumCols());
    //_explanationModel.computeSelectionDimensionRanks(dimRanking, selection->indices);
}

void ProjectionExplorerPlugin::onMouseDragged(Vector2f cursorPos)
{
    Timer t("Mouse drag");
    qDebug() << "Mouse drag";
    // Update the lens
    _explanationModel.getLens().position = cursorPos;
    const Lens& lens = _explanationModel.getLens();

    // Reserve space for the maximum number of points that can possibly fall within the lens selection
    std::vector<std::uint32_t> lensSelectionIndices;
    lensSelectionIndices.reserve(_projectionDataset->getNumPoints());

    const auto dataBounds = ui().getScatterplotWidget()->getBounds();
    const auto w = ui().getScatterplotWidget()->width();
    const auto h = ui().getScatterplotWidget()->height();
    const auto size = w < h ? w : h;
    const auto uvOffset = Vector2f((ui().getScatterplotWidget()->width() - size) / 2.0f, (ui().getScatterplotWidget()->height() - size) / 2.0f);

    DataMatrix& projection = _explanationModel.getProjection();
    float lensRadiusSqr = lens.radius * lens.radius;

    Vector2f lensPositionInDataSpace = lens.position - uvOffset;
    lensPositionInDataSpace /= Vector2f(size, size);
    lensPositionInDataSpace *= Vector2f(dataBounds.getWidth(), -dataBounds.getHeight());
    lensPositionInDataSpace += Vector2f(dataBounds.getLeft(), dataBounds.getTop());
    //    ((lens.position / size) - uvOffset) * Vector2f(dataBounds.getWidth(), dataBounds.getHeight()) + Vector2f()

    float lensRadiusInDataSpace = _explanationModel.getLens().radius;
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
    // FIXME Why does this need to notify source dataset?
    events().notifyDatasetDataSelectionChanged(_projectionDataset->getSourceDataset<Points>());

    //qDebug() << "Lens selection indices: " << lensSelectionIndices.size();
}

void ProjectionExplorerPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    ViewPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "SettingsAction");

    _userInterface.getSettingsAction().fromVariantMap(variantMap["SettingsAction"].toMap());
}

QVariantMap ProjectionExplorerPlugin::toVariantMap() const
{
    QVariantMap variantMap = ViewPlugin::toVariantMap();

    const SettingsAction& settingsAction = _userInterface.getSettingsAction();
    settingsAction.insertIntoVariantMap(variantMap);

    return variantMap;
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
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<ProjectionExplorerPluginFactory*>(this), this, "Example", "View example data", icon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance();
        });

        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}
