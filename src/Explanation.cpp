#include "Explanation.h"

void findNeighbourhood(const Eigen::ArrayXXf& projection, int centerId, float radius, std::vector<int>& neighbourhood)
{
    float x = projection(centerId, 0);
    float y = projection(centerId, 1);

    float radSquared = radius * radius;

    neighbourhood.clear();
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

float euclideanDimensionRank(const Eigen::ArrayXXf& dataset, int p, int dim, Eigen::ArrayXXf& localDistContribs, const std::vector<float>& globalDistContribs)
{
    float sum = 0;
    for (int j = 0; j < dataset.cols(); j++)
    {
        sum += localDistContribs(p, j) / globalDistContribs[j];
    }
    return (localDistContribs(p, dim) / globalDistContribs[dim]) / sum;
}

float varianceDimensionRank(const Eigen::ArrayXXf& dataset, int p, int dim, Eigen::ArrayXXf& localVariances, const std::vector<float>& globalVariance)
{
    float sum = 0;
    for (int j = 0; j < dataset.cols(); j++)
    {
        sum += localVariances(p, j) / globalVariance[j];
    }
    return (localVariances(p, dim) / globalVariance[dim]) / sum;
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

    // Compute projection diameter
    float minX = std::numeric_limits<float>::max(), maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max(), maxY = -std::numeric_limits<float>::max();
    for (int i = 0; i < _projection.rows(); i++)
    {
        float x = _projection(i, 0);
        float y = _projection(i, 1);

        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }
    float rangeX = maxX - minX;
    float rangeY = maxY - minY;

    float diameter = rangeX > rangeY ? rangeX : rangeY;
    std::cout << "Diameter: " << diameter << std::endl;

    // Norm dataset
    _normDataset = _dataset;
    auto means = _normDataset.colwise().mean();
    _normDataset.rowwise() -= means;
    //std::cout << _normDataset << std::endl;
    //Eigen::Array<double, 1, Eigen::Dynamic> std_dev = ((_normDataset.rowwise() - _normDataset.colwise().mean()).square().colwise().sum() / (_normDataset.cols() - 1)).sqrt();
    for (int j = 0; j < _normDataset.cols(); j++)
    {
        float variance = 0;
        for (int i = 0; i < _normDataset.rows(); i++)
        {
            variance += _normDataset(i, j) * _normDataset(i, j);
        }
        variance /= _normDataset.rows() - 1;
        float stddev = sqrt(variance);
        _normDataset.col(j) /= stddev;
    }

    computeNeighbourhoodMatrix(diameter * 0.2f);
    //computeCentroid();
    //computeGlobalContribs();
    //computeLocalContribs();
    computeGlobalVariances();
    computeLocalVariances();

    //std::cout << "Standardized" << std::endl;
    //std::cout << _normDataset << std::endl;
}

void Explanation::computeNeighbourhoodMatrix(float radius)
{
    _neighbourhoodMatrix.clear();
    _neighbourhoodMatrix.resize(_dataset.rows());
    for (int i = 0; i < _dataset.rows(); i++)
    {
        findNeighbourhood(_projection, i, radius, _neighbourhoodMatrix[i]);

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

void Explanation::computeGlobalVariances()
{
    int numPoints = _dataset.rows();
    int numDimensions = _dataset.cols();

    _globalVariance.resize(numDimensions);
    for (int j = 0; j < numDimensions; j++)
    {
        // Compute mean
        float mean = 0;
        for (int i = 0; i < numPoints; i++)
        {
            mean += _dataset(i, j);
        }
        mean /= numPoints;

        // Compute variance
        float variance = 0;
        for (int i = 0; i < numPoints; i++)
        {
            float x = _dataset(i, j) - mean;
            variance += x * x;
        }
        variance /= numPoints;

        _globalVariance[j] = variance;
    }
}

void Explanation::computeLocalVariances()
{
    int numPoints = _dataset.rows();
    int numDimensions = _dataset.cols();

    _localVariances.resize(numPoints, numDimensions);
    for (int i = 0; i < numPoints; i++)
    {
        const std::vector<int>& neighbourhood = _neighbourhoodMatrix[i];

        for (int j = 0; j < numDimensions; j++)
        {
            // Compute mean
            float mean = 0;
            for (int n = 0; n < neighbourhood.size(); n++)
            {
                mean += _dataset(neighbourhood[n], j);
            }
            mean /= neighbourhood.size();

            // Compute variance
            float variance = 0;
            for (int n = 0; n < neighbourhood.size(); n++)
            {
                float x = _dataset(neighbourhood[n], j) - mean;
                variance += x * x;
            }
            variance /= neighbourhood.size();

            _localVariances(i, j) = variance;
        }
        if (i % 1000 == 0)
            std::cout << "Local var: " << i << std::endl;
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
            float dimRank = euclideanDimensionRank(_dataset, si, j, _localDistContribs, _globalDistContribs);
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

void Explanation::computeDimensionRanks(Eigen::ArrayXXf& dimRanking, std::vector<unsigned int>& selection, Metric metric)
{
    dimRanking.resize(selection.size(), _dataset.cols());
    for (int i = 0; i < selection.size(); i++)
    {
        int si = selection[i];

        for (int j = 0; j < _dataset.cols(); j++)
        {
            float dimRank = 0;

            if (metric == Metric::EUCLIDEAN)
                dimRank = euclideanDimensionRank(_dataset, si, j, _localDistContribs, _globalDistContribs);
            if (metric == Metric::VARIANCE)
                dimRank = varianceDimensionRank(_dataset, si, j, _localVariances, _globalVariance);

            dimRanking(i, j) = dimRank;
        }
    }
}

void Explanation::computeDimensionRanks(Eigen::ArrayXXf& dimRanks, Metric metric)
{
    std::vector<unsigned int> selection(_dataset.rows());
    std::iota(selection.begin(), selection.end(), 0);

    computeDimensionRanks(dimRanks, selection, metric);
}

QImage Explanation::computeEigenImage(std::vector<unsigned int>& selection, std::vector<float>& importantDims)
{
    int numPoints = selection.size();
    int numDimensions = _normDataset.cols();

    if (numPoints <= 0)
        return QImage();

    // Form new selection only dataset
    Eigen::MatrixXf selectedData(numPoints, numDimensions);
    for (int i = 0; i < selection.size(); i++)
    {
        selectedData.row(i) = _normDataset.row(selection[i]);
    }

    auto meansArray = selectedData.colwise().mean();

    Eigen::MatrixXf covariance(numDimensions, numDimensions);
    covariance.setZero();

    // Compute covariance matrix
    for (int i = 0; i < numPoints; i++)
    {
        Eigen::MatrixXf eigenDev = selectedData.row(i) - meansArray;
        //if (i == 0)
        //    std::cout << eigenDev.transpose() * eigenDev << std::endl;
        covariance += eigenDev.transpose() * eigenDev;
    }
    covariance /= (float)numPoints;

    // Compute eigenvectors for the covariance matrix
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> solver(covariance);
    Eigen::MatrixXf eVectors = solver.eigenvectors().real();
    Eigen::VectorXf eValues = solver.eigenvalues().real();

    eVectors.transposeInPlace();

    //// Sort eigenvectors and eigenvalues
    //std::vector<int> sortedIndices(eValues.size());
    //std::iota(sortedIndices.begin(), sortedIndices.end(), 0);
    std::vector<float> sortedEigenValues(eValues.size());
    for (int i = 0; i < eValues.size(); i++)
        sortedEigenValues[i] = eValues[i];
    //std::sort(std::begin(sortedIndices), std::end(sortedIndices), [&](int i1, int i2) { return sortedEigenValues[i1] > sortedEigenValues[i2]; });

    //std::cout << "Covariance Matrix: " << std::endl;
    //std::cout << covariance << std::endl;

    //std::cout << "Eigenvalues: " << std::endl;
    //for (int i = 0; i < sortedEigenValues.size(); i++)
    //{
    //    std::cout << sortedEigenValues[i] << " ";
    //}
    //std::cout << std::endl;
    //std::cout << sortedEigenValues[sortedIndices[0]] << " " << sortedEigenValues[sortedIndices[1]] << " " << sortedEigenValues[sortedIndices[2]] << std::endl;
    //std::cout << sortedIndices[0] << " " << sortedIndices[1] << " " << sortedIndices[2] << std::endl;

    float totalEigenValue = 0;
    for (int i = 0; i < sortedEigenValues.size(); i++)
    {
        totalEigenValue += sortedEigenValues[i];
    }

    float eigenValueSum = 0;
    int lastIndex = 0;
    for (int i = 0; i < sortedEigenValues.size(); i++)
    {
        eigenValueSum += sortedEigenValues[sortedEigenValues.size()-1-i];
        if (eigenValueSum / totalEigenValue > 0.9)
        {
            lastIndex = i;
            break;
        }
    }

    std::cout << eVectors.rows() << " " << eVectors.cols() << std::endl;
    std::cout << "Number of eigenvectors needed to explain 90% of variance: " << lastIndex << std::endl;

    std::vector<std::vector<float>> eigenVectors(lastIndex, std::vector<float>(numDimensions));

    for (int i = 0; i < lastIndex; i++)
    {
        int index = sortedEigenValues.size() - 1 - i;
        for (int d = 0; d < numDimensions; d++)
        {
            eigenVectors[i][d] = eVectors.coeff(index, d);
        }
    }
    std::cout << "Copied eigen vectors" << std::endl;

    ////// Load up important dims
    importantDims.resize(numDimensions, 0);
    for (int i = 0; i < eigenVectors.size(); i++)
    {
        for (int d = 0; d < numDimensions; d++)
        {
            importantDims[d] += eigenVectors[i][d];
        }
    }

    //////

    //// Normalize the eigen vectors
    //for (int i = 0; i < eigenVectors.size(); i++)
    //{
    //    std::vector<float>& eigenVector = eigenVectors[i];
    //    float length = 0;
    //    for (int d = 0; d < numDimensions; d++)
    //    {
    //        length += eigenVector[d] * eigenVector[d];
    //    }
    //    length = sqrt(length);
    //    for (int d = 0; d < numDimensions; d++)
    //    {
    //        eigenVector[d] /= length;
    //    }
    //}
    //std::cout << "Normalized eigen vectors" << std::endl;

    std::vector<float> pixels(numDimensions, 0);
    for (int i = 0; i < eigenVectors.size(); i++)
    {
        const std::vector<float>& eigenVector = eigenVectors[i];
        float eigenValue = sortedEigenValues[sortedEigenValues.size() - 1 - i];
        float factor = eigenValue / eigenValueSum;

        for (int d = 0; d < numDimensions; d++)
        {
            pixels[d] += eigenVector[d] * factor;
        }
    }
    QImage image(28, 28, QImage::Format::Format_ARGB32);
    return image;

    //for (int d = 0; d < numDimensions; d++)
    //{
    //    int value = (int)(fabs(pixels[d]) * 4096);
    //    image.setPixel(d % 28, d / 28, qRgba(value, value, value, 255));
    //}
}
