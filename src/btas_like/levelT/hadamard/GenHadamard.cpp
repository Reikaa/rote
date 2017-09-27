/*
   Copyright (c) 2009-2013, Jack Poulson
                      2013, Jeff Hammond
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "rote.hpp"
#include "rote/core/dist_tensor_forward_decl.hpp"

namespace rote{

template <typename T>
void Hadamard<T>::run(
  const DistTensor<T>& A, const std::string& indicesA,
  const DistTensor<T>& B, const std::string& indicesB,
        DistTensor<T>& C, const std::string& indicesC,
  const std::vector<Unsigned>& blkSizes
) {
  // Convert to index array
	IndexArray indA(indicesA.size());
	for(int i = 0; i < indicesA.size(); i++)
		indA[i] = indicesA[i];

	IndexArray indB(indicesB.size());
	for(int i = 0; i < indicesB.size(); i++)
		indB[i] = indicesB[i];

	IndexArray indC(indicesC.size());
	for(int i = 0; i < indicesC.size(); i++)
		indC[i] = indicesC[i];

  //Determine Stationary variant.
  const Unsigned numElemA = prod(A.Shape());
  const Unsigned numElemB = prod(B.Shape());
  const Unsigned numElemC = prod(C.Shape());

  bool isBiggerAB = numElemA > numElemB;
  bool isBiggerAC = numElemA > numElemC;
  bool isBiggerBC = numElemB > numElemC;

  bool isEqualAB = numElemA == numElemB;

  bool isSmallerAB = numElemA < numElemB;
  bool isSmallerAC = numElemA < numElemC;

  bool isSmallerEqualAC = numElemA <= numElemC;
  bool isSmallerEqualBC = numElemB <= numElemC;

  bool isBiggerEqualAB = numElemA >= numElemB;
  bool isBiggerEqualAC = numElemA >= numElemC;

  if(isBiggerEqualAB && isBiggerAC){
  	Hadamard::run(
      A, indA,
      B, indB,
      C, indC,
      blkSizes,
      false
    );
  } else if((isSmallerAB && isBiggerEqualAC) ||
  		 (isSmallerAB && isSmallerAC && isBiggerBC)){
  	Hadamard::run(
      B, indB,
      A, indA,
      C, indC,
      blkSizes,
      false
    );
  } else if((isBiggerAB && isSmallerEqualAC) ||
  		 (isEqualAB && isSmallerEqualAC) ||
		 (isSmallerAB && isSmallerAC && isSmallerEqualBC)){
  	Hadamard::run(
      A, indA,
      B, indB,
      C, indC,
      blkSizes,
      false
    );
  } else {
  	LogicError("Should never occur");
  }
}

//Non-template functions
//bool AnyFalseElem(const std::vector<bool>& vec);
#define PROTO(T) \
  template class Hadamard<T>;

//PROTO(Unsigned)
//PROTO(Int)
PROTO(float)
PROTO(double)
//PROTO(char)

} // namespace rote