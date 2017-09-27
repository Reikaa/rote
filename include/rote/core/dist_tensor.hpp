/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef ROTE_CORE_DISTTENSOR_HPP
#define ROTE_CORE_DISTTENSOR_HPP

#include "rote/core/types_decl.hpp"

namespace rote {
struct DistData
{
    TensorDistribution distribution;
    IndexArray indices;
    std::vector<Unsigned> modeAlignments;
//    Int root;     // only relevant for [o ,o ]
//    Int diagPath; // only relevant for [MD,* ]/[* ,MD] distributions
    const Grid* grid;
};
} // namespace rote

#include "rote/core/dist_tensor/dist_tensor.hpp"
#include "rote/core/dist_tensor/redist_tensor.hpp"

#endif // ifndef ROTE_CORE_DISTTENSOR_HPP