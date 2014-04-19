/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef TMEN_CORE_DISTTENSOR_REDISTRIBUTE_DECL_HPP
#define TMEN_CORE_DISTTENSOR_REDISTRIBUTE_DECL_HPP

#include "tensormental/core/imports/mpi.hpp"
#include "tensormental/core/imports/choice.hpp"
#include "tensormental/core/imports/mpi_choice.hpp"

#include <vector>
#include <iostream>
#include "tensormental/core/dist_tensor/mc_mr.hpp"

namespace tmen{

/////////////////
//Check routines
/////////////////

template<typename T>
Int CheckPermutationRedist(const DistTensor<T>& B, const DistTensor<T>& A, const Index permuteIndex);

template<typename T>
Int CheckPartialReduceScatterRedist(const DistTensor<T>& B, const DistTensor<T>& A, const Index reduceScatterIndex);

template<typename T>
Int CheckReduceScatterRedist(const DistTensor<T>& B, const DistTensor<T>& A, const Index reduceIndex, const Index scatterIndex);

template<typename T>
Int CheckAllGatherRedist(const DistTensor<T>& B, const DistTensor<T>& A, const Index allGatherIndex);

template<typename T>
Int CheckAllToAllDoubleIndexRedist(const DistTensor<T>& B, const DistTensor<T>& A, const std::pair<Index, Index>& allToAllIndices, const std::pair<ModeArray, ModeArray >& a2aCommGroups);

template<typename T>
Int CheckLocalRedist(DistTensor<T>& B, const DistTensor<T>& A, const Index localIndex, const ModeArray& gridRedistModes);

template<typename T>
Int CheckRemoveUnitIndicesRedist(const DistTensor<T>& B, const DistTensor<T>& A, const IndexArray& unitIndices);

template<typename T>
Int CheckIntroduceUnitIndicesRedist(const DistTensor<T>& B, const DistTensor<T>& A, const std::vector<Unsigned>& newIndexPositions);

//template<typename T>
//int CheckPartialReduceScatterRedist(const DistTensor<T>& B, const DistTensor<T>& A, const Index index, const ModeArray& rsGridModes);

//TODO: Not entirely correct definition
template<typename T>
Int CheckAllToAllRedist(const DistTensor<T>& B, const DistTensor<T>& A, const Index allToAllIndex);


/////////////////
//Redist routines
/////////////////

template<typename T>
void PermutationRedist(DistTensor<T>& B, const DistTensor<T>& A, const Index permuteIndex);

template<typename T>
void PartialReduceScatterRedist(DistTensor<T>& B, const DistTensor<T>& A, const Index reduceScatterIndex);

template<typename T>
void ReduceScatterRedist(DistTensor<T>& B, const DistTensor<T>& A, const Index reduceIndex, const Index scatterIndex);

template<typename T>
void AllGatherRedist(DistTensor<T>& B, const DistTensor<T>& A, const Index allGatherIndex);

template<typename T>
void AllToAllDoubleIndexRedist(DistTensor<T>& B, const DistTensor<T>& A, const std::pair<Index, Index>& a2aIndices, const std::pair<ModeArray, ModeArray >& a2aCommGroups);

template<typename T>
void LocalRedist(DistTensor<T>& B, const DistTensor<T>& A, const Index localIndex, const ModeArray& gridRedistModes);

template<typename T>
void RemoveUnitIndicesRedist(DistTensor<T>& B, const DistTensor<T>& A, const IndexArray& unitIndices);

template<typename T>
void IntroduceUnitIndicesRedist(const DistTensor<T>& B, const DistTensor<T>& A, const Unsigned& nIndices);

//template<typename T>
//void PartialReduceScatterRedist(DistTensor<T>& B, const DistTensor<T>& A, const Index reduceScatterIndex, const ModeArray& rsGridModes);

//TODO: Not entirely correct definition
template<typename T>
void AllToAllRedist(DistTensor<T>& B, const DistTensor<T>& A, const Index index);


}
#endif // ifndef TMEN_CORE_DISTTENSOR_REDISTRIBUTE_DECL_HPP
