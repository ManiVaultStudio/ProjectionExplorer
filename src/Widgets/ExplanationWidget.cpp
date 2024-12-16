#include "ExplanationWidget.h"

#include <QVBoxLayout>

#include <QPainter>

HistogramChart::HistogramChart(QWidget* parent, Explanation::Model& explanationModel) :
    QWidget(parent),
    _explanationModel(explanationModel)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(500);
    setContentsMargins(0, 0, 0, 0);
}

void HistogramChart::computeGlobalHistograms()
{
    DataMatrix& dataset = _explanationModel.getDataset();
    int numDimensions = dataset.getNumCols();

    _globalHistograms.clear();
    _globalHistograms.resize(numDimensions, Histogram(20));

    // Compute global histograms
    for (int j = 0; j < numDimensions; j++)
    {
        _globalHistograms[j].setRange(_explanationModel.getDataStatistics().minRange[j], _explanationModel.getDataStatistics().maxRange[j]);
        for (int i = 0; i < dataset.getNumRows(); i++)
        {
            _globalHistograms[j].addDataValue(dataset(i, j));
        }
    }
}

void HistogramChart::setRanking(const std::vector<unsigned int>& selection)
{
    int numDimensions = _explanationModel.getSelectionDimRanking().size();

    _localHistograms.clear();
    _localHistograms.resize(numDimensions, Histogram(20));

    if (selection.empty())
    {
        _sortIndices.clear();
        _sortIndices.resize(numDimensions);
        std::iota(_sortIndices.begin(), _sortIndices.end(), 0);
        return;
    }

    DataMatrix& dataset = _explanationModel.getDataset();

    // Compute local histograms
    for (int j = 0; j < numDimensions; j++)
    {
        _localHistograms[j].setRange(_explanationModel.getDataStatistics().minRange[j], _explanationModel.getDataStatistics().maxRange[j]);
        for (int i = 0; i < selection.size(); i++)
        {
            int si = selection[i];

            _localHistograms[j].addDataValue(dataset(si, j));
        }
    }

    // Compute sorting
    _sortIndices.clear();
    _sortIndices.resize(numDimensions);
    std::iota(_sortIndices.begin(), _sortIndices.end(), 0);

    std::sort(_sortIndices.begin(), _sortIndices.end(), [&](int i, int j) {return _explanationModel.getSelectionDimRanking()[i] > _explanationModel.getSelectionDimRanking()[j]; });
}

void HistogramChart::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    //if (!_explanationModel.hasDataset())
    //{
    //    return;
    //}

    std::cout << ">>>>>>>>>>>>>>>>>>>>>>> BarChart::paintEvent" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    const DataMatrix& dataset = _explanationModel.getDataset();
    //const DataStatistics& dataStats = _explanationModel.getDataStatistics();

    int numDimensions = dataset.getNumCols();

    const ColorMapping& colorMapping = _explanationModel.getColorMapping();

    int BOX_HEIGHT = 64;
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);

    painter.fillRect(rect(), Qt::black);

    painter.fillRect(0, 0, 600, (numDimensions + 2) * BOX_HEIGHT, QColor(38, 38, 38));

    QFont font = QFont("MS Shell Dlg 2", 10, QFont::ExtraBold);
    QFontMetricsF fm(font);
    font.setPixelSize(16);
    painter.setFont(font);

    float RANGE_WIDTH = 200;
    float RANGE_OFFSET = 150;
    float TOP_MARGIN = 30;

    int maxDimensionShown = height() / BOX_HEIGHT;
    qDebug() << "Max dims shown: " << maxDimensionShown;

    auto& dimNames = _explanationModel.getDataset().getDimensionNames();
    if (dimNames.empty())
        return;

    for (int i = 0; i < std::min<int>(numDimensions, maxDimensionShown); i++)
    {
        int sortIndex = _sortIndices.size() > 0 ? _sortIndices[i] : i;

        QColor color(180, 180, 180, 255);
        if (sortIndex < colorMapping.getColors().size())
            color = colorMapping.getColors()[sortIndex];
        
        painter.setPen(color);

        QString dimName = _explanationModel.getDataset().getDimensionNames()[sortIndex];
        dimName = fm.elidedText(dimName, Qt::TextElideMode::ElideRight, 150);
        painter.drawText(30, TOP_MARGIN + 10 + i * BOX_HEIGHT, dimName);

        painter.drawLine(RANGE_OFFSET, TOP_MARGIN + BOX_HEIGHT * i + 0, RANGE_OFFSET + RANGE_WIDTH, TOP_MARGIN + BOX_HEIGHT * i + 0);

        painter.fillRect(10, TOP_MARGIN + BOX_HEIGHT * i, 14, 14, color);

        if (_localHistograms.size() > 0) // TEMP
        {
            Histogram& globalHist = _globalHistograms[sortIndex];
            Histogram& hist = _localHistograms[sortIndex];
            int globalNumPoints = globalHist.getNumDataPoints();
            int localNumPoints = hist.getNumDataPoints();
            int globalHighestBinValue = globalHist.getHighestBinValue();
            int localHighestBinValue = hist.getHighestBinValue();

            // Draw global histograms
            float globalBoxWidth = (float)RANGE_WIDTH / globalHist.getBins().size();
            for (int b = 0; b < globalHist.getBins().size(); b++)
            {
                float binHeight = ((float)globalHist.getBins()[b] / globalHighestBinValue);
                if (std::isnan(binHeight)) binHeight = 0;
                painter.fillRect(RANGE_OFFSET + b * globalBoxWidth, TOP_MARGIN + BOX_HEIGHT * i + 0, globalBoxWidth, binHeight * (BOX_HEIGHT-2) / 2, QColor(180, 180, 180));
            }

            // Draw local histograms
            float boxWidth = (float)RANGE_WIDTH / hist.getBins().size();
            for (int b = 0; b < hist.getBins().size(); b++)
            {
                float binHeight = ((float)hist.getBins()[b] / localHighestBinValue);
                if (std::isnan(binHeight)) binHeight = 0;
                painter.fillRect(RANGE_OFFSET + b * boxWidth, TOP_MARGIN + BOX_HEIGHT * i + 0, boxWidth, -binHeight * (BOX_HEIGHT-2) / 2, QColor(0, 180, 225));
            }
        }
    }
}

ExplanationWidget::ExplanationWidget(Explanation::Model& explanationModel) :
    _explanationModel(explanationModel),
    _histogramChart(new HistogramChart(this, explanationModel))
{
    setMinimumHeight(500);
    setMaximumWidth(400);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_histogramChart);

    setLayout(layout);
}

void ExplanationWidget::updateWidgets()
{
    _histogramChart->update();
}
