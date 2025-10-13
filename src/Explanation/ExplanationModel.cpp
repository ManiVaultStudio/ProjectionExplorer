#include "Explanation/ExplanationModel.h"

#include <Dataset.h>
#include <PointData/PointData.h>
#include <PointData/DimensionsPickerAction.h>
#include <graphics/Bounds.h>

#include "Globals.h" /////// Temp

namespace
{
    float computeProjectionDiameter(DataMatrix& projection, int xDim, int yDim)
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

        //bounds = Bounds(minX, maxX, minY, maxY);
        //qDebug() << "Bounds Left: " << bounds.getLeft() << "minX: " << minX;
        //qDebug() << "Bounds Right: " << bounds.getRight() << "maxX: " << maxX;
        float diameter = rangeX > rangeY ? rangeX : rangeY;
        return diameter;
    }

    void computeTopRankedDims(const DataMatrix& dataset, const DataMatrix& dimRanking, std::vector<int>& topRankedDims)
    {
        // Build vector of top ranked dimensions
        topRankedDims.clear();
        topRankedDims.resize(dimRanking.getNumRows());

        // Top dimension
        int d = 0;

        std::vector<int> indices(dimRanking.getNumCols());

        for (int i = 0; i < dimRanking.getNumRows(); i++)
        {
            std::iota(indices.begin(), indices.end(), 0);
            std::sort(indices.begin(), indices.end(), [&](int a, int b) {return dimRanking(i, a) > dimRanking(i, b); });

            //while (dataset.isExcluded(indices[j]) && j < dataset.numDimensions() - 1) { j++; }
            topRankedDims[i] = indices[d];
        }
    }
}

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
    DataMatrix::fromDataset(projection, _projection, true);

    computeStatistics();

    _selectionRanking.clear();
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

void Model::computeExplanationMethod()
{
    _colorMapping.recreate(_dataset);

    float projectionDiameter = computeProjectionDiameter(_projection, 0, 1);

    //NeighbourhoodMatrix matrix;
    //computeNeighbourhoodMatrix(_projection, matrix, projectionDiameter * 0.1, 0, 1, gridIndex);

    _valueMethod.recompute(_dataset, _projection);
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
    _rankAggregation.clear();
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

    computeTopRankedDims(_dataset, _dimRanking, _topRankedDimensions);

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
