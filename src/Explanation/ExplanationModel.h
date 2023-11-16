#pragma once

#include <QObject>
#include <QColor>

#include "PointData/PointData.h"

#include "DataTypes.h"
#include "Methods/ExplanationMethod.h"
#include "ColorMapping.h"

#include "Methods/SilvaEuclidean.h"
#include "Methods/SilvaVariance.h"
#include "Methods/ValueRanking.h"

class DataStatistics
{
public:
    std::vector<float> means;
    std::vector<float> variances;
    std::vector<float> minRange;
    std::vector<float> maxRange;
    std::vector<float> ranges;
};

class ExplanationModel : public QObject
{
    Q_OBJECT
public:
    ExplanationModel();

    bool hasDataset() { return _hasDataset; }
    const DataTable& getDataset() { return _dataset; }
    const DataStatistics& getDataStatistics() { return _dataStats; }
    const std::vector<QString>& getDataNames() { return _dimensionNames; }

    Explanation::Metric currentMetric() { return _explanationMetric; }
    const std::vector<QColor>& getColorMapping() { return _colorMapping.getColors(); }

    void resetDataset() { _hasDataset = false; }
    void setDataset(mv::Dataset<Points> dataset, mv::Dataset<Points> projection);
    void recomputeNeighbourhood(float neighbourhoodRadius, int xDim, int yDim);
    void recomputeMetrics();
    void recomputeColorMapping(DataMatrix& dimRanks);

    void excludeDimension(int dim);

    void setExplanationMetric(Explanation::Metric metric);
    void computeDimensionRanks(std::vector<float>& dimRanking, std::vector<unsigned int>& selection);
    void computeDimensionRanks(DataMatrix& dimRanking);

    std::vector<float> computeConfidences(const DataMatrix& dimRanks);

signals:
    void datasetChanged();
    void explanationMetricChanged(Explanation::Metric metric);

    void datasetDimensionsChanged();

private:
    /** Initialize model */
    void initialize();

    /**
     * For every point in the projection compute the indices of the points
     * in its neighbourhood and add them to the matrix.
     */
    void computeNeighbourhoodMatrix(NeighbourhoodMatrix& neighbourhoodMatrix, float radius, int xDim, int yDim);

    Explanation::Method* getCurrentExplanationMethod();

private:
    bool                    _hasDataset;

    DataTable               _dataset;
    DataMatrix              _standardizedDataset;
    DataMatrix              _normDataset;
    DataMatrix              _projection;
    DataStatistics          _dataStats;
    std::vector<QString>    _dimensionNames;

    ColorMapping            _colorMapping;

    /** Largest extent of the projection */
    float                   _projectionDiameter;

    /** Matrix of neighbourhood indices for every point in the projection */
    NeighbourhoodMatrix     _neighbourhoodMatrix;

    NeighbourhoodMatrix     _confidenceNeighbourhoodMatrix;

    // Explanation metrics
    /** Enum of which method is currently selected */
    Explanation::Metric     _explanationMetric;
    /** Da Silva euclidean-based explanation method */
    EuclideanMethod         _euclideanMethod;
    /** Da Silva variance-based explanation method */
    VarianceMethod          _varianceMethod;
    /** Value-based explanation method */
    ValueMethod             _valueMethod;
};
