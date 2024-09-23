#include "Explanation/ExplanationModel.h"

#include "GridIndex.h"

#include <Dataset.h>
#include <PointData/PointData.h>
#include <PointData/DimensionsPickerAction.h>

namespace Explanation
{

DataMatrix& Model::getDataset()
{
    return _dataset;
}

DataMatrix& Model::getProjection()
{
    return _projection;
}

void Model::setProjection(mv::Dataset<Points> projection)
{
    DataMatrix::fromDataset(projection->getSourceDataset<Points>(), _dataset);
    DataMatrix::fromDataset(projection, _projection);

    computeStatistics();

    _selectionRanking.resize(_dataset.getNumCols());
}

DataStatistics& Model::getDataStatistics()
{
    return _dataStatistics;
}

Lens& Model::getLens()
{
    return _lens;
}

float computeProjectionDiameter(DataMatrix& projection, int xDim, int yDim, Bounds& bounds)
{
    float minX = std::numeric_limits<float>::max(), maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max(), maxY = -std::numeric_limits<float>::max();
    for (int i = 0; i < projection.getNumRows(); i++)
    {
        float x = projection(i, xDim);
        float y = projection(i, yDim);

        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }
    float rangeX = maxX - minX;
    float rangeY = maxY - minY;

    bounds = Bounds(minX, maxX, minY, maxY);
    qDebug() << "Bounds Left: " << bounds.getLeft() << "minX: " << minX;
    qDebug() << "Bounds Right: " << bounds.getRight() << "maxX: " << maxX;
    float diameter = rangeX > rangeY ? rangeX : rangeY;
    return diameter;
}

void findNeighbourhood(DataMatrix& projection, int centerId, float radius, std::vector<int>& neighbourhood, int xDim, int yDim)
{
    float x = projection(centerId, xDim);
    float y = projection(centerId, yDim);

    float radSquared = radius * radius;

    neighbourhood.clear();

    for (int i = 0; i < projection.getNumRows(); i++)
    {
        float xd = projection(i, xDim) - x;
        if (abs(xd) > radius) continue;
        float yd = projection(i, yDim) - y;
        if (abs(yd) > radius) continue;
        float magSquared = xd * xd + yd * yd;

        if (magSquared > radSquared)
            continue;

        neighbourhood.push_back(i);
    }
}

using Neighbourhood = std::vector<int>;
using NeighbourhoodMatrix = std::vector<Neighbourhood>;
void computeNeighbourhoodMatrix(DataMatrix& projection, NeighbourhoodMatrix& neighbourhoodMatrix, float radius, int xDim, int yDim, GridIndex& gridIndex)
{
    auto start = std::chrono::high_resolution_clock::now();

    neighbourhoodMatrix.clear();
    neighbourhoodMatrix.resize(projection.getNumRows());

    for (int i = 0; i < projection.getNumRows(); i++)
    {
        findNeighbourhood(projection, i, radius, neighbourhoodMatrix[i], xDim, yDim);

        if (i % 10000 == 0) std::cout << "Computing neighbourhood for points: [" << i << "/" << projection.getNumRows() << "]" << std::endl;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Neighbourhood Elapsed time : " << elapsed.count() << " s\n";
}

void Model::computeExplanationMethod()
{
    _colorMapping.recreate(_dataset);

    Bounds bounds;
    float projectionDiameter = computeProjectionDiameter(_projection, 0, 1, bounds);
    bounds.expand(0.001);

    GridIndex gridIndex(bounds, 16);

    for (int i = 0; i < _projection.getNumRows(); i++)
    {
        gridIndex.addPoint(i, _projection(i, 0), _projection(i, 1));
        if (i % 100000 == 0) qDebug() << "Grid points: " << i;
    }
    //NeighbourhoodMatrix matrix;
    //computeNeighbourhoodMatrix(_projection, matrix, projectionDiameter * 0.1, 0, 1, gridIndex);

    _valueMethod.recompute(_dataset, _projection, gridIndex);
}

void Model::computeDimensionRanks()
{
    std::vector<unsigned int> selection(_dataset.getNumRows());
    std::iota(selection.begin(), selection.end(), 0);

    std::cout << "Selection size: " << selection.size() << std::endl;
    _dimRanking.resize(selection.size(), _dataset.getNumCols());
    for (int i = 0; i < selection.size(); i++)
    {
        int si = selection[i];

        for (int j = 0; j < _dataset.getNumCols(); j++)
        {
            _dimRanking(i, j) = _valueMethod.computeDimensionRank(_dataset, si, j);
        }
    }

    int numPoints = _dimRanking.getNumRows();
    int numDimensions = _dimRanking.getNumCols();
    _rankAggregation.resize(numDimensions, 0);
    for (int i = 0; i < _dimRanking.getNumRows(); i++)
    {
        for (int j = 0; j < numDimensions; j++)
        {
            _rankAggregation[j] += _dimRanking(i, j);
        }
    }
    for (int i = 0; i < numDimensions; i++)
    {
        _rankAggregation[i] /= (float)numPoints;
    }

    _colorMapping.recompute(_dataset, _dimRanking);
}

void Model::computeSelectionDimensionRanks(std::vector<unsigned int>& selection)
{
    _valueMethod.computeDimensionRank(_dataset, selection, _selectionRanking);
}

void Model::computeStatistics()
{
    int numPoints = _dataset.getNumRows();
    int numDimensions = _dataset.getNumCols();

    _dataStatistics.means.clear();
    _dataStatistics.variances.clear();
    _dataStatistics.minRange.clear();
    _dataStatistics.maxRange.clear();
    _dataStatistics.ranges.clear();

    _dataStatistics.means.resize(numDimensions);
    _dataStatistics.variances.resize(numDimensions);
    _dataStatistics.minRange.resize(numDimensions, std::numeric_limits<float>::max());
    _dataStatistics.maxRange.resize(numDimensions, -std::numeric_limits<float>::max());
    _dataStatistics.ranges.resize(numDimensions, 0);

    for (int j = 0; j < numDimensions; j++)
    {
        // Compute mean
        float mean = 0;
        for (int i = 0; i < numPoints; i++)
        {
            float value = _dataset(i, j);

            if (value < _dataStatistics.minRange[j]) _dataStatistics.minRange[j] = value;
            if (value > _dataStatistics.maxRange[j]) _dataStatistics.maxRange[j] = value;
            mean += value;
        }
        mean /= numPoints;
        _dataStatistics.means[j] = mean;

        // Compute variance
        float variance = 0;
        for (int i = 0; i < numPoints; i++)
        {
            float x = _dataset(i, j) - mean;
            variance += x * x;
        }
        variance /= numPoints;
        _dataStatistics.variances[j] = variance;
    }
    for (int j = 0; j < numDimensions; j++)
    {
        _dataStatistics.ranges[j] = _dataStatistics.maxRange[j] - _dataStatistics.minRange[j];
        if (_dataStatistics.ranges[j] == 0) _dataStatistics.ranges[j] = 1;

        std::cout << j << ": " << "Means : " << _dataStatistics.means[j] << " Variances : " << _dataStatistics.variances[j] << " Min range : " << _dataStatistics.minRange[j] << " Max range : " << _dataStatistics.maxRange[j] << std::endl;
    }
}

} // namespace Explanation
