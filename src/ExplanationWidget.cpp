#include "ExplanationWidget.h"

#include <QVBoxLayout>

#include <string>
#include <iostream>

BarChart::BarChart()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(270);
    setMinimumWidth(400);
    _colors.resize(23);
    const char* kelly_colors[] = { "#31a09a", "#222222", "#F3C300", "#875692", "#F38400", "#A1CAF1", "#BE0032", "#C2B280", "#848482", "#008856", "#E68FAC", "#0067A5", "#F99379", "#604E97", "#F6A600", "#B3446C", "#DCD300", "#882D17", "#8DB600", "#654522", "#E25822", "#2B3D26", "#A13237" };

    for (int i = 0; i < _colors.size(); i++)
    {
        _colors[i].setNamedColor(kelly_colors[i]);
    }
}

void BarChart::setDataset(hdps::Dataset<Points> dataset)
{
    _dataset = dataset;
}

void BarChart::setRanking(Eigen::ArrayXXf& ranking)
{
    std::cout << "Ranking: " <<  ranking.rows() << std::endl;
    if (ranking.rows() == 0)
    {
        _dimAggregation.clear();
        return;
    }

    std::vector<float> dimAggregation(ranking.cols(), 0);
    for (int i = 0; i < ranking.rows(); i++)
    {
        for (int j = 0; j < ranking.cols(); j++)
        {
            dimAggregation[j] += ranking(i, j);
        }
    }
    for (int i = 0; i < dimAggregation.size(); i++)
    {
        dimAggregation[i] /= (float) ranking.rows();
    }

    _dimAggregation = dimAggregation;

    // Compute sorting
    _sortIndices.resize(dimAggregation.size());
    std::iota(_sortIndices.begin(), _sortIndices.end(), 0);
    std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return dimAggregation[i] < dimAggregation[j]; });
}

void BarChart::setImportantDims(const std::vector<float>& importantDims)
{
    _importantDims = importantDims;
}

void BarChart::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    if (_dataset.isValid())
        painter.fillRect(0, 0, 400, (_dataset->getNumDimensions() + 1) * 20, QColor(38, 38, 38));

    if (_dimAggregation.size() > 0)
    {
        for (int i = 0; i < _dimAggregation.size(); i++)
        {
            int sortIndex = _sortIndices[i];
            painter.fillRect(10, 10 + 16 * i, 14, 14, _colors[sortIndex]);
            painter.setPen(QColor(255, 255, 255));
            if (_dataset->getDimensionNames().size() > 0)
            {
                painter.drawText(30, 20 + 16 * i, _dataset->getDimensionNames()[sortIndex]);
            }
            else
            {
                painter.drawText(30, 20 + 16 * i, QString::number(sortIndex));
            }
        
            painter.fillRect(180, 10 + 16 * i, _dimAggregation[sortIndex] * 600, 14, QColor(255, 255, 255));
            //painter.fillRect(50, 16 * i + 7, _importantDims[i] * 60, 7, QColor(0, 0, 255));
        }
    }
    else if (_dataset.isValid())
    {
        for (int i = 0; i < _dataset->getNumDimensions(); i++)
        {
            painter.fillRect(10, 10 + 16 * i, 14, 14, _colors[i]);
            painter.setPen(QColor(255, 255, 255));
            if (_dataset->getDimensionNames().size() > 0)
            {
                painter.drawText(30, 20 + 16 * i, _dataset->getDimensionNames()[i]);
            }
            else
            {
                painter.drawText(30, 20 + 16 * i, QString::number(i));
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
    layout->addWidget(_imageViewWidget);
    layout->addWidget(_rankLabel);

    setLayout(layout);

    setMaximumWidth(300);
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
