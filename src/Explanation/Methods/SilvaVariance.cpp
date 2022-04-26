#include "SilvaVariance.h"

#include <iostream>
#include <chrono>

void VarianceMethod::recompute(const DataTable& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    precomputeGlobalVariances(dataset);
    precomputeLocalVariances(_localVariances, dataset, neighbourhoodMatrix);
}

float VarianceMethod::computeDimensionRank(const DataTable& dataset, int i, int j)
{
    float sum = 0;
    for (int k = 0; k < dataset.numDimensions(); k++)
    {
        sum += _localVariances(i, k) / _globalVariances[k];
    }
    return (_localVariances(i, j) / _globalVariances[j]) / sum;
}

void VarianceMethod::computeDimensionRank(const DataTable& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking)
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

    // Compute variances
    std::vector<float> localVariances(numDimensions, 0);
    localVariances.resize(numDimensions, 0);
    for (int j = 0; j < numDimensions; j++)
    {
        for (int i = 0; i < selection.size(); i++)
        {
            int si = selection[i];
            float x = dataset(si, j) - localMeans[j];
            localVariances[j] += x * x;
        }
        localVariances[j] /= selection.size();
        //variances[j] = sqrt(variances[j]);
    }

    // Compute ranking
    float sum = 0;
    for (int k = 0; k < numDimensions; k++)
    {
        sum += localVariances[k] / _globalVariances[k];
    }
    for (int j = 0; j < numDimensions; j++)
    {
        dimRanking[j] = (localVariances[j] / _globalVariances[j]) / sum;
    }
}

void VarianceMethod::precomputeGlobalVariances(const DataTable& dataset)
{
    auto start = std::chrono::high_resolution_clock::now();

    int numPoints = dataset.numPoints();
    int numDimensions = dataset.numDimensions();

    _globalVariances.clear();
    _globalVariances.resize(numDimensions);
    for (int j = 0; j < numDimensions; j++)
    {
        // Compute mean
        float mean = 0;
        for (int i = 0; i < numPoints; i++)
        {
            mean += dataset(i, j);
        }
        mean /= numPoints;

        // Compute variance
        float variance = 0;
        for (int i = 0; i < numPoints; i++)
        {
            float x = dataset(i, j) - mean;
            variance += x * x;
        }
        variance /= numPoints;

        _globalVariances[j] = variance;

        if (variance == 0) _globalVariances[j] = 1;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Global Elapsed time : " << elapsed.count() << " s\n";
}

void VarianceMethod::precomputeLocalVariances(DataMatrix& localVariance, const DataTable& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    int numPoints = dataset.numPoints();
    int numDimensions = dataset.numDimensions();
    auto start = std::chrono::high_resolution_clock::now();

    localVariance.resize(numPoints, numDimensions);

#pragma omp parallel for
    for (int i = 0; i < numPoints; i++)
    {
        const std::vector<int>& neighbourhood = neighbourhoodMatrix[i];

        //auto subdata = dataset(neighbourhood, Eigen::all);
        //auto variances = ((subdata.rowwise() - subdata.colwise().mean()).pow(2).colwise().sum()) / neighbourhood.size();
        //localVariance.row(i) = variances;
        
        for (int j = 0; j < numDimensions; j++)
        {
            // Compute mean
            float mean = 0;
            for (const int ni : neighbourhood)
            {
                mean += dataset(ni, j);
            }
            mean /= neighbourhood.size();

            // Compute variance
            float variance = 0;
            for (const int ni : neighbourhood)
            {
                float x = dataset(ni, j) - mean;
                variance += x * x;
            }
            variance /= neighbourhood.size();

            localVariance(i, j) = variance;
        }
        if (i % 10000 == 0)
            std::cout << "Local var: " << i << std::endl;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Local Variance Elapsed time: " << elapsed.count() << " s\n";
}
