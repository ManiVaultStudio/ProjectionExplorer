#include "ExplanationWidget.h"

#include <QVBoxLayout>
#include <QPushButton>

#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>

#define RANGE_OFFSET 250
#define RANGE_WIDTH 200
#define TOP_MARGIN 30

void DataMetrics::compute(const DataMatrix& dataset, const std::vector<unsigned int>& selection, const DataMatrix& dataRanges)
{
    int numDimensions = dataset.cols();

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
        float normalizedValue = (averageValues[j] - dataRanges(0, j)) / (dataRanges(1, j) - dataRanges(0, j));
        averageValues[j] = normalizedValue;
        float normalizedStddev = variances[j] / (dataRanges(1, j) - dataRanges(0, j));
        variances[j] = normalizedStddev;

        //std::cout << "Dim: " << j << " Min ranges: " << minRanges[j] << " Max ranges: " << maxRanges[j] << std::endl;
    }
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
    _newMetrics.compute(_explanationModel.getDataset(), _selection, _explanationModel.getDataRanges());

    // Compute sorting
    _sortIndices.resize(dimAggregation.size());
    std::iota(_sortIndices.begin(), _sortIndices.end(), 0);
    if (_sortingType == SortingType::VARIANCE)
        std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return dimAggregation[i] < dimAggregation[j]; });
    else if (_sortingType == SortingType::VALUE)
        std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return _newMetrics.averageValues[i] > _newMetrics.averageValues[j]; });
    else if (_sortingType == SortingType::NONE) {}
}

void BarChart::computeOldMetrics(const std::vector<unsigned int>& oldSelection)
{
    _oldMetrics.compute(_explanationModel.getDataset(), oldSelection, _explanationModel.getDataRanges());
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
        return;

    const DataMatrix& dataset = _explanationModel.getDataset();
    const DataMatrix& dataRanges = _explanationModel.getDataRanges();

    int numDimensions = dataset.cols();

    const std::vector<QColor>& palette = _explanationModel.getPalette();

    QPainter painter(this);
    //painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    painter.fillRect(0, 0, 600, (numDimensions + 1) * 20, QColor(38, 38, 38));

    painter.setPen(Qt::white);
    painter.setFont(QFont("Open Sans", 10, QFont::ExtraBold));
    switch (_sortingType)
    {
    case SortingType::VARIANCE: painter.drawText(10, 20, "Variance Sort"); break;
    case SortingType::VALUE: painter.drawText(10, 20, "Value Sort"); break;
    default: painter.drawText(10, 20, "No Sorting");
    }

    if (_dimAggregation.size() > 0)
    {
        if (!_differentialRanking)
        {
            // Draw PCP
            int alpha = std::max<int>(255.0f / _selection.size(), 1);
            painter.setPen(QColor(0, 255, 0, 50));

            for (const unsigned int& si : _selection)
            {
                for (int j = 0; j < numDimensions - 1; j++)
                {
                    int sortIndexA = _sortIndices[j];
                    int sortIndexB = _sortIndices[j + 1];

                    float normValueA = (dataset(si, sortIndexA) - dataRanges(0, sortIndexA)) / (dataRanges(1, sortIndexA) - dataRanges(0, sortIndexA));
                    float normValueB = (dataset(si, sortIndexB) - dataRanges(0, sortIndexB)) / (dataRanges(1, sortIndexB) - dataRanges(0, sortIndexB));

                    painter.drawLine(RANGE_OFFSET + normValueA * RANGE_WIDTH, TOP_MARGIN + 16 * j + 8, RANGE_OFFSET + normValueB * RANGE_WIDTH, TOP_MARGIN + 16 * (j + 1) + 8);
                }
            }
        }

        for (int i = 0; i < _dimAggregation.size(); i++)
        {
            int sortIndex = _sortIndices[i];

            QColor color(180, 180, 180, 255);
            if (i < palette.size())
                color = palette[sortIndex];

            painter.fillRect(10, TOP_MARGIN + 16 * i, 14, 14, color);
            painter.setPen(color);

            // Draw dimension names
            painter.drawText(30, TOP_MARGIN + 10 + 16 * i, _explanationModel.getDataNames()[sortIndex]);

            //float normalizedValue = (_averageValues[sortIndex] - _minRanges[sortIndex]) / (_maxRanges[sortIndex] - _minRanges[sortIndex]);

            //painter.fillRect(180, 10 + 16 * i, std::max(0.0f, 0.166f - _dimAggregation[sortIndex]) * 600, 14, color);
            // 
            //painter.fillRect(180, 10 + 16 * i, std::max(0.0f, _dimAggregation[sortIndex]) * 600, 14, _colors[sortIndex]);

            //painter.fillRect(180, 10 + 16 * i, _dimAggregation[sortIndex] * 600, 7, QColor(0, 255, 0));
            //painter.fillRect(50, 16 * i + 7, _importantDims[i] * 60, 7, QColor(0, 0, 255));

            //// Draw values
            // Draw range line
            painter.drawLine(RANGE_OFFSET, TOP_MARGIN + 16 * i + 8, RANGE_OFFSET + RANGE_WIDTH, TOP_MARGIN + 16 * i + 8);

            if (!_differentialRanking)
            {
                // Draw mean
                float mean = _newMetrics.averageValues[sortIndex];
                painter.setPen(QColor(255, 0, 0, 255));
                painter.drawLine(RANGE_OFFSET + mean * RANGE_WIDTH, TOP_MARGIN + 16 * i, RANGE_OFFSET + mean * RANGE_WIDTH, TOP_MARGIN + 16 * i + 16);
                // Draw variance
                painter.setPen(QColor(255, 255, 255, 255));
                painter.drawLine(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i, RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i);
                painter.drawLine(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 2 + 16 * i, RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 14 + 16 * i);
                painter.drawLine(RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 2 + 16 * i, RANGE_OFFSET + (mean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 14 + 16 * i);
                //painter.drawRect(RANGE_OFFSET + (mean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, 10 + 16 * i, 2 * _newMetrics.variances[sortIndex] * RANGE_WIDTH, 16);
            }
            else
            {
                // Draw mean
                float oldMean = _oldMetrics.averageValues[sortIndex];
                float newMean = _newMetrics.averageValues[sortIndex];

                int oldMeanX = RANGE_OFFSET + oldMean * RANGE_WIDTH;
                int newMeanX = RANGE_OFFSET + newMean * RANGE_WIDTH;

                QColor diffColor = newMeanX - oldMeanX >= 0 ? QColor(0, 255, 0, 128) : QColor(255, 0, 0, 128);
                painter.fillRect(oldMeanX, TOP_MARGIN + 16 * i, newMeanX - oldMeanX, 12, diffColor);

                painter.setPen(QColor(180, 180, 180, 255));
                painter.drawLine(oldMeanX, TOP_MARGIN + 16 * i, oldMeanX, TOP_MARGIN + 16 * i + 16);
                painter.setPen(QColor(255, 0, 0, 255));
                painter.drawLine(newMeanX, TOP_MARGIN + 16 * i, newMeanX, TOP_MARGIN + 16 * i + 16);

                qDebug() << newMeanX << oldMeanX << newMean << oldMean;

                // Draw variance
                painter.setPen(QColor(255, 255, 255, 255));
                //painter.drawRect(RANGE_OFFSET + (newMean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, 10 + 16 * i, 2 * _newMetrics.variances[sortIndex] * RANGE_WIDTH, 16);

                painter.drawLine(RANGE_OFFSET + (newMean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i, RANGE_OFFSET + (newMean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 8 + 16 * i);
                painter.drawLine(RANGE_OFFSET + (newMean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 2 + 16 * i, RANGE_OFFSET + (newMean - _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 14 + 16 * i);
                painter.drawLine(RANGE_OFFSET + (newMean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 2 + 16 * i, RANGE_OFFSET + (newMean + _newMetrics.variances[sortIndex]) * RANGE_WIDTH, TOP_MARGIN + 14 + 16 * i);
            }
        }
    }
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
    layout->addWidget(_barChart);
    //layout->addWidget(_imageViewWidget);
    //layout->addWidget(_rankLabel);
    QPushButton* noSortButton = new QPushButton("No Ranking");
    QPushButton* varianceSortButton = new QPushButton("Variance Ranking");
    QPushButton* valueSortButton = new QPushButton("Value Ranking");

    QPushButton* _varianceColoringButton = new QPushButton("Color by Variance Ranking");
    QPushButton* _valueColoringButton = new QPushButton("Color by Value Ranking");

    connect(_varianceColoringButton, &QPushButton::pressed, [&explanationModel](){
        explanationModel.setExplanationMetric(Explanation::Metric::VARIANCE);
    });
    connect(_valueColoringButton, &QPushButton::pressed, [&explanationModel]() {
        explanationModel.setExplanationMetric(Explanation::Metric::VALUE);
    });

    QLabel* radiusSliderLabel = new QLabel("Neighbourhood Size:");
    _radiusSlider = new QSlider(Qt::Orientation::Horizontal);
    _radiusSlider->setRange(0, 50);
    _radiusSlider->setValue(10);

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
    layout->addWidget(radiusSliderLabel);
    layout->addWidget(_radiusSlider);
    layout->addWidget(_rankingCombobox);
    //layout->addStretch();
    layout->addWidget(_varianceColoringButton);
    layout->addWidget(_valueColoringButton);

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
