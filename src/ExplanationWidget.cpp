#include "ExplanationWidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QEvent>
#include <QMouseEvent>

#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>

#define RANGE_OFFSET 250
#define RANGE_WIDTH 200
#define TOP_MARGIN 30
#define BOX_HEIGHT 40
#define DIFF_HEIGHT 12
#define LEGEND_BUTTON 400
#define HIGHLIGHT_COLOR Qt::white

bool isMouseOverBox(QPoint mousePos, int x, int y, int size)
{
    return mousePos.x() > x && mousePos.x() < x + size && mousePos.y() > y && mousePos.y() < y + size;
}

void DataMetrics::compute(const DataTable& dataset, const std::vector<unsigned int>& selection, const DataStatistics& dataStats)
{
    int numDimensions = dataset.numDimensions();

    // Compute average values
    averageValues.clear();
    averageValues.resize(numDimensions, 0);
    for (int j = 0; j < numDimensions; j++)
    {
        for (int i = 0; i < selection.size(); i++)
        {
            int si = selection[i];

            averageValues[j] += dataset(si, j);
        }

        averageValues[j] /= selection.size();
    }

    // Compute variances
    variances.clear();
    variances.resize(numDimensions, 0);
    for (int j = 0; j < numDimensions; j++)
    {
        for (int i = 0; i < selection.size(); i++)
        {
            int si = selection[i];
            float x = dataset(si, j) - averageValues[j];
            variances[j] += x * x;
        }
        variances[j] /= selection.size();
        variances[j] = sqrt(variances[j]);
    }

    // Normalization
    for (int j = 0; j < numDimensions; j++)
    {
        float range = dataStats.ranges[j];

        float normalizedValue = (averageValues[j] - dataStats.minRange[j]) / range;
        averageValues[j] = normalizedValue;
        float normalizedStddev = variances[j] / range;
        variances[j] = normalizedStddev;

        //std::cout << "Dim: " << j << " Min ranges: " << minRanges[j] << " Max ranges: " << maxRanges[j] << std::endl;
    }
}

void drawVarianceWhiskers(QPainter& painter, float x1, float x2, int y)
{
    QPen originalPen = painter.pen();
    QPen variancePen;
    variancePen.setColor(QColor(255, 255, 255, 255));
    variancePen.setWidth(2);
    painter.setPen(variancePen);

    painter.drawLine(RANGE_OFFSET + x1 * RANGE_WIDTH, TOP_MARGIN + y, RANGE_OFFSET + x2 * RANGE_WIDTH, TOP_MARGIN + y);

    variancePen.setWidth(1);
    painter.setPen(variancePen);

    painter.drawLine(RANGE_OFFSET + x1 * RANGE_WIDTH, TOP_MARGIN + y - 4, RANGE_OFFSET + x1 * RANGE_WIDTH, TOP_MARGIN + y + 4);
    painter.drawLine(RANGE_OFFSET + x2 * RANGE_WIDTH, TOP_MARGIN + y - 4, RANGE_OFFSET + x2 * RANGE_WIDTH, TOP_MARGIN + y + 4);

    // Restore original pen
    painter.setPen(originalPen);
}

BarChart::BarChart(ExplanationModel& explanationModel) :
    _explanationModel(explanationModel),
    _differentialRanking(false),
    _sortingType(SortingType::NONE),
    _sortingDirection(SortingDirection::LOW_TO_HIGH)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(270);
    setMinimumWidth(460);
    setContentsMargins(0, 0, 0, 0);

    installEventFilter(this);
    setMouseTracking(true);

    _legend.load(":/Legend.png");
    _diffLegend.load(":/DiffLegend.png");

    connect(&_explanationModel, &ExplanationModel::datasetChanged, this, &BarChart::datasetChanged);
    //const char* tab_colors[] = { "#4e79a7", "#59a14f", "#9c755f", "#f28e2b", "#edc948", "#bab0ac", "#e15759", "#b07aa1", "#76b7b2", "#ff9da7" };
}

void BarChart::setRanking(DataMatrix& ranking, const std::vector<unsigned int>& selection)
{
    int numPoints = ranking.rows();
    int numDimensions = ranking.cols();

    _selection = selection;

    if (numPoints == 0)
    {
        _dimAggregation.clear();
        return;
    }

    std::vector<float> dimAggregation(numDimensions, 0);
    for (int i = 0; i < numPoints; i++)
    {
        for (int j = 0; j < numDimensions; j++)
        {
            dimAggregation[j] += ranking(i, j);
        }
    }
    for (int i = 0; i < numDimensions; i++)
    {
        dimAggregation[i] /= (float) numPoints;
    }
    _dimAggregation = dimAggregation;

    // Compute metrics on current selection
    _newMetrics.compute(_explanationModel.getDataset(), _selection, _explanationModel.getDataStatistics());

    // Compute sorting
    _sortIndices.resize(numDimensions);
    std::iota(_sortIndices.begin(), _sortIndices.end(), 0);

    switch (_explanationModel.currentMetric())
    {
    case Explanation::Metric::VARIANCE:
        std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return dimAggregation[i] < dimAggregation[j]; }); break;
    case Explanation::Metric::VALUE:
        std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return dimAggregation[i] > dimAggregation[j]; }); break;
    default: break;
    }
    //if (_explanationModel.currentMetric() == Explanation::Metric::VARIANCE)
    //    std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return dimAggregation[i] < dimAggregation[j]; });
    //else if (_sortingType == SortingType::VALUE)
    //    std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return _newMetrics.averageValues[i] > _newMetrics.averageValues[j]; });
    //else if (_sortingType == SortingType::NONE) {}
}

void BarChart::setRanking(const std::vector<float>& dimRanking, const std::vector<unsigned int>& selection)
{
    _selection = selection;

    if (selection.size() == 0)
    {
        _dimAggregation.clear();
        _sortIndices.clear();
        return;
    }

    int numDimensions = dimRanking.size();

    _globalHistograms.clear();
    _globalHistograms.resize(numDimensions, Histogram(20));

    _histograms.clear();
    _histograms.resize(numDimensions, Histogram(20));

    // Print rankings
    //for (int j = 0; j < numDimensions; j++)
    //{
    //    std::cout << "Dim selection rank 1: " << dimRanking[j];
    //}
    //std::cout << std::endl;

    _dimAggregation = dimRanking;

    // Compute metrics on current selection
    _newMetrics.compute(_explanationModel.getDataset(), _selection, _explanationModel.getDataStatistics());

    const DataTable& dataset = _explanationModel.getDataset();

    // Compute global histograms
    for (int j = 0; j < numDimensions; j++)
    {
        _globalHistograms[j].setRange(_explanationModel.getDataStatistics().minRange[j], _explanationModel.getDataStatistics().maxRange[j]);
        for (int i = 0; i < dataset.numPoints(); i++)
        {
            _globalHistograms[j].addDataValue(dataset(i, j));
        }
    }

    // Compute local histograms
    for (int j = 0; j < numDimensions; j++)
    {
        _histograms[j].setRange(_explanationModel.getDataStatistics().minRange[j], _explanationModel.getDataStatistics().maxRange[j]);
        for (int i = 0; i < selection.size(); i++)
        {
            int si = selection[i];

            _histograms[j].addDataValue(dataset(si, j));
        }
    }

    // Compute sorting
    _sortIndices.clear();
    _sortIndices.resize(numDimensions);
    std::iota(_sortIndices.begin(), _sortIndices.end(), 0);

    // Exclude dimensions
    for (int j = 0; j < numDimensions; j++)
    {
        if (_explanationModel.getDataset().isExcluded(j))
        {
            if (_explanationModel.currentMetric() == Explanation::Metric::VARIANCE)
                _dimAggregation[j] = std::numeric_limits<float>::max();
            if (_explanationModel.currentMetric() == Explanation::Metric::VALUE)
                _dimAggregation[j] = -std::numeric_limits<float>::max();
        }
    }

    switch (_explanationModel.currentMetric())
    {
    case Explanation::Metric::VARIANCE:
        std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return _dimAggregation[i] < _dimAggregation[j]; }); break;
    case Explanation::Metric::VALUE:
        std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return _dimAggregation[i] > _dimAggregation[j]; }); break;
    default: break;
    }

    update();
}

void BarChart::computeOldMetrics(const std::vector<unsigned int>& oldSelection)
{
    _oldMetrics.compute(_explanationModel.getDataset(), oldSelection, _explanationModel.getDataStatistics());
}

void BarChart::datasetChanged()
{
    _dimAggregation.clear();
    _sortIndices.clear();
    _selection.clear();

    // Draw legend or not depending on the number of dimensions
    std::cout << "Height: " << height() << " Dim height: " << (_explanationModel.getDataset().numDimensions() * 20 + 270) << std::endl;
    if (_explanationModel.getDataset().numDimensions() * 20 + 270 > height())
        _drawLegend = false;
    else
        _drawLegend = true;
}

void BarChart::sortByDefault()
{
    _sortingType = SortingType::NONE;
}

void BarChart::sortByVariance()
{
    _sortingType = SortingType::VARIANCE;
}

void BarChart::sortByValue()
{
    _sortingType = SortingType::VALUE;
}

void BarChart::paintEvent(QPaintEvent* event)
{
    if (!_explanationModel.hasDataset())
    {
        return;
    }

    const DataTable& dataset = _explanationModel.getDataset();
    const DataStatistics& dataStats = _explanationModel.getDataStatistics();

    int numDimensions = dataset.numDimensions();

    const std::vector<QColor>& colorMapping = _explanationModel.getColorMapping();

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    painter.fillRect(0, 0, 600, (numDimensions + 2) * BOX_HEIGHT, QColor(38, 38, 38));

    painter.setPen(Qt::white);
    QFont font = QFont("MS Shell Dlg 2", 10, QFont::ExtraBold);
    QFontMetricsF fm(font);
    font.setPixelSize(16);
    painter.setFont(font);
    
    if (_dimAggregation.size() > 0)
    {
        // Draw sorting label
        switch (_explanationModel.currentMetric())
        {
        case Explanation::Metric::VARIANCE: painter.drawText(10, 20, "Variance Sort"); break;
        case Explanation::Metric::VALUE: painter.drawText(10, 20, "Value Sort"); break;
        default: painter.drawText(10, 20, "No Sorting");
        }

        if (!_differentialRanking)
        {
            //// Draw PCP
            //int alpha = std::min(2 * std::max<int>(255.0f / _selection.size(), 1), 255);

            //QPen pcpPen;
            //pcpPen.setColor(QColor(255, 255, 0, alpha));
            //pcpPen.setWidth(2);
            //painter.setPen(pcpPen);
            //
            //for (const unsigned int& si : _selection)
            //{
            //    for (int j = 0; j < numDimensions - 1; j++)
            //    {
            //        int sortIndexA = _sortIndices[j];
            //        int sortIndexB = _sortIndices[j + 1];

            //        bool excluded = _explanationModel.getDataset().isExcluded(sortIndexB);
            //        if (excluded) continue;

            //        float normValueA = (dataset(si, sortIndexA) - dataStats.minRange[sortIndexA]) / dataStats.ranges[sortIndexA];
            //        float normValueB = (dataset(si, sortIndexB) - dataStats.minRange[sortIndexB]) / dataStats.ranges[sortIndexB];

            //        painter.drawLine(RANGE_OFFSET + normValueA * RANGE_WIDTH, TOP_MARGIN + BOX_HEIGHT * j + 0, RANGE_OFFSET + normValueB * RANGE_WIDTH, TOP_MARGIN + BOX_HEIGHT * (j + 1) + 0);
            //    }
            //}
        }

        for (int i = 0; i < _dimAggregation.size(); i++)
        {
            int sortIndex = _sortIndices[i];

            bool excluded = _explanationModel.getDataset().isExcluded(sortIndex);

            QColor color(180, 180, 180, 255);
            if (sortIndex < colorMapping.size())
                color = colorMapping[sortIndex];
            if (excluded)
                color = QColor(255, 255, 255);

            //painter.setPen(Qt::red);
            //painter.drawEllipse(_mousePos.x() - 8, _mousePos.y() - 8, 16, 16);

            // Draw colored legend boxes
            // Check if mouse is over box, if so, draw a cross over it
            if (isMouseOverBox(_mousePos, 10, TOP_MARGIN + BOX_HEIGHT * i, 14))
            {
                painter.fillRect(10, TOP_MARGIN + BOX_HEIGHT * i, 14, 14, HIGHLIGHT_COLOR);
            }
            else
            {
                painter.fillRect(10, TOP_MARGIN + BOX_HEIGHT * i, 14, 14, color);
            }

            // Draw dimension names
            painter.setPen(color);
            QString dimName = _explanationModel.getDataNames()[sortIndex];
            dimName = fm.elidedText(dimName, Qt::TextElideMode::ElideRight, 150);
            painter.drawText(30, TOP_MARGIN + 10 + i * BOX_HEIGHT, dimName);

            if (excluded)
                continue;

            //float normalizedValue = (_averageValues[sortIndex] - _minRanges[sortIndex]) / (_maxRanges[sortIndex] - _minRanges[sortIndex]);

            //painter.fillRect(180, 10 + 16 * i, std::max(0.0f, 0.166f - _dimAggregation[sortIndex]) * 600, 14, color);
            // 
            //painter.fillRect(180, 10 + 16 * i, std::max(0.0f, _dimAggregation[sortIndex]) * 600, 14, _colors[sortIndex]);

            // Draw importance
            //painter.fillRect(180, TOP_MARGIN + 16 * i, _dimAggregation[sortIndex] * 600, 7, QColor(0, 255, 0));
            // 
            // Draw relative value bars
            //if (_explanationModel.currentMetric() == Explanation::Metric::VALUE)
            //{
            //    painter.fillRect(180, TOP_MARGIN + 16 * i, _dimAggregation[sortIndex] * 10, 7, QColor(0, 255, 0));
            //}
            
            //painter.fillRect(50, 16 * i + 7, _importantDims[i] * 60, 7, QColor(0, 0, 255));

            //// Draw values
            // Draw range line
            painter.drawLine(RANGE_OFFSET, TOP_MARGIN + BOX_HEIGHT * i + 0, RANGE_OFFSET + RANGE_WIDTH, TOP_MARGIN + BOX_HEIGHT * i + 0);

            // Draw histogram
            if (!_differentialRanking)
            {
                Histogram& globalHist = _globalHistograms[sortIndex];
                Histogram& hist = _histograms[sortIndex];
                int globalNumPoints = globalHist.getNumDataPoints();
                int localNumPoints = hist.getNumDataPoints();
                int globalHighestBinValue = globalHist.getHighestBinValue();
                int localHighestBinValue = hist.getHighestBinValue();

                // Draw global histograms
                float globalBoxWidth = (float)RANGE_WIDTH / globalHist.getBins().size();
                for (int b = 0; b < globalHist.getBins().size(); b++)
                {
                    painter.fillRect(RANGE_OFFSET + b * globalBoxWidth, TOP_MARGIN + BOX_HEIGHT * i + 0, globalBoxWidth, ((float) globalHist.getBins()[b] / globalHighestBinValue) * BOX_HEIGHT / 2, QColor(180, 180, 180));
                }

                // Draw local histograms
                float boxWidth = (float) RANGE_WIDTH / hist.getBins().size();
                for (int b = 0; b < hist.getBins().size(); b++)
                {
                    painter.fillRect(RANGE_OFFSET + b * boxWidth, TOP_MARGIN + BOX_HEIGHT * i + 0, boxWidth, ((float) -hist.getBins()[b] / localHighestBinValue) * BOX_HEIGHT / 2, QColor(0, 180, 225));
                }

                //// Draw mean
                //float mean = _newMetrics.averageValues[sortIndex];
                //float globalMean = (dataStats.means[sortIndex] - dataStats.minRange[sortIndex]) / dataStats.ranges[sortIndex];

                //int oldMeanX = RANGE_OFFSET + globalMean * RANGE_WIDTH;
                //int newMeanX = RANGE_OFFSET + mean * RANGE_WIDTH;

                //QColor diffColor = newMeanX - oldMeanX >= 0 ? QColor(0, 255, 0, 128) : QColor(255, 0, 0, 128);
                //painter.fillRect(oldMeanX, TOP_MARGIN + 16 * i + 2, newMeanX - oldMeanX, DIFF_HEIGHT, diffColor);

                //// Draw variance
                //painter.setPen(QColor(255, 255, 255, 255));
                ////painter.drawRect(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 2 + 16 * i, (_newMetrics.variances[sortIndex]) * RANGE_WIDTH * 2, 12);

                //drawVarianceWhiskers(painter, mean - _newMetrics.variances[sortIndex], mean + _newMetrics.variances[sortIndex], 16 * i + 8);
                ////painter.drawLine(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i, RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i);
                ////painter.drawLine(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 4 + 16 * i, RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 12 + 16 * i);
                ////painter.drawLine(RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 4 + 16 * i, RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 12 + 16 * i);

                //QPen meanPen;
                //meanPen.setColor(QColor(255, 0, 0, 255));
                //meanPen.setWidth(2);
                //painter.setPen(meanPen);
                //painter.drawLine(RANGE_OFFSET + mean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 2, RANGE_OFFSET + mean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 14);

                //// Draw global mean
                ////painter.setPen(QColor(180, 180, 180, 255));
                //meanPen.setColor(QColor(180, 180, 180, 255));
                //painter.setPen(meanPen);
                //painter.drawLine(RANGE_OFFSET + globalMean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 0, RANGE_OFFSET + globalMean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 16);
            }
            // Draw normally
            //if (!_differentialRanking)
            //{
            //    // Draw mean
            //    float mean = _newMetrics.averageValues[sortIndex];
            //    float globalMean = (dataStats.means[sortIndex] - dataStats.minRange[sortIndex]) / dataStats.ranges[sortIndex];
            //    
            //    int oldMeanX = RANGE_OFFSET + globalMean * RANGE_WIDTH;
            //    int newMeanX = RANGE_OFFSET + mean * RANGE_WIDTH;

            //    QColor diffColor = newMeanX - oldMeanX >= 0 ? QColor(0, 255, 0, 128) : QColor(255, 0, 0, 128);
            //    painter.fillRect(oldMeanX, TOP_MARGIN + 16 * i + 2, newMeanX - oldMeanX, DIFF_HEIGHT, diffColor);

            //    // Draw variance
            //    painter.setPen(QColor(255, 255, 255, 255));
            //    //painter.drawRect(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 2 + 16 * i, (_newMetrics.variances[sortIndex]) * RANGE_WIDTH * 2, 12);

            //    drawVarianceWhiskers(painter, mean - _newMetrics.variances[sortIndex], mean + _newMetrics.variances[sortIndex], 16 * i + 8);
            //    //painter.drawLine(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i, RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i);
            //    //painter.drawLine(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 4 + 16 * i, RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 12 + 16 * i);
            //    //painter.drawLine(RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 4 + 16 * i, RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 12 + 16 * i);

            //    QPen meanPen;
            //    meanPen.setColor(QColor(255, 0, 0, 255));
            //    meanPen.setWidth(2);
            //    painter.setPen(meanPen);
            //    painter.drawLine(RANGE_OFFSET + mean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 2, RANGE_OFFSET + mean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 14);

            //    // Draw global mean
            //    //painter.setPen(QColor(180, 180, 180, 255));
            //    meanPen.setColor(QColor(180, 180, 180, 255));
            //    painter.setPen(meanPen);
            //    painter.drawLine(RANGE_OFFSET + globalMean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 0, RANGE_OFFSET + globalMean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 16);


            //    //painter.drawRect(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, 10 + 16 * i, 2 * _newMetrics.variances[sortIndex] * RANGE_WIDTH, 16);


            //}
            else
            {
                // Draw mean
                float oldMean = _oldMetrics.averageValues[sortIndex];
                float newMean = _newMetrics.averageValues[sortIndex];

                int oldMeanX = RANGE_OFFSET + oldMean * RANGE_WIDTH;
                int newMeanX = RANGE_OFFSET + newMean * RANGE_WIDTH;

                QColor diffColor = newMeanX - oldMeanX >= 0 ? QColor(0, 255, 0, 128) : QColor(255, 0, 0, 128);
                painter.fillRect(oldMeanX, TOP_MARGIN + 16 * i + 2, newMeanX - oldMeanX, DIFF_HEIGHT, diffColor);

                QPen meanPen;
                meanPen.setWidth(2);

                meanPen.setColor(QColor(128, 0, 0, 255));
                painter.setPen(meanPen);
                painter.drawLine(oldMeanX, TOP_MARGIN + 16 * i + 2, oldMeanX, TOP_MARGIN + 16 * i + 14);
                
                meanPen.setColor(QColor(255, 0, 0, 255));
                painter.setPen(meanPen);
                painter.drawLine(newMeanX, TOP_MARGIN + 16 * i, newMeanX, TOP_MARGIN + 16 * i + 16);

                qDebug() << newMeanX << oldMeanX << newMean << oldMean;

                // Draw variance
                painter.setPen(QColor(255, 255, 255, 255));
                //painter.drawRect(RANGE_OFFSET + (newMean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, 10 + 16 * i, 2 * _newMetrics.variances[sortIndex] * RANGE_WIDTH, 16);
                drawVarianceWhiskers(painter, newMean - _newMetrics.variances[sortIndex], newMean + _newMetrics.variances[sortIndex], 16 * i + 8);

                //painter.drawLine(RANGE_OFFSET + (newMean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i, RANGE_OFFSET + (newMean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i);
                //painter.drawLine(RANGE_OFFSET + (newMean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 4 + 16 * i, RANGE_OFFSET + (newMean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 12 + 16 * i);
                //painter.drawLine(RANGE_OFFSET + (newMean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 4 + 16 * i, RANGE_OFFSET + (newMean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 12 + 16 * i);
            }
        }
    }
    else if (_explanationModel.hasDataset())
    {
        painter.drawText(10, 20, "No Sorting");

        for (int i = 0; i < _explanationModel.getDataset().numDimensions(); i++)
        {
            // Draw colored legend boxes
            // Set color according to exclusion
            bool excluded = _explanationModel.getDataset().isExcluded(i);

            QColor color(180, 180, 180, 255);
            if (i < colorMapping.size())
                color = colorMapping[i];
            if (excluded)
                color = QColor(255, 255, 255);

            // Check if mouse is over box, if so, draw a cross over it
            if (isMouseOverBox(_mousePos, 10, TOP_MARGIN + BOX_HEIGHT * i, 14))
            {
                painter.fillRect(10, TOP_MARGIN + BOX_HEIGHT * i, 14, 14, HIGHLIGHT_COLOR);
            }
            else
            {
                painter.fillRect(10, TOP_MARGIN + BOX_HEIGHT * i, 14, 14, color);
            }

            painter.setPen(QColor(255, 255, 255));
            if (_explanationModel.getDataNames().size() > 0)
            {
                painter.drawText(30, TOP_MARGIN + 10 + BOX_HEIGHT * i, _explanationModel.getDataNames()[i]);
            }
            else
            {
                painter.drawText(30, TOP_MARGIN + 10 + BOX_HEIGHT * i, QString::number(i));
            }
        }
    }

    // Draw legend
    int legendY = _drawLegend ? std::min(TOP_MARGIN + 8 + BOX_HEIGHT * (numDimensions + 2), height()-270) : height() - 30;
    int legendHeight = _drawLegend ? 270 : 30;

    painter.fillRect(0, legendY, 600, legendHeight, QColor(60, 60, 60));

    painter.setPen(QColor(255, 255, 255));
    painter.drawText(10, legendY + 20, "Legend");

    // Draw minimize / maximize
    QPen arrowPen;
    arrowPen.setColor(Qt::white);
    arrowPen.setWidth(3);
    painter.setPen(arrowPen);

    if (_drawLegend)
    {
        // Draw minus sign
        painter.drawLine(LEGEND_BUTTON, legendY + 15, LEGEND_BUTTON + 10, legendY + 15);

        if (_differentialRanking)
            painter.drawImage(60, legendY + 40, _diffLegend);
        else
            painter.drawImage(60, legendY + 40, _legend);
    }
    else
    {
        // Draw plus sign
        painter.drawLine(LEGEND_BUTTON, legendY + 15, LEGEND_BUTTON + 10, legendY + 15);
        painter.drawLine(LEGEND_BUTTON + 5, legendY + 10, LEGEND_BUTTON + 5, legendY + 20);
    }
    
    //// Draw range line
    //int leftEnd = 50;
    //int rightEnd = 150;
    //int mid = 100;

    //// Range line
    //painter.setPen(QColor(180, 90, 0, 255));
    //painter.drawLine(RANGE_OFFSET, varHeight, RANGE_OFFSET + RANGE_WIDTH, varHeight);

    //// Stddev lines
    //painter.setPen(QColor(255, 255, 255, 255));
    //painter.drawLine(RANGE_OFFSET + leftEnd, varHeight, RANGE_OFFSET + rightEnd, varHeight);
    //painter.drawLine(RANGE_OFFSET + leftEnd, varHeight - 5, RANGE_OFFSET + leftEnd, varHeight + 5);
    //painter.drawLine(RANGE_OFFSET + rightEnd, varHeight - 5, RANGE_OFFSET + rightEnd, varHeight + 5);
    //// Labels
    //painter.drawText(RANGE_OFFSET + leftEnd - 40, varHeight - 15, QString("-1 Stddev"));
    //painter.drawText(RANGE_OFFSET + rightEnd -40, varHeight - 15, QString("+1 Stddev"));

    //// Local mean
    //painter.setPen(QColor(255, 0, 0, 255));
    //painter.drawLine(RANGE_OFFSET + mid, varHeight - 5, RANGE_OFFSET + mid, varHeight + 5);
    //// Little Arrow
    //painter.drawLine(RANGE_OFFSET + mid, varHeight + 10, RANGE_OFFSET + mid - 5, varHeight + 15);
    //painter.drawLine(RANGE_OFFSET + mid, varHeight + 10, RANGE_OFFSET + mid + 5, varHeight + 15);
    //// Label
    //painter.drawText(RANGE_OFFSET + mid - 50, varHeight + 30, QString("Selection Mean"));

    ////// + Difference legend
    //varHeight = TOP_MARGIN + 8 + 16 * (numDimensions + 5);

    //// Range line
    //painter.setPen(QColor(180, 90, 0, 255));
    //painter.drawLine(RANGE_OFFSET, varHeight, RANGE_OFFSET + RANGE_WIDTH, varHeight);

    //// Local mean
    //painter.setPen(QColor(255, 0, 0, 255));
    //painter.drawLine(RANGE_OFFSET + mid+30, varHeight - 5, RANGE_OFFSET + mid+30, varHeight + 5);

    //painter.setPen(QColor(180, 180, 180, 255));
    //painter.drawLine(RANGE_OFFSET + mid - 30, varHeight-5, RANGE_OFFSET + mid - 30, varHeight + 5);
    //painter.fillRect(RANGE_OFFSET + mid - 30, varHeight-4, 60, DIFF_HEIGHT, QColor(0, 255, 0, 128));
    //painter.setPen(QColor(255, 255, 255, 255));
    //painter.drawText(RANGE_OFFSET + mid - 330, varHeight + 5, QString("Selection Mean > Global Mean"));

    ////// - Difference legend
    //varHeight = TOP_MARGIN + 8 + 16 * (numDimensions + 6);

    //// Range line
    //painter.setPen(QColor(180, 90, 0, 255));
    //painter.drawLine(RANGE_OFFSET, varHeight, RANGE_OFFSET + RANGE_WIDTH, varHeight);

    //// Local mean
    //painter.setPen(QColor(255, 0, 0, 255));
    //painter.drawLine(RANGE_OFFSET + mid - 60, varHeight - 5, RANGE_OFFSET + mid - 60, varHeight + 5);

    //painter.setPen(QColor(180, 180, 180, 255));
    //painter.drawLine(RANGE_OFFSET + mid, varHeight - 5, RANGE_OFFSET + mid, varHeight + 5);
    //painter.fillRect(RANGE_OFFSET + mid - 60, varHeight - 4, 60, DIFF_HEIGHT, QColor(255, 0, 0, 128));
    //painter.setPen(QColor(255, 255, 255, 255));
    //painter.drawText(RANGE_OFFSET + mid - 330, varHeight + 5, QString("Selection Mean < Global Mean"));
}

bool BarChart::eventFilter(QObject* target, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        QPoint mousePos = mouseEvent->pos();

        for (int j = 0; j < _explanationModel.getDataset().numDimensions(); j++)
        {
            if (isMouseOverBox(mousePos, 10, TOP_MARGIN + 16 * j, 14))
            {
                if (_sortIndices.size() > 0)
                    emit dimensionExcluded(_sortIndices[j]);
                else
                    emit dimensionExcluded(j);
            }
        }

        if (_explanationModel.hasDataset())
        {
            int numDimensions = _explanationModel.getDataset().numDimensions();
            int legendY = _drawLegend ? std::min(TOP_MARGIN + 8 + 16 * (numDimensions + 2), height() - 270) : height() - 30;

            if (isMouseOverBox(mousePos, LEGEND_BUTTON - 4, legendY + 10 - 4, 18)) // Padded by 4 pixels all sides
            {
                _drawLegend = !_drawLegend;
                update();
            }
        }

        qDebug() << "Mouse button press";
        break;
    }
    case QEvent::MouseMove:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);


        _mousePos = QPoint(mouseEvent->position().x(), mouseEvent->position().y());
        update();

        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(target, event);
}

ImageViewWidget::ImageViewWidget() :
    _image(30, 30, QImage::Format::Format_ARGB32)
{
    setFixedSize(100, 100);
}

void ImageViewWidget::setImage(QImage image)
{
    _image = image;
}

void ImageViewWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    painter.drawImage(0, 0, _image);
}

ExplanationWidget::ExplanationWidget(ExplanationModel& explanationModel)
{
    _rankLabel = new QLabel("No ranking");
    _barChart = new BarChart(explanationModel);
    _imageViewWidget = new ImageViewWidget();

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_barChart);
    //layout->addWidget(_imageViewWidget);
    //layout->addWidget(_rankLabel);
    QPushButton* noSortButton = new QPushButton("No Ranking");
    QPushButton* varianceSortButton = new QPushButton("Variance Ranking");
    QPushButton* valueSortButton = new QPushButton("Value Ranking");

    _rankingCombobox = new QComboBox();
    _rankingCombobox->addItem("No Ranking");
    _rankingCombobox->addItem("Variance Ranking");
    _rankingCombobox->addItem("Value Ranking");

    connect(noSortButton, &QPushButton::pressed, _barChart, &BarChart::sortByDefault);
    connect(varianceSortButton, &QPushButton::pressed, _barChart, &BarChart::sortByVariance);
    connect(valueSortButton, &QPushButton::pressed, _barChart, &BarChart::sortByValue);
    //layout->addWidget(noSortButton);
    //layout->addWidget(varianceSortButton);
    //layout->addWidget(valueSortButton);

    //layout->addWidget(_rankingCombobox);
    //layout->addStretch();

    // Neighbourhood slider UI
    {
        QGroupBox* groupBox = new QGroupBox(tr("Global neighbourhood size"));

        QVBoxLayout* vBoxLayout = new QVBoxLayout();
        QHBoxLayout* hBoxLayout = new QHBoxLayout();
        QLabel* radiusSliderLabel = new QLabel("Neighbourhood Size:");
        _radiusSliderValueLabel = new QLabel("");
        _radiusSlider = new QSlider(Qt::Orientation::Horizontal);
        _radiusSlider->setRange(0, 50);
        connect(_radiusSlider, &QSlider::valueChanged, this, &ExplanationWidget::neighbourhoodRadiusValueChanged);
        _radiusSlider->setValue(10);

        hBoxLayout->addWidget(radiusSliderLabel);
        hBoxLayout->addWidget(_radiusSliderValueLabel);
        hBoxLayout->setContentsMargins(0, 6, 0, 6);
        vBoxLayout->addLayout(hBoxLayout);
        vBoxLayout->addWidget(_radiusSlider);
        groupBox->setLayout(vBoxLayout);
        layout->addWidget(groupBox);
    }

    // Projection coloring UI
    {
        QPushButton* _varianceColoringButton = new QPushButton("Color Projection by Variance Ranking");
        QPushButton* _valueColoringButton = new QPushButton("Color Projection by Value Ranking");

        _varianceColoringButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        _valueColoringButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        _varianceColoringButton->setMaximumHeight(50);
        _valueColoringButton->setMaximumHeight(50);

        connect(_varianceColoringButton, &QPushButton::pressed, [&explanationModel]() {
            explanationModel.setExplanationMetric(Explanation::Metric::VARIANCE);
        });
        connect(_valueColoringButton, &QPushButton::pressed, [&explanationModel]() {
            explanationModel.setExplanationMetric(Explanation::Metric::VALUE);
        });

        QHBoxLayout* boxLayout = new QHBoxLayout();
        boxLayout->addWidget(_varianceColoringButton);
        boxLayout->addWidget(_valueColoringButton);
        layout->addLayout(boxLayout);
    }

    setLayout(layout);

    setMaximumWidth(460);
}

ExplanationWidget::~ExplanationWidget()
{

}

void ExplanationWidget::update()
{
    _barChart->repaint();
    _imageViewWidget->repaint();
}

void ExplanationWidget::neighbourhoodRadiusValueChanged(int value)
{
    // Display value of slider (times two, because explaining neighbourhood size is easier than radius)
    _radiusSliderValueLabel->setText(QString::number(value*2) + QString("% of projection size"));
}
