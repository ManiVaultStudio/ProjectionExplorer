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

class Model
{
public:
    DataMatrix& getDataset();
    DataMatrix& getProjection();
    void setProjection(mv::Dataset<Points> projection);

    Lens& getLens();
    ColorMapping& getColorMapping() { return _colorMapping; }

    void computeExplanationMethod();
    void computeDimensionRanks(DataMatrix& dimRanks);
    void computeSelectionDimensionRanks(std::vector<float>& dimRanking, std::vector<unsigned int>& selection);

private:
    DataMatrix              _dataset;
    DataMatrix              _projection;

    Lens                    _lens;

    Explanation::Method*    _method;
    /** Value-based explanation method */
    ValueMethod             _valueMethod;

    ColorMapping            _colorMapping;
};

} // namespace Explanation
