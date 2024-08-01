#include "DataMatrix.h"

#include <PointData/PointData.h>
#include <PointData/DimensionsPickerAction.h>

#include <QDebug> //////////////////
#include "util/Timer.h" ////////////

void DataMatrix::fromDataset(mv::Dataset<Points> dataset, DataMatrix& dataMatrix)
{
    int numPoints = dataset->getNumPoints();
    int numDimensions = dataset->getNumDimensions();

    std::vector<bool> enabledDims = dataset->getDimensionsPickerAction().getEnabledDimensions();
    int numEnabledDims = std::count(enabledDims.begin(), enabledDims.end(), true);

    dataMatrix._data.resize(numPoints, numEnabledDims);

    Timer t("Copy");

    dataset->visitFromBeginToEnd([&dataMatrix, &dataset, numPoints, numEnabledDims, enabledDims](auto begin, auto end)
        {
            for (int d = 0; d < numEnabledDims; d++)
            {
                int dim = enabledDims[d];
                std::copy(begin + dim * numPoints, begin + (dim + 1) * numPoints, dataMatrix._data.data() + d * numPoints);
            }
        });

    qDebug() << dataMatrix._data(0, 0);
}
