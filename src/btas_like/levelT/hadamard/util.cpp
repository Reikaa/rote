/*
   Copyright (c) 2009-2013, Jack Poulson
                      2013, Jeff Hammond
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "rote.hpp"

namespace rote{

// Partition helpers
template <typename T>
void Hadamard<T>::runHelperPartitionBC(
	Unsigned depth, BlkHadamardStatCInfo& hadamardInfo,
	const DistTensor<T>& A, const IndexArray& indicesA,
	const DistTensor<T>& B, const IndexArray& indicesB,
	      DistTensor<T>& C, const IndexArray& indicesC
) {
	if(depth == hadamardInfo.partModesBCB.size()){
		if(hadamardInfo.isStatC) {
			DistTensor<T> intA(hadamardInfo.distIntA, A.Grid());
			intA.SetLocalPermutation(hadamardInfo.permA);
			intA.AlignModesWith(hadamardInfo.alignModesA, C, hadamardInfo.alignModesATo);
			intA.RedistFrom(A);

			DistTensor<T> intB(hadamardInfo.distIntB, B.Grid());
			intB.AlignModesWith(hadamardInfo.alignModesB, C, hadamardInfo.alignModesBTo);
			intB.SetLocalPermutation(hadamardInfo.permB);
			intB.RedistFrom(B);

			Hadamard<T>::run(
				intA.LockedTensor(), hadamardInfo.permA.applyTo(indicesA),
				intB.LockedTensor(), hadamardInfo.permB.applyTo(indicesB),
				C.Tensor(), indicesC
			);
		} else {
			DistTensor<T> intC(hadamardInfo.distIntC, C.Grid());
			intC.SetLocalPermutation(hadamardInfo.permC);
			intC.AlignModesWith(hadamardInfo.alignModesC, A, hadamardInfo.alignModesCTo);
			intC.RedistFrom(C);

			DistTensor<T> intB(hadamardInfo.distIntB, B.Grid());
			intB.AlignModesWith(hadamardInfo.alignModesB, A, hadamardInfo.alignModesBTo);
			intB.SetLocalPermutation(hadamardInfo.permB);
			intB.RedistFrom(B);

			Hadamard<T>::run(
				A.LockedTensor(), indicesA,
				intB.LockedTensor(), hadamardInfo.permB.applyTo(indicesB),
				intC.Tensor(), hadamardInfo.permB.applyTo(indicesC)
			);
			C.RedistFrom(intC);
		}
		return;
	}

	//Must partition and recur
	//Note: pull logic out
	Unsigned blkSize = hadamardInfo.blkSizes[depth];
	Mode partModeB = hadamardInfo.partModesBCB[depth];
	Mode partModeC = hadamardInfo.partModesBCC[depth];
	DistTensor<T> B_T(B.TensorDist(), B.Grid());
	DistTensor<T> B_B(B.TensorDist(), B.Grid());
	DistTensor<T> B_0(B.TensorDist(), B.Grid());
	DistTensor<T> B_1(B.TensorDist(), B.Grid());
	DistTensor<T> B_2(B.TensorDist(), B.Grid());

	DistTensor<T> C_T(C.TensorDist(), C.Grid());
	DistTensor<T> C_B(C.TensorDist(), C.Grid());
	DistTensor<T> C_0(C.TensorDist(), C.Grid());
	DistTensor<T> C_1(C.TensorDist(), C.Grid());
	DistTensor<T> C_2(C.TensorDist(), C.Grid());

	//Do the partitioning and looping
	int count = 0;
	LockedPartitionDown(B, B_T, B_B, partModeB, 0);
	PartitionDown(C, C_T, C_B, partModeC, 0);
	while(B_T.Dimension(partModeB) < B.Dimension(partModeB)){
		LockedRepartitionDown(B_T, B_0,
						/**/ /**/
							 B_1,
						B_B, B_2, partModeB, blkSize);
		RepartitionDown(C_T, C_0,
						/**/ /**/
							 C_1,
						C_B, C_2, partModeC, blkSize);

		/*----------------------------------------------------------------*/
		Hadamard<T>::runHelperPartitionBC(depth+1, hadamardInfo, A, indicesA, B_1, indicesB, C_1, indicesC);

		count++;
		/*----------------------------------------------------------------*/
		SlideLockedPartitionDown(B_T, B_0,
				                B_1,
						   /**/ /**/
						   B_B, B_2, partModeB);
	 SlidePartitionDown(C_T, C_0,
				                C_1,
						   /**/ /**/
						   C_B, C_2, partModeC);
	}
}

template <typename T>
void Hadamard<T>::runHelperPartitionAC(
	Unsigned depth, BlkHadamardStatCInfo& hadamardInfo,
	const DistTensor<T>& A, const IndexArray& indicesA,
	const DistTensor<T>& B, const IndexArray& indicesB,
	      DistTensor<T>& C, const IndexArray& indicesC
) {
	if(depth == hadamardInfo.partModesACA.size()){
		Hadamard<T>::runHelperPartitionBC(0, hadamardInfo, A, indicesA, B, indicesB, C, indicesC);
		return;
	}

	//Must partition and recur
	//Note: pull logic out
	Unsigned blkSize = hadamardInfo.blkSizes[depth];
	Mode partModeA = hadamardInfo.partModesACA[depth];
	Mode partModeC = hadamardInfo.partModesACC[depth];
	DistTensor<T> A_T(A.TensorDist(), A.Grid());
	DistTensor<T> A_B(A.TensorDist(), A.Grid());
	DistTensor<T> A_0(A.TensorDist(), A.Grid());
	DistTensor<T> A_1(A.TensorDist(), A.Grid());
	DistTensor<T> A_2(A.TensorDist(), A.Grid());

	DistTensor<T> C_T(C.TensorDist(), C.Grid());
	DistTensor<T> C_B(C.TensorDist(), C.Grid());
	DistTensor<T> C_0(C.TensorDist(), C.Grid());
	DistTensor<T> C_1(C.TensorDist(), C.Grid());
	DistTensor<T> C_2(C.TensorDist(), C.Grid());

	//Do the partitioning and looping
	int count = 0;
	LockedPartitionDown(A, A_T, A_B, partModeA, 0);
	PartitionDown(C, C_T, C_B, partModeC, 0);
	while(A_T.Dimension(partModeA) < A.Dimension(partModeA)){
		LockedRepartitionDown(A_T, A_0,
				        /**/ /**/
				             A_1,
				        A_B, A_2, partModeA, blkSize);
		RepartitionDown(C_T, C_0,
						/**/ /**/
							 C_1,
						C_B, C_2, partModeC, blkSize);

		/*----------------------------------------------------------------*/
		Hadamard<T>::runHelperPartitionAC(depth+1, hadamardInfo, A_1, indicesA, B, indicesB, C_1, indicesC);
		count++;
		/*----------------------------------------------------------------*/
		SlideLockedPartitionDown(A_T, A_0,
				                A_1,
						   /**/ /**/
						   A_B, A_2, partModeA);
	 SlidePartitionDown(C_T, C_0,
				                C_1,
						   /**/ /**/
						   C_B, C_2, partModeC);
	}
}

// Struct interface
template<typename T>
void Hadamard<T>::setHadamardInfo(
	const DistTensor<T>& A, const IndexArray& indicesA,
  const DistTensor<T>& B, const IndexArray& indicesB,
  const DistTensor<T>& C, const IndexArray& indicesC,
  const std::vector<Unsigned>& blkSizes, bool isStatC,
        BlkHadamardStatCInfo& hadamardInfo
) {
	TensorDistribution distA = A.TensorDist();
	TensorDistribution distB = B.TensorDist();
	TensorDistribution distC = C.TensorDist();

	TensorDistribution distIntA(distA.size() - 1); // isStatC
	TensorDistribution distIntB(distB.size() - 1);
	TensorDistribution distIntC(distC.size() - 1); // !isStatC

	//Setup temp dist A
	distIntA.SetToMatch(distC, indicesC, indicesA); // isStatC
	if (isStatC) {
		distIntB.SetToMatch(distC, indicesC, indicesB);
	} else {
		distIntB.SetToMatch(distA, indicesA, indicesB);
	}
	distIntC.SetToMatch(distA, indicesA, indicesC); // !isStatC

	Unsigned i;
	IndexArray indicesCA = isStatC
		? DiffVector(IsectVector(indicesC, indicesA), indicesB)
		: DiffVector(IsectVector(indicesA, indicesC), indicesB);
	IndexArray indicesCB = DiffVector(IsectVector(indicesC, indicesB), indicesA);
	IndexArray indicesCBA = isStatC
		? IsectVector(IsectVector(indicesC, indicesB), indicesA)
		: IsectVector(IsectVector(indicesA, indicesB), indicesC);

	// PrintVector(indicesCA, "indicesAC", true);
	// PrintVector(indicesCB, "indicesBC", true);
	// PrintVector(indicesCBA, "indicesABC", true);
	hadamardInfo.isStatC = isStatC;
	//Set the intermediate dists
	hadamardInfo.distIntB = distIntB;
	if (isStatC) {
		hadamardInfo.distIntA = distIntA;
	} else {
		hadamardInfo.distIntC = distIntC;
	}

	//Determine the modes to partition
	hadamardInfo.partModesACA.resize(indicesCA.size());
	hadamardInfo.partModesACC.resize(indicesCA.size());
	hadamardInfo.partModesBCB.resize(indicesCB.size());
	hadamardInfo.partModesBCC.resize(indicesCB.size());
	for(i = 0; i < indicesCA.size(); i++){
		hadamardInfo.partModesACA[i] = IndexOf(indicesA, indicesCA[i]);
		hadamardInfo.partModesACC[i] = IndexOf(indicesC, indicesCA[i]);
	}
	for(i = 0; i < indicesCB.size(); i++) {
		hadamardInfo.partModesBCB[i] = IndexOf(indicesB, indicesCB[i]);
		hadamardInfo.partModesBCC[i] = IndexOf(indicesC, indicesCB[i]);
	}

	//Determine the final alignments needed
	hadamardInfo.alignModesB.resize(indicesCB.size());
	hadamardInfo.alignModesBTo.resize(indicesCB.size());
	for(i = 0; i < indicesCB.size(); i++){
		hadamardInfo.alignModesB[i] = IndexOf(indicesB, indicesCB[i]);
		hadamardInfo.alignModesBTo[i] = IndexOf(indicesC, indicesCB[i]);
	}

	if (isStatC) {
		hadamardInfo.alignModesA.resize(indicesCA.size());
		hadamardInfo.alignModesATo.resize(indicesCA.size());
		for(i = 0; i < indicesCA.size(); i++){
			hadamardInfo.alignModesA[i] = IndexOf(indicesA, indicesCA[i]);
			hadamardInfo.alignModesATo[i] = IndexOf(indicesC, indicesCA[i]);
		}
	}
	else {
		hadamardInfo.alignModesC.resize(indicesCA.size());
		hadamardInfo.alignModesCTo.resize(indicesCA.size());
		for(i = 0; i < indicesCA.size(); i++){
			hadamardInfo.alignModesC[i] = IndexOf(indicesC, indicesCA[i]);
			hadamardInfo.alignModesCTo[i] = IndexOf(indicesA, indicesCA[i]);
		}
	}

	//Set the Block-size info
	//NOTE: There are better ways to do this
	if(blkSizes.size() == 0){
		hadamardInfo.blkSizes.resize(indicesCA.size() + indicesCB.size());
		for(i = 0; i < indicesCA.size() + indicesCB.size(); i++)
			hadamardInfo.blkSizes[i] = 1;
	}else{
		hadamardInfo.blkSizes = blkSizes;
	}

	//Set the local permutation info
	Permutation permA(indicesA, ConcatenateVectors(indicesCA, indicesCBA));
	Permutation permB(indicesB, ConcatenateVectors(indicesCBA, indicesCB));
	Permutation permC(indicesC, ConcatenateVectors(ConcatenateVectors(indicesCBA, indicesCB), indicesCA));

	hadamardInfo.permA = permA;
	hadamardInfo.permB = permB;
	hadamardInfo.permC = permC;
}

#define PROTO(T) \
	template class Hadamard<T>;

//PROTO(Unsigned)
//PROTO(Int)
PROTO(float)
PROTO(double)
//PROTO(char)

#ifndef DISABLE_COMPLEX
#ifndef DISABLE_FLOAT
PROTO(std::complex<float>)
#endif
PROTO(std::complex<double>)
#endif

} // namespace rote
