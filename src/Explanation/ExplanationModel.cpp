#include "Explanation/ExplanationModel.h"

#include <Dataset.h>
#include <PointData/PointData.h>
#include <PointData/DimensionsPickerAction.h>

namespace Explanation
{

DataMatrix& Model::getProjection()
{
    return _projection;
}

void Model::setProjection(mv::Dataset<Points> projection)
{
    DataMatrix::fromDataset(projection->getSourceDataset<Points>(), _dataset);
    DataMatrix::fromDataset(projection, _projection);
}

Lens& Model::getLens()
{
    return _lens;
}

} // namespace Explanation
