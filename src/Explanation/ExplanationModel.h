#pragma once

#include "DataMatrix.h"

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
    void setProjection(mv::Dataset<Points> projection);

private:
    DataMatrix _dataset;
    DataMatrix _projection;
};

} // namespace Explanation
