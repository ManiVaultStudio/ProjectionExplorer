#pragma once

#include <Eigen/Eigen>

namespace mv
{
    template<class T>
    class Dataset;
}
class Points;

class DataMatrix
{
public:
    static void fromDataset(mv::Dataset<Points> dataset, DataMatrix& dataMatrix);

private:
    Eigen::ArrayXXf _data;
};
