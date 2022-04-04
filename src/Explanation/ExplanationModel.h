#pragma once

#include <QObject>
#include <QColor>

#include "PointData.h"

#include "Methods/ExplanationMethod.h"

#include "Methods/SilvaEuclidean.h"
#include "Methods/SilvaVariance.h"
#include "Methods/ValueRanking.h"

using namespace hdps;

class ExplanationModel : public QObject
{
    Q_OBJECT
public:
    ExplanationModel();

    bool hasDataset() { return _hasDataset; }
    const DataMatrix& getDataset() { return _dataset; }
    const DataMatrix& getDataRanges() { return _dataRanges; }
    const std::vector<QString>& getDataNames() { return _dimensionNames; }

    Explanation::Metric currentMetric() { return _explanationMetric; }
    const std::vector<QColor>& getColorMapping() { return _colorMapping; }

    void setDataset(Dataset<Points> dataset, Dataset<Points> projection);
    void recomputeNeighbourhood(float neighbourhoodRadius);
    void recomputeMetrics();
    void recomputeColorMapping(DataMatrix& dimRanks);

    void setExplanationMetric(Explanation::Metric metric);
    void computeDimensionRanks(DataMatrix& dimRanks, std::vector<unsigned int>& selection);
    void computeDimensionRanks(DataMatrix& dimRanks);

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
    DataMatrix              _normDataset;
    DataMatrix              _projection;
    DataMatrix              _dataRanges;
    std::vector<QString>    _dimensionNames;

    std::vector<QColor>     _palette;
    std::vector<QColor>     _colorMapping;

    /** Largest extent of the projection */
    float                   _projectionDiameter;

    /** Matrix of neighbourhood indices for every point in the projection */
    NeighbourhoodMatrix     _neighbourhoodMatrix;

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
