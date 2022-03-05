#include "SilvaVariance.h"

#include <iostream>

void VarianceMetric::recompute(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    precomputeGlobalVariances(dataset);
    precomputeLocalVariances(dataset, neighbourhoodMatrix);
}

float VarianceMetric::computeDimensionRank(const Eigen::ArrayXXf& dataset, int i, int j)
{
    float sum = 0;
    for (int k = 0; k < dataset.cols(); k++)
    {
        sum += _localVariances(i, k) / _globalVariances[k];
    }
    return (_localVariances(i, j) / _globalVariances[j]) / sum;
}

void VarianceMetric::precomputeGlobalVariances(const Eigen::ArrayXXf& dataset)
{
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
}

void VarianceMetric::precomputeLocalVariances(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    int numPoints = dataset.rows();
    int numDimensions = dataset.cols();

    _localVariances.resize(numPoints, numDimensions);
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

            // Compute variance
            float variance = 0;
            for (int n = 0; n < neighbourhood.size(); n++)
            {
                float x = dataset(neighbourhood[n], j) - mean;
                variance += x * x;
            }
            variance /= neighbourhood.size();

            _localVariances(i, j) = variance;
        }
        if (i % 1000 == 0)
            std::cout << "Local var: " << i << std::endl;
    }
}
