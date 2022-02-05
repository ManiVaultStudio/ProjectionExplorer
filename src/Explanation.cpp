#include "Explanation.h"

void findNeighbourhood(const Eigen::ArrayXXf& projection, int centerId, float radius, std::vector<int>& neighbourhood)
{
    float x = projection(centerId, 0);
    float y = projection(centerId, 1);

    float radSquared = radius * radius;

    for (int i = 0; i < projection.rows(); i++)
    {
        if (i == centerId) continue;

        float xi = projection(i, 0);
        float yi = projection(i, 1);

        float xd = xi - x;
        float yd = yi - y;

        float magSquared = xd * xd + yd * yd;

        if (magSquared > radSquared)
            continue;

        neighbourhood.push_back(i);
    }
}

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
    return sqrt(sqrMag(dataset, a, b));
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

float dimensionRank(const Eigen::ArrayXXf& dataset, int p, int dim, Eigen::ArrayXXf& localDistContribs, const std::vector<float>& globalDistContribs)
{
    float sum = 0;
    for (int j = 0; j < dataset.cols(); j++)
    {
        sum += localDistContribs(p, j) / globalDistContribs[dim];
    }
    return (localDistContribs(p, dim) / globalDistContribs[dim]) / sum;
}

void computeDimensionRanking(const Eigen::ArrayXXf& dataset, const Eigen::ArrayXXf& projection, std::vector<float>& dimRanking)
{
    int numProjDims = 2;

    // Precompute neighbourhoods
    std::cout << "Precomputing neighbourhood matrix.." << std::endl;
    std::vector<std::vector<int>> V;
    for (int i = 0; i < dataset.rows(); i++)
    {
        std::vector<int> neighbourhood;
        findNeighbourhood(projection, i, 3, neighbourhood);
        V.push_back(neighbourhood);
        std::cout << i << std::endl;
    }

    // Precompute dist contribs
    std::cout << "Precomputing distance contributions.." << std::endl;
    std::vector<float> globalDistContribs;

    std::cout << "Computing centroid" << std::endl;
    std::vector<float> centroid = computeNDCentroid(dataset);

    for (int dim = 0; dim < dataset.cols(); dim++)
    {
        std::cout << "Dim: " << dim << std::endl;
        globalDistContribs.push_back(globalDistContrib(dataset, centroid, dim));
    }

    dimRanking.resize(dataset.rows());
    for (int i = 0; i < dataset.rows(); i++)
    {
        //float x = projection->getValueAt(i * 2 + 0);
        //float y = projection->getValueAt(i * 2 + 1);
        std::vector<float> dimRanks;
        for (int j = 0; j < dataset.cols(); j++)
        {
            //std::cout << "Dim rank: " << j << std::endl;
            float dimRank = dimensionRank(dataset, i, j, V[i], globalDistContribs);
            dimRanks.push_back(dimRank);
        }

        std::vector<int> indices(dataset.cols());
        std::iota(indices.begin(), indices.end(), 0); //Initializing
        std::sort(indices.begin(), indices.end(), [&](int i, int j) {return dimRanks[i] < dimRanks[j]; });

        //std::cout << "Most important dimension: " << indices[0] << std::endl;
        dimRanking[i] = indices[0];
    }
}

void Explanation::setDataset(Dataset<Points> dataset, Dataset<Points> projection)
{
    // Store dataset and projection as eigen arrays
    _dataset.resize(dataset->getNumPoints(), dataset->getNumDimensions());
    for (int j = 0; j < dataset->getNumDimensions(); j++)
    {
        std::vector<float> result;
        dataset->extractDataForDimension(result, j);
        for (int i = 0; i < result.size(); i++)
            _dataset(i, j) = result[i];
    }

    _projection.resize(projection->getNumPoints(), projection->getNumDimensions());
    for (int j = 0; j < projection->getNumDimensions(); j++)
    {
        std::vector<float> result;
        projection->extractDataForDimension(result, j);
        for (int i = 0; i < result.size(); i++)
            _projection(i, j) = result[i];
    }

    computeNeighbourhoodMatrix();
    computeCentroid();
    computeGlobalContribs();
    computeLocalContribs();
}

void Explanation::computeNeighbourhoodMatrix()
{
    _neighbourhoodMatrix.clear();
    for (int i = 0; i < _dataset.rows(); i++)
    {
        std::vector<int> neighbourhood;
        findNeighbourhood(_projection, i, 3, neighbourhood);
        _neighbourhoodMatrix.push_back(neighbourhood);
        if (i % 1000 == 0) std::cout << i << std::endl;
    }
}

void Explanation::computeCentroid()
{
    int numPoints = _dataset.rows();
    int numDimensions = _dataset.cols();

    _centroid.clear();
    _centroid.resize(numDimensions, 0);

    for (int i = 0; i < numPoints; i++)
    {
        for (int j = 0; j < numDimensions; j++)
        {
            _centroid[j] += _dataset(i, j);
        }
    }
    for (int j = 0; j < numDimensions; j++)
    {
        _centroid[j] /= numPoints;
    }
}

void Explanation::computeGlobalContribs()
{
    std::cout << "Precomputing global distance contributions.." << std::endl;
    int numDimensions = _dataset.cols();

    _globalDistContribs.clear();
    _globalDistContribs.resize(numDimensions);
    for (int dim = 0; dim < numDimensions; dim++)
    {
        std::cout << "Dim: " << dim << std::endl;
        _globalDistContribs[dim] = globalDistContrib(_dataset, _centroid, dim);
    }
}

void Explanation::computeLocalContribs()
{
    std::cout << "Precomputing local distance contributions.." << std::endl;
    int numPoints = _dataset.rows();
    int numDimensions = _dataset.cols();

    _localDistContribs.resize(numPoints, numDimensions);

    for (int i = 0; i < numPoints; i++)
    {
        for (int j = 0; j < numDimensions; j++)
        {
            _localDistContribs(i, j) = localDistContrib(_dataset, i, j, _neighbourhoodMatrix[i]);
        }
        if (i % 100 == 0) std::cout << "Local dist contribs: " << i << std::endl;
    }
}

void Explanation::computeDimensionRanking(Eigen::ArrayXXi& dimRanking, std::vector<unsigned int> selection)
{
    int numProjDims = 2;

    dimRanking.resize(selection.size(), _dataset.cols());
    for (int i = 0; i < selection.size(); i++)
    {
        int si = selection[i];

        std::vector<float> dimRanks(_dataset.cols());
        for (int j = 0; j < _dataset.cols(); j++)
        {
            float dimRank = dimensionRank(_dataset, si, j, _localDistContribs, _globalDistContribs);
            dimRanks[j] = dimRank;
        }

        std::vector<int> indices(_dataset.cols());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&](int i, int j) {return dimRanks[i] < dimRanks[j]; });

        for (int j = 0; j < _dataset.cols(); j++)
        {
            dimRanking(i, j) = indices[j];
        }
    }
}

void Explanation::computeDimensionRanking(Eigen::ArrayXXi& dimRanking)
{
    std::vector<unsigned int> selection(_dataset.rows());
    std::iota(selection.begin(), selection.end(), 0);

    computeDimensionRanking(dimRanking, selection);
}

void Explanation::computeDimensionRanks(Eigen::ArrayXXf& dimRanking, std::vector<unsigned int> selection)
{
    int numProjDims = 2;

    dimRanking.resize(selection.size(), _dataset.cols());
    for (int i = 0; i < selection.size(); i++)
    {
        int si = selection[i];

        std::vector<float> dimRanks(_dataset.cols());
        for (int j = 0; j < _dataset.cols(); j++)
        {
            float dimRank = dimensionRank(_dataset, si, j, _localDistContribs, _globalDistContribs);
            dimRanking(i, j) = dimRank;
        }
    }
}
