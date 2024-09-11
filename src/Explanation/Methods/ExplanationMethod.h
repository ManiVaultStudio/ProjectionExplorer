#pragma once

#include "Explanation/DataMatrix.h"

namespace Explanation
{
    class Method
    {
    public:
        virtual void recompute(DataMatrix& dataset) = 0;
    };
}
