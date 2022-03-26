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

void DataMetrics::compute(const hdps::Dataset<Points>& dataset, const std::vector<unsigned int>& selection, const std::vector<float>& minRanges, const std::vector<float>& maxRanges)
{
    int numDimensions = dataset->getNumDimensions();

    // Compute average values
    averageValues.clear();
    averageValues.resize(numDimensions, 0);
    for (int j = 0; j < numDimensions; j++)
    {
        for (int i = 0; i < selection.size(); i++)
        {
            int si = selection[i];

            averageValues[j] += dataset->getValueAt(si * numDimensions + j);
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
            float x = dataset->getValueAt(si * numDimensions + j) - averageValues[j];
            variances[j] += x * x;
        }
        variances[j] /= selection.size();
        variances[j] = sqrt(variances[j]);
    }

    // Normalization
    for (int j = 0; j < numDimensions; j++)
    {
        float normalizedValue = (averageValues[j] - minRanges[j]) / (maxRanges[j] - minRanges[j]);
        averageValues[j] = normalizedValue;
        float normalizedStddev = variances[j] / (maxRanges[j] - minRanges[j]);
        variances[j] = normalizedStddev;

        //std::cout << "Dim: " << j << " Min ranges: " << minRanges[j] << " Max ranges: " << maxRanges[j] << std::endl;
    }
}

BarChart::BarChart()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(270);
    setMinimumWidth(460);
    _colors.resize(22);
    //const char* kelly_colors[] = { "#31a09a", "#FFFFFF", "#F3C300", "#875692", "#F38400", "#A1CAF1", "#BE0032", "#C2B280", "#848482", "#008856", "#E68FAC", "#0067A5", "#F99379", "#604E97", "#F6A600", "#B3446C", "#DCD300", "#882D17", "#8DB600", "#654522", "#E25822", "#9B3D96", "#A13237" };
    const char* kelly_colors[] = { "#31a09a", "#59a14f", "#F3C300", "#875692", "#F38400", "#A1CAF1", "#BE0032", "#C2B280", "#848482", "#008856", "#E68FAC", "#0067A5", "#F99379", "#604E97", "#F6A600", "#B3446C", "#DCD300", "#882D17", "#8DB600", "#654522", "#E25822", "#2B3D26", "#A13237" };

    const char* tab_colors[] = { "#4e79a7", "#59a14f", "#9c755f", "#f28e2b", "#edc948", "#bab0ac", "#e15759", "#b07aa1", "#76b7b2", "#ff9da7" };

    for (int i = 0; i < _colors.size(); i++)
    {
        _colors[i].setNamedColor(kelly_colors[i]);
    }
}

void BarChart::setDataset(hdps::Dataset<Points> dataset)
{
    _dataset = dataset;

    // Compute ranges per dimension
    _minRanges.resize(dataset->getNumDimensions());
    _maxRanges.resize(dataset->getNumDimensions());

    for (int j = 0; j < dataset->getNumDimensions(); j++)
    {
        _minRanges[j] = std::numeric_limits<float>::max();
        _maxRanges[j] = -std::numeric_limits<float>::max();

        for (int i = 0; i < dataset->getNumPoints(); i++)
        {
            float value = dataset->getValueAt(i * dataset->getNumDimensions() + j);

            if (value < _minRanges[j]) _minRanges[j] = value;
            if (value > _maxRanges[j]) _maxRanges[j] = value;
        }
    }
}

void BarChart::setRanking(Eigen::ArrayXXf& ranking, const std::vector<unsigned int>& selection)
{
    int numPoints = ranking.rows();
    int numDimensions = ranking.cols();

    _selection = selection;

    //std::cout << "Ranking: " << numPoints << "numdims: " << numDimensions << std::endl;
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

    // Compute metrics on current selection
    _newMetrics.compute(_dataset, _selection, _minRanges, _maxRanges);

    _dimAggregation = dimAggregation;

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
    _oldMetrics.compute(_dataset, oldSelection, _minRanges, _maxRanges);
}

void BarChart::setImportantDims(const std::vector<float>& importantDims)
{
    _importantDims = importantDims;
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
    if (!_dataset.isValid())
        return;

    int numDimensions = _dataset->getNumDimensions();

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

                    float normValueA = (_dataset->getValueAt(si * numDimensions + sortIndexA) - _minRanges[sortIndexA]) / (_maxRanges[sortIndexA] - _minRanges[sortIndexA]);
                    float normValueB = (_dataset->getValueAt(si * numDimensions + sortIndexB) - _minRanges[sortIndexB]) / (_maxRanges[sortIndexB] - _minRanges[sortIndexB]);

                    painter.drawLine(RANGE_OFFSET + normValueA * RANGE_WIDTH, TOP_MARGIN + 16 * j + 8, RANGE_OFFSET + normValueB * RANGE_WIDTH, TOP_MARGIN + 16 * (j + 1) + 8);
                }
            }
        }

        for (int i = 0; i < _dimAggregation.size(); i++)
        {
            int sortIndex = _sortIndices[i];

            QColor color(180, 180, 180, 255);
            if (i < 22)
                color = _colors[sortIndex];

            painter.fillRect(10, TOP_MARGIN + 16 * i, 14, 14, color);
            painter.setPen(color);
            if (_dataset->getDimensionNames().size() > 0)
            {
                painter.drawText(30, TOP_MARGIN + 10 + 16 * i, _dataset->getDimensionNames()[sortIndex]);
            }
            else
            {
                painter.drawText(30, TOP_MARGIN + 10 + 16 * i, QString::number(sortIndex));
            }
            
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
    else if (_dataset.isValid())
    {
        for (int i = 0; i < numDimensions; i++)
        {
            QColor color(180, 180, 180);

            painter.fillRect(10, TOP_MARGIN + 16 * i, 14, 14, color);
            painter.setPen(color);
            if (_dataset->getDimensionNames().size() > 0)
            {
                painter.drawText(30, TOP_MARGIN + 10 + 16 * i, _dataset->getDimensionNames()[i]);
            }
            else
            {
                painter.drawText(30, TOP_MARGIN + 10 + 16 * i, QString::number(i));
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

ExplanationWidget::ExplanationWidget()
{
    _rankLabel = new QLabel("No ranking");
    _barChart = new BarChart();
    _imageViewWidget = new ImageViewWidget();

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(_barChart);
    //layout->addWidget(_imageViewWidget);
    //layout->addWidget(_rankLabel);
    QPushButton* noSortButton = new QPushButton("No Sorting");
    QPushButton* varianceSortButton = new QPushButton("Variance Sort");
    QPushButton* valueSortButton = new QPushButton("Value Sort");

    _varianceColoringButton = new QPushButton("Color by Variance");
    _valueColoringButton = new QPushButton("Color by Value");

    _radiusSlider = new QSlider(Qt::Orientation::Horizontal);
    _radiusSlider->setRange(0, 100);
    _radiusSlider->setValue(10);
    connect(noSortButton, &QPushButton::pressed, _barChart, &BarChart::sortByDefault);
    connect(varianceSortButton, &QPushButton::pressed, _barChart, &BarChart::sortByVariance);
    connect(valueSortButton, &QPushButton::pressed, _barChart, &BarChart::sortByValue);
    layout->addWidget(noSortButton);
    layout->addWidget(varianceSortButton);
    layout->addWidget(valueSortButton);
    layout->addWidget(_radiusSlider);
    layout->addStretch();
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

void ExplanationWidget::setRanking(Eigen::ArrayXXf& ranking)
{
    //if (ranking.rows() == 0)
    //    return;

    //std::string rankText;

    //std::vector<float> dimAggregation(ranking.cols(), 0);
    //for (int i = 0; i < ranking.rows(); i++)
    //{
    //    for (int j = 0; j < ranking.cols(); j++)
    //    {
    //        dimAggregation[j] += ranking(i, j);
    //    }
    //}
    //for (int i = 0; i < dimAggregation.size(); i++)
    //{
    //    dimAggregation[i] /= (float)ranking.rows();
    //}

    //for (int i = 0; i < dimAggregation.size(); i++)
    //    rankText += std::to_string(dimAggregation[i]) + " ";

    //_rankLabel->setText(rankText.c_str());

    //_barChart->setRanking(ranking);
}
