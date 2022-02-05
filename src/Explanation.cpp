#include "Explanation.h"

void findNeighbourhood(Dataset<Points> projection, int centerId, float radius, std::vector<int>& neighbourhood)
{
    float x = projection->getValueAt(centerId * 2 + 0);
    float y = projection->getValueAt(centerId * 2 + 1);

    float radSquared = radius * radius;

    for (int i = 0; i < projection->getNumPoints(); i++)
    {
        if (i == centerId) continue;

        float xi = projection->getValueAt(i * 2 + 0);
        float yi = projection->getValueAt(i * 2 + 1);

        float xd = xi - x;
        float yd = yi - y;

        float magSquared = xd * xd + yd * yd;

        if (magSquared > radSquared)
            continue;

        neighbourhood.push_back(i);
    }
}

float sqrMag(Dataset<Points> dataset, int a, int b)
{
    int numDims = dataset->getNumDimensions();

    float dist = 0;
    for (int i = 0; i < numDims; i++)
    {
        float d = dataset->getValueAt(a * numDims + i) - dataset->getValueAt(b * numDims + i);
        dist += d * d;
    }
    return dist;
}

float sqrMag(Dataset<Points> dataset, const std::vector<float>& a, int b)
{
    int numDims = dataset->getNumDimensions();

    float dist = 0;
    for (int i = 0; i < numDims; i++)
    {
        float d = a[i] - dataset->getValueAt(b * numDims + i);
        dist += d * d;
    }
    return dist;
}

float distance(Dataset<Points> dataset, int a, int b)
{
    return sqrt(sqrMag(dataset, a, b));
}

float distContrib(Dataset<Points> dataset, int p, int r, int dim)
{
    int numDims = dataset->getNumDimensions();
    float dimDistSquared = dataset->getValueAt(p * numDims + dim) - dataset->getValueAt(r * numDims + dim);
    dimDistSquared *= dimDistSquared;

    float totalDistSquared = sqrMag(dataset, p, r);
    return dimDistSquared / totalDistSquared;
}

float distContrib(Dataset<Points> dataset, const std::vector<float>& p, int r, int dim)
{
    int numDims = dataset->getNumDimensions();
    float dimDistSquared = p[dim] - dataset->getValueAt(r * numDims + dim);
    dimDistSquared *= dimDistSquared;

    float totalDistSquared = sqrMag(dataset, p, r);
    return dimDistSquared / totalDistSquared;
}

float localDistContrib(Dataset<Points> dataset, int p, int dim, const std::vector<int>& neighbourhood)
{
    float localDistContrib = 0;
    for (int i = 0; i < neighbourhood.size(); i++)
    {
        localDistContrib += distContrib(dataset, p, neighbourhood[i], dim);
    }
    return localDistContrib / neighbourhood.size();
}

std::vector<float> computeNDCentroid(Dataset<Points> dataset)
{
    int numPoints = dataset->getNumPoints();
    int numDimensions = dataset->getNumDimensions();
    std::vector<float> centroid(numDimensions, 0);

    for (int i = 0; i < dataset->getNumPoints(); i++)
    {
        for (int j = 0; j < numDimensions; j++)
        {
            centroid[j] += dataset->getValueAt(i * numDimensions + j);
        }
    }
    for (int j = 0; j < numDimensions; j++)
    {
        centroid[j] /= numPoints;
    }
    return centroid;
}

float globalDistContrib(Dataset<Points> dataset, const std::vector<float>& centroid, int dim)
{
    float globalDistContrib = 0;
    for (int i = 0; i < dataset->getNumPoints(); i++)
    {
        globalDistContrib += distContrib(dataset, centroid, i, dim);
    }
    return globalDistContrib / dataset->getNumPoints();
}

float dimensionRank(Dataset<Points> dataset, int p, int dim, const std::vector<int>& neighbourhood, const std::vector<float>& globalDistContribs)
{
    float sum = 0;
    for (int j = 0; j < dataset->getNumDimensions(); j++)
    {
        sum += localDistContrib(dataset, p, j, neighbourhood) / globalDistContribs[dim];
    }
    return (localDistContrib(dataset, p, dim, neighbourhood) / globalDistContribs[dim]) / sum;
}

void computeDimensionRanking(Dataset<Points> dataset, Dataset<Points> projection, std::vector<float>& dimRanking)
{
    int numProjDims = 2;

    // Precompute neighbourhoods
    std::cout << "Precomputing neighbourhood matrix.." << std::endl;
    std::vector<std::vector<int>> V;
    for (int i = 0; i < dataset->getNumPoints(); i++)
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

    for (int dim = 0; dim < dataset->getNumDimensions(); dim++)
    {
        std::cout << "Dim: " << dim << std::endl;
        globalDistContribs.push_back(globalDistContrib(dataset, centroid, dim));
    }

    dimRanking.resize(dataset->getNumPoints());
    for (int i = 0; i < dataset->getNumPoints(); i++)
    {
        //float x = projection->getValueAt(i * 2 + 0);
        //float y = projection->getValueAt(i * 2 + 1);
        std::vector<float> dimRanks;
        for (int j = 0; j < dataset->getNumDimensions(); j++)
        {
            //std::cout << "Dim rank: " << j << std::endl;
            float dimRank = dimensionRank(dataset, i, j, V[i], globalDistContribs);
            dimRanks.push_back(dimRank);
        }

        std::vector<int> indices(dataset->getNumDimensions());
        std::iota(indices.begin(), indices.end(), 0); //Initializing
        std::sort(indices.begin(), indices.end(), [&](int i, int j) {return dimRanks[i] < dimRanks[j]; });

        //std::cout << "Most important dimension: " << indices[0] << std::endl;
        dimRanking[i] = indices[0];
    }
}

void Explanation::setDataset(Dataset<Points> dataset, Dataset<Points> projection)
{
    _dataset = dataset;
    _projection = projection;

    computeNeighbourhoodMatrix();
    computeCentroid();
    computeGlobalContrib();
}

void Explanation::computeNeighbourhoodMatrix()
{
    _neighbourhoodMatrix.clear();
    for (int i = 0; i < _dataset->getNumPoints(); i++)
    {
        std::vector<int> neighbourhood;
        findNeighbourhood(_projection, i, 3, neighbourhood);
        _neighbourhoodMatrix.push_back(neighbourhood);
        if (i % 1000 == 0) std::cout << i << std::endl;
    }
}

void Explanation::computeCentroid()
{
    int numPoints = _dataset->getNumPoints();
    int numDimensions = _dataset->getNumDimensions();

    _centroid.clear();
    _centroid.resize(numDimensions, 0);

    for (int i = 0; i < numPoints; i++)
    {
        for (int j = 0; j < numDimensions; j++)
        {
            _centroid[j] += _dataset->getValueAt(i * numDimensions + j);
        }
    }
    for (int j = 0; j < numDimensions; j++)
    {
        _centroid[j] /= numPoints;
    }
}

void Explanation::computeGlobalContrib()
{
    std::cout << "Precomputing distance contributions.." << std::endl;
    int numDimensions = _dataset->getNumDimensions();

    _globalDistContribs.clear();
    _globalDistContribs.resize(numDimensions);
    for (int dim = 0; dim < _dataset->getNumDimensions(); dim++)
    {
        std::cout << "Dim: " << dim << std::endl;
        _globalDistContribs[dim] = globalDistContrib(_dataset, _centroid, dim);
    }
}

void Explanation::computeDimensionRanking(Eigen::ArrayXXi& dimRanking)
{
    int numProjDims = 2;

    dimRanking.resize(_dataset->getNumPoints(), _dataset->getNumDimensions());
    for (int i = 0; i < _dataset->getNumPoints(); i++)
    {
        std::vector<float> dimRanks(_dataset->getNumDimensions());
        for (int j = 0; j < _dataset->getNumDimensions(); j++)
        {
            float dimRank = dimensionRank(_dataset, i, j, _neighbourhoodMatrix[i], _globalDistContribs);
            dimRanks[j] = dimRank;
        }

        std::vector<int> indices(_dataset->getNumDimensions());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&](int i, int j) {return dimRanks[i] < dimRanks[j]; });

        for (int j = 0; j < _dataset->getNumDimensions(); j++)
        {
            dimRanking(i, j) = indices[j];
        }
    }
}
