#include "ExplanationModel.h"

#include <iostream>

namespace
{
    void convertToEigenMatrix(hdps::Dataset<Points> dataset, DataMatrix& dataMatrix)
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

    float computeProjectionDiameter(const DataMatrix& projection)
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

    void computeDatasetStats(const DataTable& dataset, DataStatistics& dataStats)
    {
        int numPoints = dataset.numPoints();
        int numDimensions = dataset.numDimensions();

        dataStats.means.clear();
        dataStats.variances.clear();
        dataStats.minRange.clear();
        dataStats.maxRange.clear();
        dataStats.ranges.clear();

        dataStats.means.resize(numDimensions);
        dataStats.variances.resize(numDimensions);
        dataStats.minRange.resize(numDimensions, std::numeric_limits<float>::max());
        dataStats.maxRange.resize(numDimensions, -std::numeric_limits<float>::max());
        dataStats.ranges.resize(numDimensions, 0);

        for (int j = 0; j < numDimensions; j++)
        {
            // Compute mean
            float mean = 0;
            for (int i = 0; i < numPoints; i++)
            {
                float value = dataset(i, j);

                if (value < dataStats.minRange[j]) dataStats.minRange[j] = value;
                if (value > dataStats.maxRange[j]) dataStats.maxRange[j] = value;
                mean += value;
            }
            mean /= numPoints;
            dataStats.means[j] = mean;

            // Compute variance
            float variance = 0;
            for (int i = 0; i < numPoints; i++)
            {
                float x = dataset(i, j) - mean;
                variance += x * x;
            }
            variance /= numPoints;
            dataStats.variances[j] = variance;
        }
        for (int j = 0; j < numDimensions; j++)
        {
            dataStats.ranges[j] = dataStats.maxRange[j] - dataStats.minRange[j];
            if (dataStats.ranges[j] == 0) dataStats.ranges[j] = 1;

            std::cout << j << ": " << "Means : " << dataStats.means[j] << " Variances : " << dataStats.variances[j] << " Min range : " << dataStats.minRange[j] << " Max range : " << dataStats.maxRange[j] << std::endl;
        }
    }

    void findNeighbourhood(const DataMatrix& projection, int centerId, float radius, std::vector<int>& neighbourhood)
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

}

void ExplanationModel::setDataset(hdps::Dataset<Points> dataset, hdps::Dataset<Points> projection)
{
    // Convert the dataset and projection to eigen matrices
    DataMatrix eigenDataMatrix;
    convertToEigenMatrix(dataset, eigenDataMatrix);
    _dataset.setData(eigenDataMatrix);
    convertToEigenMatrix(projection, _projection);

    initialize();

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
}

void ExplanationModel::initialize()
{
    // Compute projection diameter
    _projectionDiameter = computeProjectionDiameter(_projection);
    std::cout << "Diameter: " << _projectionDiameter << std::endl;

    //// Standardized dataset
    //_standardizedDataset = _dataset;
    //auto means = _standardizedDataset.colwise().mean();
    //_standardizedDataset.rowwise() -= means;

    //for (int j = 0; j < _standardizedDataset.cols(); j++)
    //{
    //    float variance = 0;
    //    for (int i = 0; i < _standardizedDataset.rows(); i++)
    //    {
    //        variance += _standardizedDataset(i, j) * _standardizedDataset(i, j);
    //    }
    //    variance /= _standardizedDataset.rows() - 1;
    //    float stddev = sqrt(variance);
    //    _standardizedDataset.col(j) /= stddev;
    //}

    //// Compute global statistics on data
    //computeDatasetStats(_dataset, _dataStats);

    //// Normalized dataset
    //_normDataset = _dataset;
    //std::vector<float> ranges(_dataset.cols());
    //for (int j = 0; j < ranges.size(); j++) ranges[j] = _dataStats.maxRange[j] - _dataStats.minRange[j];
    //for (int j = 0; j < _normDataset.cols(); j++)
    //{
    //    for (int i = 0; i < _normDataset.rows(); i++)
    //    {
    //        _normDataset(i, j) = (_normDataset(i, j)) / ranges[j]; // - _dataStats.means[j]
    //    }
    //}

    computeDatasetStats(_dataset, _dataStats);

    // Create color mapping
    _colorMapping.recreate(_dataset);

    _hasDataset = true;
}

void ExplanationModel::recomputeNeighbourhood(float neighbourhoodRadius)
{
    computeNeighbourhoodMatrix(_neighbourhoodMatrix, _projectionDiameter * neighbourhoodRadius);

    computeNeighbourhoodMatrix(_confidenceNeighbourhoodMatrix, _projectionDiameter * neighbourhoodRadius * 0.25f);
}

void ExplanationModel::recomputeMetrics()
{
    Explanation::Method* explanationMethod = getCurrentExplanationMethod();

    if (explanationMethod != nullptr)
        explanationMethod->recompute(_dataset, _neighbourhoodMatrix);
}

void ExplanationModel::recomputeColorMapping(DataMatrix& dimRanks)
{
    _colorMapping.recompute(_dataset, dimRanks, currentMetric());
}

void ExplanationModel::excludeDimension(int dim)
{
    _dataset.excludeDimension(dim);

    emit datasetDimensionsChanged();
}

void ExplanationModel::setExplanationMetric(Explanation::Metric metric)
{
    _explanationMetric = metric;

    emit explanationMetricChanged(metric);
}

void ExplanationModel::computeDimensionRanks(std::vector<float>& dimRanking, std::vector<unsigned int>& selection)
{
    Explanation::Method* explanationMethod = getCurrentExplanationMethod();

    explanationMethod->computeDimensionRank(_dataset, selection, dimRanking);
}

void ExplanationModel::computeDimensionRanks(DataMatrix& dimRanking)
{
    std::vector<unsigned int> selection(_dataset.numPoints());
    std::iota(selection.begin(), selection.end(), 0);

    Explanation::Method* explanationMethod = getCurrentExplanationMethod();

    dimRanking.resize(selection.size(), _dataset.numDimensions());
    for (int i = 0; i < selection.size(); i++)
    {
        int si = selection[i];

        for (int j = 0; j < _dataset.numDimensions(); j++)
        {
            dimRanking(i, j) = explanationMethod->computeDimensionRank(_dataset, si, j);
        }
    }
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
        float maxRank = -std::numeric_limits<float>::max();
        int topRank = 0;
        for (int j = 0; j < numDimensions; j++)
        {
            if (_dataset.isExcluded(j)) continue;

            float rank = dimRanks(i, j);

            if (currentMetric() == Explanation::Metric::VARIANCE)
            {
                if (rank < minRank) { minRank = rank; topRank = j; }
            }
            else if (currentMetric() == Explanation::Metric::VALUE)
            {
                if (rank > maxRank) { maxRank = rank; topRank = j; }
            }
        }
        topDimensions[i] = topRank;
    }

    // Compute confidences
    std::vector<float> confidences(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        const std::vector<int>& neighbourhood = _confidenceNeighbourhoodMatrix[i];

        //// Add top-1 rankings over all neighbouring points in vector
        //std::vector<float> topRankings(numDimensions, 0);
        //for (const int ni : neighbourhood)
        //{
        //    int topDim = topDimensions[ni];

        //    topRankings[topDim] += abs(dimRanks(ni, topDim));
        //}

        //// Compute total ranking
        //int topDim = topDimensions[i];
        //float totalRank = 0;
        //for (const int ni : neighbourhood)
        //    totalRank += abs(dimRanks(ni, topDim));

        //// Divide all rankings by total rank values to get confidences
        //float topRanking = topRankings[topDim];

        //confidences[i] = topRanking / totalRank;

        int topDim = topDimensions[i];

        int count = 0;
        for (const int ni : neighbourhood)
        {
            int nTopDim = topDimensions[ni];

            if (nTopDim == topDim) count++;
        }

        confidences[i] = ((float)count) / neighbourhood.size();

        //if (totalRank == 0)
        if (neighbourhood.size() == 0)
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
