#pragma once

#include "DataMatrix.h"
#include "Lens.h"

#include "Explanation/ColorMapping.h"
#include "Explanation/Methods/ExplanationMethod.h"
#include "Explanation/Methods/ValueMethod.h"

class Points;

namespace mv
{
    template<class T>
    class Dataset;
}

namespace Explanation
{

class DataStatistics
{
public:
    std::vector<float> means;
    std::vector<float> variances;
    std::vector<float> minRange;
    std::vector<float> maxRange;
    std::vector<float> ranges;
};

class Model
{
public:
    DataMatrix& getDataset();
    DataMatrix& getProjection();
    void setProjection(mv::Dataset<Points> projection);

    DataStatistics& getDataStatistics();
    DataMatrix& getDimRanking() { return _dimRanking; }
    const std::vector<int>& getTopRankedDims() { return _topRankedDimensions; }
    const std::vector<float>& getSelectionDimRanking() const { return _selectionRanking; }
    const std::vector<float>& getRankAggregation() const { return _rankAggregation; }

    Lens& getLens();
    ColorMapping& getColorMapping() { return _colorMapping; }

    void computeExplanationMethod();
    void computeDimensionRanks();
    void computeSelectionDimensionRanks(std::vector<unsigned int>& selection);

private:
    void computeStatistics();

private:
    DataMatrix              _dataset;
    DataMatrix              _projection;
    DataStatistics          _dataStatistics;

    Lens                    _lens;

    Explanation::Method*    _method;
    /** Value-based explanation method */
    ValueMethod             _valueMethod;

    DataMatrix              _dimRanking;
    std::vector<int>        _topRankedDimensions;

    std::vector<float>      _selectionRanking;
    std::vector<float>      _rankAggregation;

    ColorMapping            _colorMapping;
};

} // namespace Explanation
