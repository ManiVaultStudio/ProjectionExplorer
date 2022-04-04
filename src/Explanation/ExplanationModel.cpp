#include "ExplanationModel.h"

#include <iostream>

namespace
{
    void convertToEigenMatrix(Dataset<Points> dataset, DataMatrix& dataMatrix)
    {
        int numPoints = dataset->getNumPoints();
        int numDimensions = dataset->getNumDimensions();

        dataMatrix.resize(numPoints, numDimensions);
        for (int j = 0; j < numDimensions; j++)
        {
            std::vector<float> result;
            dataset->extractDataForDimension(result, j);
            for (int i = 0; i < result.size(); i++)
                dataMatrix(i, j) = result[i];
        }
    }

    float computeProjectionDiameter(const Eigen::ArrayXXf& projection)
    {
        float minX = std::numeric_limits<float>::max(), maxX = -std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max(), maxY = -std::numeric_limits<float>::max();
        for (int i = 0; i < projection.rows(); i++)
        {
            float x = projection(i, 0);
            float y = projection(i, 1);

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
        float rangeX = maxX - minX;
        float rangeY = maxY - minY;

        float diameter = rangeX > rangeY ? rangeX : rangeY;
        return diameter;
    }

    void computeDatasetRanges(const DataMatrix& dataset, DataMatrix& dataRanges)
    {
        // Store dimension minimums in row 0 and maximums in row 1
        dataRanges.resize(2, dataset.cols());
        dataRanges.row(0).setConstant(std::numeric_limits<float>::max());
        dataRanges.row(1).setConstant(-std::numeric_limits<float>::max());

        for (int j = 0; j < dataset.cols(); j++)
        {
            for (int i = 0; i < dataset.rows(); i++)
            {
                float value = dataset(i, j);

                if (value < dataRanges(0, j)) dataRanges(0, j) = value;
                if (value > dataRanges(1, j)) dataRanges(1, j) = value;
            }
        }
        const Eigen::IOFormat fmt(2, Eigen::DontAlignCols, "\t", " ", "", "", "", "");
        std::cout << dataRanges.row(0).transpose().format(fmt) << "\t|" << dataRanges.row(1).transpose().format(fmt) << std::endl;
    }

    void findNeighbourhood(const Eigen::ArrayXXf& projection, int centerId, float radius, std::vector<int>& neighbourhood)
    {
        float x = projection(centerId, 0);
        float y = projection(centerId, 1);

        float radSquared = radius * radius;

        neighbourhood.clear();

        for (int i = 0; i < projection.rows(); i++)
        {
            float xd = projection(i, 0) - x;
            float yd = projection(i, 1) - y;

            float magSquared = xd * xd + yd * yd;

            if (magSquared > radSquared)
                continue;

            neighbourhood.push_back(i);
        }
    }
}

ExplanationModel::ExplanationModel() :
    _hasDataset(false),
    _projectionDiameter(1),
    _explanationMetric(Explanation::Metric::VARIANCE)
{
    // Initialize color palette
    _palette.resize(20); // "#31a09a", "#59a14f", "#A13237"
    const char* kelly_colors[] = { "#F3C300", "#875692", "#F38400", "#A1CAF1", "#BE0032", "#C2B280", "#59a14f", "#008856", "#E68FAC", "#0067A5", "#F99379", "#604E97", "#F6A600", "#B3446C", "#DCD300", "#882D17", "#8DB600", "#654522", "#E25822", "#2B3D26" };
    for (int i = 0; i < _palette.size(); i++)
    {
        _palette[i].setNamedColor(kelly_colors[i]);
    }
}

void ExplanationModel::setDataset(Dataset<Points> dataset, Dataset<Points> projection)
{
    // Convert the dataset and projection to eigen matrices
    convertToEigenMatrix(dataset, _dataset);
    convertToEigenMatrix(projection, _projection);

    // Compute projection diameter
    _projectionDiameter = computeProjectionDiameter(_projection);
    std::cout << "Diameter: " << _projectionDiameter << std::endl;

    // Norm dataset
    _normDataset = _dataset;
    auto means = _normDataset.colwise().mean();
    _normDataset.rowwise() -= means;

    for (int j = 0; j < _normDataset.cols(); j++)
    {
        float variance = 0;
        for (int i = 0; i < _normDataset.rows(); i++)
        {
            variance += _normDataset(i, j) * _normDataset(i, j);
        }
        variance /= _normDataset.rows() - 1;
        float stddev = sqrt(variance);
        _normDataset.col(j) /= stddev;
    }

    // Compute ranges per dimension
    computeDatasetRanges(_dataset, _dataRanges);

    // Store dimension names
    if (dataset->getDimensionNames().size() > 0)
    {
        _dimensionNames = dataset->getDimensionNames();
    }
    else
    {
        for (int j = 0; j < dataset->getNumDimensions(); j++)
        {
            _dimensionNames.push_back(QString("Dim " + QString::number(j)));
        }
    }

    // Create color mapping
    _colorMapping.resize(_dataset.cols());
    for (int i = 0; i < _colorMapping.size(); i++)
    {
        QColor color = (i < _palette.size()) ? _palette[i] : QColor(180, 180, 180);
        _colorMapping[i] = color;
    }

    _hasDataset = true;
}

void ExplanationModel::recomputeNeighbourhood(float neighbourhoodRadius)
{
    computeNeighbourhoodMatrix(_neighbourhoodMatrix, _projectionDiameter * neighbourhoodRadius);
}

void ExplanationModel::recomputeMetrics()
{
    Explanation::Method* explanationMethod = getCurrentExplanationMethod();

    if (explanationMethod != nullptr)
        explanationMethod->recompute(_dataset, _neighbourhoodMatrix);
}

void ExplanationModel::recomputeColorMapping(DataMatrix& dimRanks)
{
    bool lowRankBest = currentMetric() == Explanation::Metric::VARIANCE ? true : false;

    // Compute top ranked dimensions to assign colors to
    std::vector<int> topCount(dimRanks.cols(), 0);

    for (int i = 0; i < dimRanks.rows(); i++)
    {
        float topRank = lowRankBest ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();

        for (int j = 0; j < dimRanks.cols(); j++)
        {
            float rank = dimRanks(i, j);

            if (lowRankBest) { if (rank < topRank) { topRank = rank; } }
            else             { if (rank > topRank) { topRank = rank; } }
        }

        // Count every dimension that has the same rank
        for (int j = 0; j < dimRanks.cols(); j++)
        {
            float rank = dimRanks(i, j);
            if (lowRankBest) { if (rank <= topRank) topCount[j]++; }
            else             { if (rank >= topRank) topCount[j]++; }
        }
    }
    for (int i = 0; i < topCount.size(); i++)
    {
        std::cout << "Top count: " << i << " " << topCount[i] << std::endl;
    }
    std::vector<int> indices(topCount.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {return topCount[a] > topCount[b]; });

    std::vector<QColor> newMapping(topCount.size());
    for (int i = 0; i < newMapping.size(); i++)
    {
        newMapping[i] = QColor(180, 180, 180);
    }
    int numTopDimensions = std::min(_palette.size(), newMapping.size());
    for (int i = 0; i < numTopDimensions; i++)
    {
        newMapping[indices[i]] = _palette[i];
    }
    _colorMapping = newMapping;
}

void ExplanationModel::setExplanationMetric(Explanation::Metric metric)
{
    _explanationMetric = metric;

    emit explanationMetricChanged(metric);
}

void ExplanationModel::computeDimensionRanks(DataMatrix& dimRanking, std::vector<unsigned int>& selection)
{
    Explanation::Method* explanationMethod = getCurrentExplanationMethod();

    dimRanking.resize(selection.size(), _dataset.cols());
    for (int i = 0; i < selection.size(); i++)
    {
        int si = selection[i];

        for (int j = 0; j < _dataset.cols(); j++)
        {
            dimRanking(i, j) = explanationMethod->computeDimensionRank(_dataset, si, j);
        }
    }
}

void ExplanationModel::computeDimensionRanks(DataMatrix& dimRanks)
{
    std::vector<unsigned int> selection(_dataset.rows());
    std::iota(selection.begin(), selection.end(), 0);

    computeDimensionRanks(dimRanks, selection);
}

std::vector<float> ExplanationModel::computeConfidences(const DataMatrix& dimRanks)
{
    int numPoints = dimRanks.rows();
    int numDimensions = dimRanks.cols();
    auto start = std::chrono::high_resolution_clock::now();

    // Compute the top-ranked dimension for every point
    std::vector<int> topDimensions(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        float minRank = std::numeric_limits<float>::max();
        int topRank = 0;
        for (int j = 0; j < numDimensions; j++)
        {
            float rank = dimRanks(i, j);
            if (rank < minRank) { minRank = rank; topRank = j; }
        }
        topDimensions[i] = topRank;
    }

    // Compute confidences
    std::vector<float> confidences(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        const std::vector<int>& neighbourhood = _neighbourhoodMatrix[i];

        // Add top-1 rankings over all neighbouring points in vector
        std::vector<float> topRankings(numDimensions, 0);
        for (const int ni : neighbourhood)
        {
            int topDim = topDimensions[ni];

            topRankings[topDim] += dimRanks(ni, topDim);
        }

        // Compute total ranking
        int topDim = topDimensions[i];
        float totalRank = 0;
        for (const int ni : neighbourhood)
            totalRank += dimRanks(ni, topDim);

        // Divide all rankings by total rank values to get confidences
        float topRanking = topRankings[topDim];

        confidences[i] = topRanking / totalRank;

        if (totalRank == 0)
            confidences[i] = 0;
    }

    // Normalize confidences
    float minVal = std::numeric_limits<float>::max();
    float maxVal = -std::numeric_limits<float>::max();
    for (int i = 0; i < confidences.size(); i++)
    {
        if (confidences[i] < minVal) minVal = confidences[i];
        if (confidences[i] > maxVal) maxVal = confidences[i];
    }
    std::cout << "Min val: " << minVal << " Max val: " << maxVal << std::endl;
    for (int i = 0; i < confidences.size(); i++)
    {
        confidences[i] = (confidences[i] - minVal) / (maxVal - minVal);
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Confidence Elapsed time: " << elapsed.count() << " s\n";
    return confidences;
}

void ExplanationModel::computeNeighbourhoodMatrix(NeighbourhoodMatrix& neighbourhoodMatrix, float radius)
{
    auto start = std::chrono::high_resolution_clock::now();

    neighbourhoodMatrix.clear();
    neighbourhoodMatrix.resize(_projection.rows());

#pragma omp parallel for
    for (int i = 0; i < _projection.rows(); i++)
    {
        findNeighbourhood(_projection, i, radius, neighbourhoodMatrix[i]);

        if (i % 10000 == 0) std::cout << "Computing neighbourhood for points: [" << i << "/" << _projection.rows() << "]" << std::endl;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Neighbourhood Elapsed time : " << elapsed.count() << " s\n";
}

Explanation::Method* ExplanationModel::getCurrentExplanationMethod()
{
    Explanation::Method* explanationMethod = nullptr;

    switch (_explanationMetric)
    {
    case Explanation::Metric::EUCLIDEAN: explanationMethod = &_euclideanMethod; break;
    case Explanation::Metric::VARIANCE: explanationMethod = &_varianceMethod; break;
    case Explanation::Metric::VALUE: explanationMethod = &_valueMethod; break;
    default: break;
    }

    return explanationMethod;
}
