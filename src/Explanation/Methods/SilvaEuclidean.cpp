#include "SilvaEuclidean.h"

#include <iostream>
#include <chrono>

namespace
{
    float sqrMag(const Eigen::ArrayXXf& dataset, int a, int b)
    {
        int numDims = dataset.cols();

        float dist = 0;
        for (int i = 0; i < numDims; i++)
        {
            float d = dataset(a, i) - dataset(b, i);
            dist += d * d;
        }
        return dist;
    }

    float sqrMag(const Eigen::ArrayXXf& dataset, const std::vector<float>& a, int b)
    {
        int numDims = dataset.cols();

        float dist = 0;
        for (int i = 0; i < numDims; i++)
        {
            float d = a[i] - dataset(b, i);
            dist += d * d;
        }
        return dist;
    }

    float distance(const Eigen::ArrayXXf& dataset, int a, int b)
    {
        return (dataset.row(a) - dataset.row(b)).matrix().norm();
        //return sqrt(sqrMag(dataset, a, b));
    }

    float distContrib(const Eigen::ArrayXXf& dataset, int p, int r, int dim)
    {
        int numDims = dataset.cols();
        float dimDistSquared = dataset(p, dim) - dataset(r, dim);
        dimDistSquared *= dimDistSquared;

        float totalDistSquared = sqrMag(dataset, p, r);
        return dimDistSquared / totalDistSquared;
    }

    float distContrib(const Eigen::ArrayXXf& dataset, const std::vector<float>& p, int r, int dim)
    {
        int numDims = dataset.cols();
        float dimDistSquared = p[dim] - dataset(r, dim);
        dimDistSquared *= dimDistSquared;

        float totalDistSquared = sqrMag(dataset, p, r);
        return dimDistSquared / totalDistSquared;
    }

    float localDistContrib(const Eigen::ArrayXXf& dataset, int p, int dim, const std::vector<int>& neighbourhood)
    {
        float localDistContrib = 0;
        for (int i = 0; i < neighbourhood.size(); i++)
        {
            localDistContrib += distContrib(dataset, p, neighbourhood[i], dim);
        }
        return localDistContrib / neighbourhood.size();
    }

    std::vector<float> computeNDCentroid(const Eigen::ArrayXXf& dataset)
    {
        int numPoints = dataset.rows();
        int numDimensions = dataset.cols();
        std::vector<float> centroid(numDimensions, 0);

        for (int i = 0; i < numPoints; i++)
        {
            for (int j = 0; j < numDimensions; j++)
            {
                centroid[j] += dataset(i, j);
            }
        }
        for (int j = 0; j < numDimensions; j++)
        {
            centroid[j] /= numPoints;
        }
        return centroid;
    }

    float globalDistContrib(const Eigen::ArrayXXf& dataset, const std::vector<float>& centroid, int dim)
    {
        float globalDistContrib = 0;
        for (int i = 0; i < dataset.rows(); i++)
        {
            globalDistContrib += distContrib(dataset, centroid, i, dim);
        }
        return globalDistContrib / dataset.rows();
    }

    float dimensionRank(const Eigen::ArrayXXf& dataset, int p, int dim, const std::vector<int>& neighbourhood, const std::vector<float>& globalDistContribs)
    {
        float sum = 0;
        for (int j = 0; j < dataset.cols(); j++)
        {
            sum += localDistContrib(dataset, p, j, neighbourhood) / globalDistContribs[dim];
        }
        return (localDistContrib(dataset, p, dim, neighbourhood) / globalDistContribs[dim]) / sum;
    }
}

void EuclideanMethod::recompute(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    computeCentroid(dataset);
    computeGlobalContribs(dataset);
    computeLocalContribs(dataset, neighbourhoodMatrix);
}

float EuclideanMethod::computeDimensionRank(const Eigen::ArrayXXf& dataset, int i, int j)
{
    float sum = 0;
    for (int k = 0; k < dataset.cols(); k++)
    {
        sum += _localDistContribs(i, k) / _globalDistContribs[k];
    }
    return (_localDistContribs(i, j) / _globalDistContribs[j]) / sum;
}

void EuclideanMethod::computeCentroid(const Eigen::ArrayXXf& dataset)
{
    int numPoints = dataset.rows();
    int numDimensions = dataset.cols();

    _centroid.clear();
    _centroid.resize(numDimensions, 0);

    for (int i = 0; i < numPoints; i++)
    {
        for (int j = 0; j < numDimensions; j++)
        {
            _centroid[j] += dataset(i, j);
        }
    }
    for (int j = 0; j < numDimensions; j++)
    {
        _centroid[j] /= numPoints;
    }
}

void EuclideanMethod::computeGlobalContribs(const Eigen::ArrayXXf& dataset)
{
    std::cout << "Precomputing global distance contributions.." << std::endl;
    int numDimensions = dataset.cols();

    _globalDistContribs.clear();
    _globalDistContribs.resize(numDimensions);
    for (int dim = 0; dim < numDimensions; dim++)
    {
        std::cout << "Dim: " << dim << std::endl;
        _globalDistContribs[dim] = globalDistContrib(dataset, _centroid, dim);
    }
}

void EuclideanMethod::computeLocalContribs(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix)
{
    std::cout << "Precomputing local distance contributions.." << std::endl;

    int numPoints = dataset.rows();
    int numDimensions = dataset.cols();

    _localDistContribs.resize(numPoints, numDimensions);

    for (int i = 0; i < numPoints; i++)
    {
        auto start = std::chrono::system_clock::now();
        for (int j = 0; j < numDimensions; j++)
        {
            _localDistContribs(i, j) = localDistContrib(dataset, i, j, neighbourhoodMatrix[i]);
        }
        if (i % 10 == 0) std::cout << "Local dist contribs: " << i << std::endl;
        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Elapsed time: " << elapsed.count() << "s";
    }
}
