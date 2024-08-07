#include "DataMatrix.h"

#include <PointData/PointData.h>
#include <PointData/DimensionsPickerAction.h>

#include <QDebug> //////////////////
#include "util/Timer.h" ////////////

void DataMatrix::fromDataset(mv::Dataset<Points> dataset, DataMatrix& dataMatrix)
{
    Timer t("Copy");

    int numPoints = dataset->getNumPoints();
    int numDimensions = dataset->getNumDimensions();

    dataMatrix._data.resize(numPoints, numDimensions);

    // Copy the full data to the data matrix
    dataset->visitFromBeginToEnd([&dataMatrix, &dataset, numPoints, numDimensions](auto begin, auto end)
        {
            std::copy(begin, end, dataMatrix._data.data());
        });

    // Find the list of enabled dimension indices
    std::vector<bool> enabledDimBools = dataset->getDimensionsPickerAction().getEnabledDimensions();
    int numEnabledDims = std::count(enabledDimBools.begin(), enabledDimBools.end(), true);
    std::vector<int> enabledDims(numEnabledDims);
    int d = 0;
    for (int i = 0; i < enabledDimBools.size(); i++)
    {
        if (enabledDimBools[i])
            enabledDims[d++] = i;
    }

    // Only retain the enabled dimensions in the data matrix
    dataMatrix._data = dataMatrix._data(Eigen::all, enabledDims);
}
