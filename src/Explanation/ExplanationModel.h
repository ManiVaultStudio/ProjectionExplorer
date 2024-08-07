#pragma once

#include "DataMatrix.h"
#include "Lens.h"

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
    DataMatrix& getProjection();
    void setProjection(mv::Dataset<Points> projection);

    Lens& getLens();

private:
    DataMatrix      _dataset;
    DataMatrix      _projection;

    Lens            _lens;
};

} // namespace Explanation
