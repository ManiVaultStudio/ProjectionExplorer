#pragma once

#include <QObject>
#include <QColor>

#include "PointData.h"

#include "Methods/ExplanationMethod.h"
#include "ColorMapping.h"

#include "Methods/SilvaEuclidean.h"
#include "Methods/SilvaVariance.h"
#include "Methods/ValueRanking.h"

using namespace hdps;

//class Data

class DataStatistics
{
public:
    std::vector<float> means;
    std::vector<float> variances;
    std::vector<float> minRange;
    std::vector<float> maxRange;
};

class ExplanationModel : public QObject
{
    Q_OBJECT
public:
    ExplanationModel();

    bool hasDataset() { return _hasDataset; }
    const DataMatrix& getDataset() { return _dataset; }
    const DataStatistics& getDataStatistics() { return _dataStats; }
    const std::vector<QString>& getDataNames() { return _dimensionNames; }

    Explanation::Metric currentMetric() { return _explanationMetric; }
    const std::vector<QColor>& getColorMapping() { return _colorMapping.getColors(); }

    void setDataset(Dataset<Points> dataset, Dataset<Points> projection);
    void recomputeNeighbourhood(float neighbourhoodRadius);
    void recomputeMetrics();
    void recomputeColorMapping(DataMatrix& dimRanks);

    void excludeDimension(int dim);
    bool isExcluded(int dim) { return _exclusionList[dim]; }

    void setExplanationMetric(Explanation::Metric metric);
    void computeDimensionRanks(std::vector<float>& dimRanking, std::vector<unsigned int>& selection);
    void computeDimensionRanks(DataMatrix& dimRanking);

    std::vector<float> computeConfidences(const DataMatrix& dimRanks);

signals:
    void explanationMetricChanged(Explanation::Metric metric);

private:
    /**
     * For every point in the projection compute the indices of the points
     * in its neighbourhood and add them to the matrix.
     */
    void computeNeighbourhoodMatrix(NeighbourhoodMatrix& neighbourhoodMatrix, float radius);

    Explanation::Method* getCurrentExplanationMethod();

private:
    bool                    _hasDataset;

    DataMatrix              _dataset;
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

    /** List of dimensions to exclude from analysis */
    std::vector<bool>       _exclusionList;
};
