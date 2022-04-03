#include "ValueRanking.h"

#include <iostream>
#include <chrono>

void ValueMethod::recompute(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    precomputeGlobalValues(dataset);
    precomputeLocalValues(dataset, neighbourhoodMatrix);
}

float ValueMethod::computeDimensionRank(const Eigen::ArrayXXf& dataset, int i, int j)
{
    float sum = 0;
    for (int k = 0; k < dataset.cols(); k++)
    {
        sum += _localValues(i, k) / _globalValues[k];
    }
    return (_localValues(i, j) / _globalValues[j]) / sum;
}

void ValueMethod::precomputeGlobalValues(const Eigen::ArrayXXf& dataset)
{
    int numPoints = dataset.rows();
    int numDimensions = dataset.cols();

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

void ValueMethod::precomputeLocalValues(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    auto start = std::chrono::high_resolution_clock::now();

    int numPoints = dataset.rows();
    int numDimensions = dataset.cols();

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
