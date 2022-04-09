#include "SilvaVariance.h"

#include <iostream>
#include <chrono>

void VarianceMethod::recompute(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    precomputeGlobalVariances(dataset);
    precomputeLocalVariances(_localVariances, dataset, neighbourhoodMatrix);
}

float VarianceMethod::computeDimensionRank(const Eigen::ArrayXXf& dataset, int i, int j)
{
    float sum = 0;
    for (int k = 0; k < dataset.cols(); k++)
    {
        sum += _localVariances(i, k) / _globalVariances[k];
    }
    return (_localVariances(i, j) / _globalVariances[j]) / sum;
}

void VarianceMethod::computeDimensionRank(const DataMatrix& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking)
{
    
}

void VarianceMethod::precomputeGlobalVariances(const Eigen::ArrayXXf& dataset)
{
    auto start = std::chrono::high_resolution_clock::now();

    int numPoints = dataset.rows();
    int numDimensions = dataset.cols();

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
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Global Elapsed time : " << elapsed.count() << " s\n";
}

void VarianceMethod::precomputeLocalVariances(Eigen::ArrayXXf& localVariance, const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    int numPoints = dataset.rows();
    int numDimensions = dataset.cols();
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
