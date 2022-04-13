#include "ValueRanking.h"

#include <iostream>
#include <chrono>

void ValueMethod::recompute(const DataTable& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    int numPoints = dataset.numPoints();
    int numDimensions = dataset.numDimensions();

    _dataRanges.clear();
    _dataRanges.resize(numDimensions);
    std::vector<float> minRanges(numDimensions, std::numeric_limits<float>::max());
    std::vector<float> maxRanges(numDimensions, -std::numeric_limits<float>::max());

    for (int j = 0; j < numDimensions; j++)
    {
        // Compute mean
        float mean = 0;
        for (int i = 0; i < numPoints; i++)
        {
            float value = dataset(i, j);

            if (value < minRanges[j]) minRanges[j] = value;
            if (value > maxRanges[j]) maxRanges[j] = value;
            mean += value;
        }
        mean /= numPoints;
        _dataRanges[j] = maxRanges[j] - minRanges[j];
    }

    precomputeGlobalValues(dataset);
    precomputeLocalValues(dataset, neighbourhoodMatrix);
}

float ValueMethod::computeDimensionRank(const DataTable& dataset, int i, int j)
{
    float sum = 0;
    for (int k = 0; k < dataset.numDimensions(); k++)
    {
        sum += abs((_localValues(i, k) - _globalValues[k]) / _dataRanges[k]); //_localValues(i, k) / _globalValues[k];
    }
    return ((_localValues(i, j) - _globalValues[j]) / _dataRanges[j]) / sum;
}

void ValueMethod::computeDimensionRank(const DataTable& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking)
{
    int numDimensions = dataset.numDimensions();

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

    // Compute ranking
    float sum = 0;
    for (int k = 0; k < numDimensions; k++)
    {
        sum += abs((localMeans[k] - _globalValues[k]) / _dataRanges[k]);
    }
    for (int j = 0; j < numDimensions; j++)
    {
        dimRanking[j] = ((localMeans[j] - _globalValues[j]) / _dataRanges[j]) / sum;
    }
}

void ValueMethod::precomputeGlobalValues(const DataTable& dataset)
{
    int numPoints = dataset.numPoints();
    int numDimensions = dataset.numDimensions();

    _globalValues.clear();
    _globalValues.resize(numDimensions);
    for (int j = 0; j < numDimensions; j++)
    {
        // Compute mean
        float mean = 0;
        for (int i = 0; i < numPoints; i++)
        {
            mean += dataset(i, j);
        }
        mean /= numPoints;

        _globalValues[j] = mean;
    }
}

void ValueMethod::precomputeLocalValues(const DataTable& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    auto start = std::chrono::high_resolution_clock::now();

    int numPoints = dataset.numPoints();
    int numDimensions = dataset.numDimensions();

    _localValues.resize(numPoints, numDimensions);
#pragma omp parallel for
    for (int i = 0; i < numPoints; i++)
    {
        const std::vector<int>& neighbourhood = neighbourhoodMatrix[i];

        for (int j = 0; j < numDimensions; j++)
        {
            // Compute mean
            float mean = 0;
            for (int n = 0; n < neighbourhood.size(); n++)
            {
                mean += dataset(neighbourhood[n], j);
            }
            mean /= neighbourhood.size();

            _localValues(i, j) = mean;
        }
        if (i % 1000 == 0)
            std::cout << "Local var: " << i << std::endl;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Local Value Elapsed time: " << elapsed.count() << " s\n";
}
