/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/

#pragma once
#ifndef ROTE_CORE_UTIL_BTAS_UTIL_HPP
#define ROTE_CORE_UTIL_BTAS_UTIL_HPP

namespace rote{

std::vector<ModeArray> DetermineContractModes(const IndexArray& indicesA, const IndexArray& indicesB, const IndexArray& indicesC);
IndexArray DetermineContractIndices(const IndexArray& indicesA, const IndexArray& indicesB);

void SetTensorShapeToMatch(const ObjShape& matchAgainst, const IndexArray& indicesMatchAgainst, ObjShape& toMatch, const IndexArray& indicesToMatch);

}
#endif // ifndef ROTE_CORE_UTIL_REDISTRIBUTE_UTIL_HPP
