#include "Explanation.h"

#include <chrono>

namespace
{
    float computeProjectionDiameter(const Eigen::ArrayXXf& projection)
    {
        float minX = std::numeric_limits<float>::max(), maxX = -std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max(), maxY = -std::numeric_limits<float>::max();
        for (int i = 0; i < projection.rows(); i++)
        {
            float x = projection(i, 0);
            float y = projection(i, 1);

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
        float rangeX = maxX - minX;
        float rangeY = maxY - minY;

        float diameter = rangeX > rangeY ? rangeX : rangeY;
        return diameter;
    }

    void findNeighbourhood(const Eigen::ArrayXXf& projection, int centerId, float radius, std::vector<int>& neighbourhood)
    {
        float x = projection(centerId, 0);
        float y = projection(centerId, 1);

        float radSquared = radius * radius;

        neighbourhood.clear();
        for (int i = 0; i < projection.rows(); i++)
        {
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
    _projectionDiameter = computeProjectionDiameter(_projection);
    std::cout << "Diameter: " << _projectionDiameter << std::endl;

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
}

void Explanation::updatePrecomputations(float neighbourhoodRadius)
{
    computeNeighbourhoodMatrix(_neighbourhoodMatrix, _projectionDiameter * neighbourhoodRadius);
    computeNeighbourhoodMatrix(_confidenceNeighbourhoodMatrix, _projectionDiameter * neighbourhoodRadius);
    
    //_euclideanMetric.recompute(_dataset, _neighbourhoodMatrix);
    _varianceMetric.recompute(_dataset, _neighbourhoodMatrix);
}

void Explanation::computeNeighbourhoodMatrix(std::vector<std::vector<int>>& neighbourhoodMatrix, float radius)
{
    neighbourhoodMatrix.clear();
    neighbourhoodMatrix.resize(_dataset.rows());
    for (int i = 0; i < _dataset.rows(); i++)
    {
        findNeighbourhood(_projection, i, radius, neighbourhoodMatrix[i]);

        if (i % 1000 == 0) std::cout << i << std::endl;
    }
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
                dimRank = _euclideanMetric.computeDimensionRank(_dataset, si, j);
            if (metric == Metric::VARIANCE)
                dimRank = _varianceMetric.computeDimensionRank(_dataset, si, j);

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

void Explanation::computeValueRanks(Eigen::ArrayXXf& valueRanks)
{
    int numPoints = _dataset.rows();
    int numDimensions = _dataset.cols();

    // Compute ranges
    std::vector<float> minRanges(numDimensions, std::numeric_limits<float>::max());
    std::vector<float> maxRanges(numDimensions, -std::numeric_limits<float>::max());

    for (int j = 0; j < numDimensions; j++)
    {
        for (int i = 0; i < numPoints; i++)
        {
            float value = _dataset(i, j);

            if (value < minRanges[j]) minRanges[j] = value;
            if (value > maxRanges[j]) maxRanges[j] = value;
        }
    }

    // Compute average values
    valueRanks.resize(numPoints, numDimensions);

    for (int j = 0; j < numDimensions; j++)
    {
        float range = maxRanges[j] - minRanges[j];
        float minRange = minRanges[j];

        for (int i = 0; i < numPoints; i++)
        {
            valueRanks(i, j) =  (_dataset(i, j) - minRange) / range;
        }
    }
}

std::vector<float> Explanation::computeConfidences(const Eigen::ArrayXXf& dimRanks)
{
    int numPoints = dimRanks.rows();
    int numDimensions = dimRanks.cols();

    // Compute sorted rankings
    Eigen::ArrayXXi sortingIndices;
    sortingIndices.resize(numPoints, numDimensions);

    for (int i = 0; i < numPoints; i++)
    {
        // Sort the rankings
        std::vector<int> indices(numDimensions);
        std::iota(indices.begin(), indices.end(), 0); //Initializing
        std::sort(indices.begin(), indices.end(), [&](int a, int b) {return dimRanks(i, a) < dimRanks(i, b); });

        for (int j = 0; j < numDimensions; j++)
            sortingIndices(i, j) = indices[j];
    }

    std::vector<float> confidences(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        // Compute sorted top ranking
        std::vector<float> topRankings(numDimensions, 0);
        for (int n = 0; n < _confidenceNeighbourhoodMatrix[i].size(); n++)
        {
            int ni = _confidenceNeighbourhoodMatrix[i][n];

            for (int j = 0; j < 1; j++)
            {
                float rank = dimRanks(ni, sortingIndices(ni, j));
                topRankings[sortingIndices(ni, j)] += rank;
            }
        }
        for (int j = 0; j < 1; j++)
        {
            float rank = dimRanks(i, sortingIndices(i, j));
            topRankings[sortingIndices(i, j)] += rank;
        }

        // Compute total ranking
        Eigen::ArrayXf totalRanking = dimRanks.row(i);
        for (int n = 0; n < _confidenceNeighbourhoodMatrix[i].size(); n++)
        {
            int ni = _confidenceNeighbourhoodMatrix[i][n];
            totalRanking += dimRanks.row(ni);
        }

        // Divide all rankings by total rank values to get confidences
        float topRanking = topRankings[sortingIndices(i, 0)];
        float totalRank = totalRanking[sortingIndices(i, 0)];
        confidences[i] = topRanking /= totalRank;

        if (totalRank == 0)
            confidences[i] = 0;
        //confidences[i] = topRankings[sortingIndices(i, 0)] /= totalRanking[sortingIndices(i, 0)];
    }
    return confidences;
}

void Explanation::computeConfidences2(const Eigen::ArrayXXf& dimRanks, Eigen::ArrayXXf& confidenceMatrix)
{
    int numPoints = dimRanks.rows();
    int numDimensions = dimRanks.cols();

    // Build confidence matrix
    confidenceMatrix.resize(numPoints, numDimensions);
    //qDebug() << "CONF SIZE: " << _confidenceNeighbourhoodMatrix[0].size();
    for (int i = 0; i < numPoints; i++)
    {
        // Compute summed rankings over neighbouring points
        std::vector<float> summedRankings(numDimensions, 0);
        for (int n = 0; n < _confidenceNeighbourhoodMatrix[i].size(); n++)
        {
            int ni = _confidenceNeighbourhoodMatrix[i][n];

            for (int j = 0; j < numDimensions; j++)
            {
                float rank = dimRanks(ni, j);
                summedRankings[j] += rank;
            }
        }

        // Compute total ranking normalization factor
        float totalRanking = 0;
        for (int j = 0; j < numDimensions; j++)
        {
            totalRanking += summedRankings[j];
        }
        if (i == 1000)
        {
            for (int j = 0; j < numDimensions; j++)
            {
                std::cout << "Sum: " << j << " " << summedRankings[j] << std::endl;
            }

            std::cout << "Total ranking: " << totalRanking << std::endl;
        }
        
        for (int j = 0; j < numDimensions; j++)
        {
            summedRankings[j] /= totalRanking;

            confidenceMatrix(i, j) = (1 - summedRankings[j]);
        }
    }
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
