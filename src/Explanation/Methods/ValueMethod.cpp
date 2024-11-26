#include "ValueMethod.h"

#include <util/Timer.h>
#include <QDebug>
#include <iostream>

void ValueMethod::setGlobalNeighbourhoodRadius(float radius)
{
    _globalNeighbourhoodRadius = radius;
}

void ValueMethod::recompute(DataMatrix& dataset)
{

}

//void ValueMethod::recompute(DataMatrix& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
//{
//    precomputeGlobalValues(dataset);
//
//    precomputeLocalValues(dataset, neighbourhoodMatrix);
//}

void ValueMethod::recompute(DataMatrix& dataset, DataMatrix& projection)
{
    // Per-dimension ranges of values found in the dataset
    _dataRanges.clear();
    _dataRanges.resize(dataset.getNumCols());
    std::vector<float> minRanges(dataset.getNumCols(), std::numeric_limits<float>::max());
    std::vector<float> maxRanges(dataset.getNumCols(), -std::numeric_limits<float>::max());

    for (int j = 0; j < dataset.getNumCols(); j++)
    {
        // Compute mean
        float mean = 0;
        for (int i = 0; i < dataset.getNumRows(); i++)
        {
            float value = dataset(i, j);

            if (value < minRanges[j]) minRanges[j] = value;
            if (value > maxRanges[j]) maxRanges[j] = value;
            mean += value;
        }
        mean /= dataset.getNumRows();
        _dataRanges[j] = maxRanges[j] - minRanges[j];

        if (_dataRanges[j] == 0) _dataRanges[j] = 1;
    }

    precomputeGlobalValues(dataset);

    precomputeLocalValues(dataset, projection);
}

void ValueMethod::precomputeGlobalValues(DataMatrix& dataset)
{
    int numPoints = dataset.getNumRows();
    int numDimensions = dataset.getNumCols();

    _globalValues.resize(numDimensions);

    {
        Timer t("Precompute Eigen Global");
        //Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> M = dataset.getData();
        auto means = dataset.getData().colwise().mean();
        qDebug() << "dataset.getData().transpose()" << dataset.getData().transpose().rows() << dataset.getData().transpose().cols();
        qDebug() << "Means rows: " << means.rows();
        qDebug() << "Means cols: " << means.cols();

        for (int d = 0; d < means.cols(); d++)
            _globalValues[d] = means[d];
        qDebug() << "Global1: " << _globalValues[0] << _globalValues[1] << _globalValues[2] << _globalValues[3] << _globalValues[4];
    }

    Timer t("Precompute Global");
    for (int d = 0; d < numDimensions; d++)
    {
        // Compute mean
        float mean = 0;
        for (int i = 0; i < numPoints; i++)
            mean += dataset(i, d);
        mean /= numPoints;

        _globalValues[d] = mean;
    }
    qDebug() << "Global2: " << _globalValues[0] << _globalValues[1] << _globalValues[2] << _globalValues[3] << _globalValues[4];
}

//void ValueMethod::precomputeLocalValues(DataMatrix& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
//{
//    auto start = std::chrono::high_resolution_clock::now();
//
//    int numPoints = dataset.getNumRows();
//    int numDimensions = dataset.getNumCols();
//
//    _localValues.resize(numPoints, std::vector<float>(numDimensions));
//#pragma omp parallel for
//    for (int i = 0; i < numPoints; i++)
//    {
//        const std::vector<int>& neighbourhood = neighbourhoodMatrix[i];
//
//        for (int j = 0; j < numDimensions; j++)
//        {
//            // Compute mean
//            float mean = 0;
//            for (int n = 0; n < neighbourhood.size(); n++)
//            {
//                mean += dataset(neighbourhood[n], j);
//            }
//            mean /= neighbourhood.size();
//
//            _localValues[i][j] = mean;
//        }
//        if (i % 1000 == 0)
//            std::cout << "Local var: " << i << std::endl;
//    }
//
//    auto finish = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double> elapsed = finish - start;
//    std::cout << "Local Value Elapsed time: " << elapsed.count() << " s\n";
//}

void ValueMethod::precomputeLocalValues(DataMatrix& dataset, DataMatrix& projection)
{
    auto start = std::chrono::high_resolution_clock::now();

    int numPoints = dataset.getNumRows();
    int numDimensions = dataset.getNumCols();

    _localValues.resize(numPoints, std::vector<float>(numDimensions));

    _lvc.initialize(projection);
    _lvc.splatValues(dataset, projection, _localValues, _globalNeighbourhoodRadius);

    _sums.clear();
    _sums.resize(dataset.getNumRows());
    for (int i = 0; i < dataset.getNumRows(); i++)
    {
        float sum = 0;
        for (int d = 0; d < dataset.getNumCols(); d++)
        {
            sum += abs((_localValues[i][d] - _globalValues[d]) / _dataRanges[d]);
        }
        _sums[i] = sum;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Local Value Elapsed time: " << elapsed.count() << " s\n";
}

float ValueMethod::computeDimensionRank(DataMatrix& dataset, int i, int j)
{
    return ((_localValues[i][j] - _globalValues[j]) / _dataRanges[j]) / _sums[i];
}

void ValueMethod::computeDimensionRank(DataMatrix& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking)
{
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>> COMPUTE DIM RANK" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    int numDimensions = dataset.getNumCols();

    // Compute mean over selection
    std::vector<float> localMeans(numDimensions, 0);
    for (int j = 0; j < numDimensions; j++)
    {
        for (int i = 0; i < selection.size(); i++)
        {
            int si = selection[i];

            localMeans[j] += dataset(si, j);
        }

        localMeans[j] /= selection.size();
    }

    auto mid = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> midelapsed = mid - start;
    std::cout << ">>>>>>>>>>>> Local means computing time: " << midelapsed.count() << " s\n" << std::endl;

    // Compute ranking
    float sum = 0;
    for (int d = 0; d < numDimensions; d++)
    {
        sum += abs((localMeans[d] - _globalValues[d]) / _dataRanges[d]);
    }
    for (int d = 0; d < numDimensions; d++)
    {
        dimRanking[d] = ((localMeans[d] - _globalValues[d]) / _dataRanges[d]) / sum;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - mid;
    std::cout << ">>>>>>>>>>>> Compute ranking time: " << elapsed.count() << " s\n" << std::endl;
}
