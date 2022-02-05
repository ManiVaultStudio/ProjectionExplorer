#include "ExplanationWidget.h"

#include <QVBoxLayout>

#include <string>
#include <iostream>

BarChart::BarChart()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(270);

    _colors.resize(23);
    const char* kelly_colors[] = { "#31a09a", "#222222", "#F3C300", "#875692", "#F38400", "#A1CAF1", "#BE0032", "#C2B280", "#848482", "#008856", "#E68FAC", "#0067A5", "#F99379", "#604E97", "#F6A600", "#B3446C", "#DCD300", "#882D17", "#8DB600", "#654522", "#E25822", "#2B3D26", "#A13237" };

    for (int i = 0; i < _colors.size(); i++)
    {
        _colors[i].setNamedColor(kelly_colors[i]);
    }
}

void BarChart::setRanking(Eigen::ArrayXXf& ranking)
{
    if (ranking.rows() == 0)
        return;

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
}

void BarChart::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    painter.drawRect(0, 0, 200, (_dimAggregation.size() + 1) * 16);

    for (int i = 0; i < _dimAggregation.size(); i++)
    {
        painter.fillRect(10, 16 * i, 14, 14, _colors[i]);
        painter.drawText(30, 10 + 16 * i, QString::number(i));
        painter.fillRect(50, 16 * i, _dimAggregation[i] * 600, 14, QColor(255, 0, 0));
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
    if (ranking.rows() == 0)
        return;

    std::string rankText;

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
        dimAggregation[i] /= (float)ranking.rows();
    }

    for (int i = 0; i < dimAggregation.size(); i++)
        rankText += std::to_string(dimAggregation[i]) + " ";

    _rankLabel->setText(rankText.c_str());

    _barChart->setRanking(ranking);
}
