#include "cpptasktest.h"

#include <cpptask/cpptask.h>

#include <algorithm>

TEST_F(CppTaskTest, ForEach_Serial)
{
    cpptask::TaskThreadPool threadPool(0);

    cpptask::for_each(testArray.begin(), testArray.end(), DoubleSqrt());
}

TEST_F(CppTaskTest, ForEach_Parallel)
{
    cpptask::TaskThreadPool threadPool(4);

    cpptask::for_each(testArray.begin(), testArray.end(), DoubleSqrt());
}

