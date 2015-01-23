/*
 This file is part of DxTer.
 DxTer is a prototype using the Design by Transformation (DxT)
 approach to program generation.

 Copyright (C) 2014, The University of Texas and Bryan Marker

 DxTer is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 DxTer is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with DxTer.  If not, see <http://www.gnu.org/licenses/>.
 */
// NOTE: It is possible to simply include "tensormental.hpp" instead
#include "tensormental.hpp"

#ifdef PROFILE
#ifdef BGQ
#include <spi/include/kernel/memory.h>
#endif
#endif

using namespace tmen;
using namespace std;

#define GRIDORDER 4

template<typename T>
void PrintLocalSizes(const DistTensor<T>& A) {
    const Int commRank = mpi::CommRank(mpi::COMM_WORLD);
    if (commRank == 0) {
        for (Unsigned i = 0; i < A.Order(); ++i) {
            cout << i << " is " << A.LocalDimension(i) << endl;
        }
    }
}

void Usage() {
    std::cout << "./DistTensor <gridDim0> <gridDim1> ... \n";
    std::cout << "<gridDimK>   : dimension of mode-K of grid\n";
}

typedef struct Arguments {
    ObjShape gridShape;
    Unsigned nProcs;
    Unsigned n_o;
    Unsigned n_v;
    Unsigned blkSize;
    Unsigned testIter;
} Params;

void ProcessInput(int argc, char** const argv, Params& args) {
    Unsigned i;
    Unsigned argCount = 0;
    if (argCount + 1 >= argc) {
        std::cerr << "Missing required gridOrder argument\n";
        Usage();
        throw ArgException();
    }

    if (argCount + GRIDORDER >= argc) {
        std::cerr << "Missing required grid dimensions\n";
        Usage();
        throw ArgException();
    }

    args.gridShape.resize(GRIDORDER);
    args.nProcs = 1;
    for (int i = 0; i < GRIDORDER; i++) {
        int gridDim = atoi(argv[++argCount]);
        if (gridDim <= 0) {
            std::cerr << "Grid dim must be greater than 0\n";
            Usage();
            throw ArgException();
        }
        args.nProcs *= gridDim;
        args.gridShape[i] = gridDim;
    }

    args.n_o = atoi(argv[++argCount]);
    args.n_v = atoi(argv[++argCount]);
    args.blkSize = atoi(argv[++argCount]);
    args.testIter = atoi(argv[++argCount]);
}

template<typename T>
void DistTensorTest(const Grid& g, Unsigned n_o, Unsigned n_v,
        Unsigned blkSize, Unsigned testIter) {
#ifndef RELEASE
    CallStackEntry entry("DistTensorTest");
#endif
    Unsigned i;
    const Int commRank = mpi::CommRank(mpi::COMM_WORLD);

ObjShape H_tempShape;
TensorDistribution dist__S__S = tmen::StringToTensorDist("[(),()]");
TensorDistribution dist__D_0__D_1__D_2__D_3 = tmen::StringToTensorDist("[(0),(1),(2),(3)]");
TensorDistribution dist__D_0__D_1__D_3__D_2 = tmen::StringToTensorDist("[(0),(1),(3),(2)]");
TensorDistribution dist__D_2_3__S__D_0_1__S = tmen::StringToTensorDist("[(2,3),(),(0,1),()]");
TensorDistribution dist__D_2_3__D_1__D_0__S = tmen::StringToTensorDist("[(2,3),(1),(0),()]");
TensorDistribution dist__D_2__D_1__D_0__D_3 = tmen::StringToTensorDist("[(2),(1),(0),(3)]");
TensorDistribution dist__D_0_1__D_2_3 = tmen::StringToTensorDist("[(0,1),(2,3)]");
Permutation perm_0_1( 2 );
perm_0_1[0] = 0;
perm_0_1[1] = 1;
Permutation perm_0_1_2_3( 4 );
perm_0_1_2_3[0] = 0;
perm_0_1_2_3[1] = 1;
perm_0_1_2_3[2] = 2;
perm_0_1_2_3[3] = 3;
Permutation perm_0_1_3_2( 4 );
perm_0_1_3_2[0] = 0;
perm_0_1_3_2[1] = 1;
perm_0_1_3_2[2] = 3;
perm_0_1_3_2[3] = 2;
Permutation perm_0_2_1_3( 4 );
perm_0_2_1_3[0] = 0;
perm_0_2_1_3[1] = 2;
perm_0_2_1_3[2] = 1;
perm_0_2_1_3[3] = 3;
Permutation perm_1_0( 2 );
perm_1_0[0] = 1;
perm_1_0[1] = 0;
ModeArray modes( 0 );
ModeArray modes_0_1_2_3( 4 );
modes_0_1_2_3[0] = 0;
modes_0_1_2_3[1] = 1;
modes_0_1_2_3[2] = 2;
modes_0_1_2_3[3] = 3;
ModeArray modes_0_1_3_2( 4 );
modes_0_1_3_2[0] = 0;
modes_0_1_3_2[1] = 1;
modes_0_1_3_2[2] = 3;
modes_0_1_3_2[3] = 2;
ModeArray modes_0_2( 2 );
modes_0_2[0] = 0;
modes_0_2[1] = 2;
ModeArray modes_1( 1 );
modes_1[0] = 1;
ModeArray modes_1_0( 2 );
modes_1_0[0] = 1;
modes_1_0[1] = 0;
ModeArray modes_2_3( 2 );
modes_2_3[0] = 2;
modes_2_3[1] = 3;
ModeArray modes_3( 1 );
modes_3[0] = 3;
IndexArray indices_em( 2 );
indices_em[0] = 'e';
indices_em[1] = 'm';
IndexArray indices_emfn( 4 );
indices_emfn[0] = 'e';
indices_emfn[1] = 'm';
indices_emfn[2] = 'f';
indices_emfn[3] = 'n';
IndexArray indices_fn( 2 );
indices_fn[0] = 'f';
indices_fn[1] = 'n';
	//H_me[D01,D23]
DistTensor<double> H_me__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//H_me_part0B[D01,D23]
DistTensor<double> H_me_part0B__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//H_me_part0T[D01,D23]
DistTensor<double> H_me_part0T__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//H_me_part0_0[D01,D23]
DistTensor<double> H_me_part0_0__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//H_me_part0_1[D01,D23]
DistTensor<double> H_me_part0_1__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
DistTensor<double> H_me_part0_1_perm10__D_2_3__D_0_1( dist__D_0_1__D_2_3, g );
H_me_part0_1_perm10__D_2_3__D_0_1.SetLocalPermutation( perm_1_0 );
	//H_me_part0_2[D01,D23]
DistTensor<double> H_me_part0_2__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//t_fj[D01,D23]
DistTensor<double> t_fj__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//t_fj_part1B[D01,D23]
DistTensor<double> t_fj_part1B__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//t_fj_part1T[D01,D23]
DistTensor<double> t_fj_part1T__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//t_fj_part1_0[D01,D23]
DistTensor<double> t_fj_part1_0__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//t_fj_part1_1[D01,D23]
DistTensor<double> t_fj_part1_1__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//t_fj_part1_1[*,*]
DistTensor<double> t_fj_part1_1__S__S( dist__S__S, g );
	//t_fj_part1_2[D01,D23]
DistTensor<double> t_fj_part1_2__D_0_1__D_2_3( dist__D_0_1__D_2_3, g );
	//H_temp1[D0,D1,D2,D3]
DistTensor<double> H_temp1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2B[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2B__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2T[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2_0[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2_0__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2_1[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2_1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2_1_part3B[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2_1_part3B__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2_1_part3T[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2_1_part3T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2_1_part3_0[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2_1_part3_0__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2_1_part3_1[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2_1_part3_1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2_1_part3_2[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2_1_part3_2__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part2_2[D0,D1,D2,D3]
DistTensor<double> H_temp1_part2_2__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3B[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3B__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3T[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3_0[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3_0__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3_1[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3_1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3_1_part2B[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3_1_part2B__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3_1_part2T[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3_1_part2T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3_1_part2_0[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3_1_part2_0__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3_1_part2_1[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3_1_part2_1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3_1_part2_1[D23,D1,D0,*]
DistTensor<double> H_temp1_part3_1_part2_1__D_2_3__D_1__D_0__S( dist__D_2_3__D_1__D_0__S, g );
	//H_temp1_part3_1_part2_1[D2,D1,D0,D3]
DistTensor<double> H_temp1_part3_1_part2_1__D_2__D_1__D_0__D_3( dist__D_2__D_1__D_0__D_3, g );
	//H_temp1_part3_1_part2_1[D23,*,D01,*]
DistTensor<double> H_temp1_part3_1_part2_1_perm0213__D_2_3__D_0_1__S__S( dist__D_2_3__S__D_0_1__S, g );
H_temp1_part3_1_part2_1_perm0213__D_2_3__D_0_1__S__S.SetLocalPermutation( perm_0_2_1_3 );
	//H_temp1_part3_1_part2_2[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3_1_part2_2__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//H_temp1_part3_2[D0,D1,D2,D3]
DistTensor<double> H_temp1_part3_2__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn[D0,D1,D2,D3]
DistTensor<double> v_femn__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2B[D0,D1,D2,D3]
DistTensor<double> v_femn_part2B__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2T[D0,D1,D2,D3]
DistTensor<double> v_femn_part2T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2_0[D0,D1,D2,D3]
DistTensor<double> v_femn_part2_0__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2_1[D0,D1,D2,D3]
DistTensor<double> v_femn_part2_1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2_1_part3B[D0,D1,D2,D3]
DistTensor<double> v_femn_part2_1_part3B__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2_1_part3T[D0,D1,D2,D3]
DistTensor<double> v_femn_part2_1_part3T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2_1_part3_0[D0,D1,D2,D3]
DistTensor<double> v_femn_part2_1_part3_0__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2_1_part3_1[D0,D1,D2,D3]
DistTensor<double> v_femn_part2_1_part3_1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2_1_part3_2[D0,D1,D2,D3]
DistTensor<double> v_femn_part2_1_part3_2__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part2_2[D0,D1,D2,D3]
DistTensor<double> v_femn_part2_2__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3B[D0,D1,D2,D3]
DistTensor<double> v_femn_part3B__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3T[D0,D1,D2,D3]
DistTensor<double> v_femn_part3T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3_0[D0,D1,D2,D3]
DistTensor<double> v_femn_part3_0__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3_1[D0,D1,D2,D3]
DistTensor<double> v_femn_part3_1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3_1_part2B[D0,D1,D2,D3]
DistTensor<double> v_femn_part3_1_part2B__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3_1_part2T[D0,D1,D2,D3]
DistTensor<double> v_femn_part3_1_part2T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3_1_part2_0[D0,D1,D2,D3]
DistTensor<double> v_femn_part3_1_part2_0__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3_1_part2_1[D0,D1,D2,D3]
DistTensor<double> v_femn_part3_1_part2_1__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3_1_part2_1[D0,D1,D3,D2]
DistTensor<double> v_femn_part3_1_part2_1__D_0__D_1__D_3__D_2( dist__D_0__D_1__D_3__D_2, g );
	//v_femn_part3_1_part2_2[D0,D1,D2,D3]
DistTensor<double> v_femn_part3_1_part2_2__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
	//v_femn_part3_2[D0,D1,D2,D3]
DistTensor<double> v_femn_part3_2__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
// v_femn has 4 dims
//	Starting distribution: [D0,D1,D2,D3] or _D_0__D_1__D_2__D_3
ObjShape v_femn__D_0__D_1__D_2__D_3_H_tempShape( 4 );
v_femn__D_0__D_1__D_2__D_3_H_tempShape[ 0 ] = n_v;
v_femn__D_0__D_1__D_2__D_3_H_tempShape[ 1 ] = n_v;
v_femn__D_0__D_1__D_2__D_3_H_tempShape[ 2 ] = n_o;
v_femn__D_0__D_1__D_2__D_3_H_tempShape[ 3 ] = n_o;
v_femn__D_0__D_1__D_2__D_3.ResizeTo( v_femn__D_0__D_1__D_2__D_3_H_tempShape );
MakeUniform( v_femn__D_0__D_1__D_2__D_3 );
H_tempShape = v_femn__D_0__D_1__D_2__D_3.Shape();
H_temp1__D_0__D_1__D_2__D_3.ResizeTo( H_tempShape );
// t_fj has 2 dims
//	Starting distribution: [D01,D23] or _D_0_1__D_2_3
ObjShape t_fj__D_0_1__D_2_3_H_tempShape( 2 );
t_fj__D_0_1__D_2_3_H_tempShape[ 0 ] = n_v;
t_fj__D_0_1__D_2_3_H_tempShape[ 1 ] = n_o;
t_fj__D_0_1__D_2_3.ResizeTo( t_fj__D_0_1__D_2_3_H_tempShape );
MakeUniform( t_fj__D_0_1__D_2_3 );
// H_me has 2 dims
ObjShape H_me__D_0_1__D_2_3_H_tempShape( 2 );
H_me__D_0_1__D_2_3_H_tempShape[ 0 ] = n_o;
H_me__D_0_1__D_2_3_H_tempShape[ 1 ] = n_v;
H_me__D_0_1__D_2_3.ResizeTo( H_me__D_0_1__D_2_3_H_tempShape );
MakeUniform( H_me__D_0_1__D_2_3 );
//**** (out of 1)

//******************************
//* Load tensors
//******************************
////////////////////////////////
//Performance testing
////////////////////////////////
std::stringstream fullName;
#ifdef CORRECTNESS
DistTensor<T> check_H(dist__D_0_1__D_2_3, g);
check_H.ResizeTo(H_me__D_0_1__D_2_3.Shape());
fullName.str("");
fullName.clear();
fullName << "ccsd_terms/term_H_iter" << testIter;
Read(check_H, fullName.str(), BINARY_FLAT, false);
#endif
//******************************
//* Load tensors
//******************************
    double gflops;
    double startTime;
    double runTime;
    double norm = 1.0;
    if(commRank == 0){
        std::cout << "starting\n";
#ifdef PROFILE
#ifdef BGQ
        uint64_t heap, heapavail;
        Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAP, &heap);
        Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAPAVAIL, &heapavail);
        printf("Allocated heap: %.2f MB, avail. heap: %.2f MB\n", (double)heap/(1024*1024),(double)heapavail/(1024*1024));
#endif
#endif
    }
    mpi::Barrier(g.OwningComm());
    startTime = mpi::Time();


PROFILE_SECTION("COMPUTE");
PROFILE_FLOPS(prod(H_me__D_0_1__D_2_3.Shape()));
	Scal( 0.0, H_me__D_0_1__D_2_3 );
PROFILE_STOP;
	//**** (out of 1)
	//**** Is real	0 shadows
		//Outputs:
		//  H_temp1__D_0__D_1__D_2__D_3
	PartitionDown(v_femn__D_0__D_1__D_2__D_3, v_femn_part2T__D_0__D_1__D_2__D_3, v_femn_part2B__D_0__D_1__D_2__D_3, 2, 0);
	PartitionDown(v_femn__D_0__D_1__D_2__D_3, v_femn_part3T__D_0__D_1__D_2__D_3, v_femn_part3B__D_0__D_1__D_2__D_3, 3, 0);
	PartitionDown(H_temp1__D_0__D_1__D_2__D_3, H_temp1_part2T__D_0__D_1__D_2__D_3, H_temp1_part2B__D_0__D_1__D_2__D_3, 2, 0);
	while(H_temp1_part2T__D_0__D_1__D_2__D_3.Dimension(2) < H_temp1__D_0__D_1__D_2__D_3.Dimension(2))
	{
		RepartitionDown
		( v_femn_part2T__D_0__D_1__D_2__D_3,  v_femn_part2_0__D_0__D_1__D_2__D_3,
		  /**/ /**/
		       v_femn_part2_1__D_0__D_1__D_2__D_3,
		  v_femn_part2B__D_0__D_1__D_2__D_3, v_femn_part2_2__D_0__D_1__D_2__D_3, 2, blkSize );
		RepartitionDown
		( v_femn_part3T__D_0__D_1__D_2__D_3,  v_femn_part3_0__D_0__D_1__D_2__D_3,
		  /**/ /**/
		       v_femn_part3_1__D_0__D_1__D_2__D_3,
		  v_femn_part3B__D_0__D_1__D_2__D_3, v_femn_part3_2__D_0__D_1__D_2__D_3, 3, blkSize );
		RepartitionDown
		( H_temp1_part2T__D_0__D_1__D_2__D_3,  H_temp1_part2_0__D_0__D_1__D_2__D_3,
		  /**/ /**/
		       H_temp1_part2_1__D_0__D_1__D_2__D_3,
		  H_temp1_part2B__D_0__D_1__D_2__D_3, H_temp1_part2_2__D_0__D_1__D_2__D_3, 2, blkSize );

		//**** (out of 1)
		//**** Is real	0 shadows
			//Outputs:
			//  H_temp1_part2_1__D_0__D_1__D_2__D_3
		PartitionDown(v_femn_part2_1__D_0__D_1__D_2__D_3, v_femn_part2_1_part3T__D_0__D_1__D_2__D_3, v_femn_part2_1_part3B__D_0__D_1__D_2__D_3, 3, 0);
		PartitionDown(v_femn_part3_1__D_0__D_1__D_2__D_3, v_femn_part3_1_part2T__D_0__D_1__D_2__D_3, v_femn_part3_1_part2B__D_0__D_1__D_2__D_3, 2, 0);
		PartitionDown(H_temp1_part2_1__D_0__D_1__D_2__D_3, H_temp1_part2_1_part3T__D_0__D_1__D_2__D_3, H_temp1_part2_1_part3B__D_0__D_1__D_2__D_3, 3, 0);
		while(H_temp1_part2_1_part3T__D_0__D_1__D_2__D_3.Dimension(3) < H_temp1_part2_1__D_0__D_1__D_2__D_3.Dimension(3))
		{
			RepartitionDown
			( v_femn_part2_1_part3T__D_0__D_1__D_2__D_3,  v_femn_part2_1_part3_0__D_0__D_1__D_2__D_3,
			  /**/ /**/
			       v_femn_part2_1_part3_1__D_0__D_1__D_2__D_3,
			  v_femn_part2_1_part3B__D_0__D_1__D_2__D_3, v_femn_part2_1_part3_2__D_0__D_1__D_2__D_3, 3, blkSize );
			RepartitionDown
			( v_femn_part3_1_part2T__D_0__D_1__D_2__D_3,  v_femn_part3_1_part2_0__D_0__D_1__D_2__D_3,
			  /**/ /**/
			       v_femn_part3_1_part2_1__D_0__D_1__D_2__D_3,
			  v_femn_part3_1_part2B__D_0__D_1__D_2__D_3, v_femn_part3_1_part2_2__D_0__D_1__D_2__D_3, 2, blkSize );
			RepartitionDown
			( H_temp1_part2_1_part3T__D_0__D_1__D_2__D_3,  H_temp1_part2_1_part3_0__D_0__D_1__D_2__D_3,
			  /**/ /**/
			       H_temp1_part2_1_part3_1__D_0__D_1__D_2__D_3,
			  H_temp1_part2_1_part3B__D_0__D_1__D_2__D_3, H_temp1_part2_1_part3_2__D_0__D_1__D_2__D_3, 3, blkSize );

			   // v_femn_part3_1_part2_1[D0,D1,D3,D2] <- v_femn_part3_1_part2_1[D0,D1,D2,D3]
			v_femn_part3_1_part2_1__D_0__D_1__D_3__D_2.AlignModesWith( modes_0_1_2_3, v_femn_part2_1_part3_1__D_0__D_1__D_2__D_3, modes_0_1_3_2 );
			v_femn_part3_1_part2_1__D_0__D_1__D_3__D_2.AllToAllRedistFrom( v_femn_part3_1_part2_1__D_0__D_1__D_2__D_3, modes_2_3 );
PROFILE_SECTION("COMPUTE");
PROFILE_FLOPS(3*prod(v_femn_part2_1_part3_1__D_0__D_1__D_2__D_3.Shape()));
			YAxpPx( 2.0, v_femn_part2_1_part3_1__D_0__D_1__D_2__D_3, -1.0, v_femn_part3_1_part2_1__D_0__D_1__D_3__D_2, perm_0_1_3_2, H_temp1_part2_1_part3_1__D_0__D_1__D_2__D_3 );
PROFILE_STOP;
			v_femn_part3_1_part2_1__D_0__D_1__D_3__D_2.EmptyData();

			SlidePartitionDown
			( v_femn_part2_1_part3T__D_0__D_1__D_2__D_3,  v_femn_part2_1_part3_0__D_0__D_1__D_2__D_3,
			       v_femn_part2_1_part3_1__D_0__D_1__D_2__D_3,
			  /**/ /**/
			  v_femn_part2_1_part3B__D_0__D_1__D_2__D_3, v_femn_part2_1_part3_2__D_0__D_1__D_2__D_3, 3 );
			SlidePartitionDown
			( v_femn_part3_1_part2T__D_0__D_1__D_2__D_3,  v_femn_part3_1_part2_0__D_0__D_1__D_2__D_3,
			       v_femn_part3_1_part2_1__D_0__D_1__D_2__D_3,
			  /**/ /**/
			  v_femn_part3_1_part2B__D_0__D_1__D_2__D_3, v_femn_part3_1_part2_2__D_0__D_1__D_2__D_3, 2 );
			SlidePartitionDown
			( H_temp1_part2_1_part3T__D_0__D_1__D_2__D_3,  H_temp1_part2_1_part3_0__D_0__D_1__D_2__D_3,
			       H_temp1_part2_1_part3_1__D_0__D_1__D_2__D_3,
			  /**/ /**/
			  H_temp1_part2_1_part3B__D_0__D_1__D_2__D_3, H_temp1_part2_1_part3_2__D_0__D_1__D_2__D_3, 3 );

		}
		//****

		SlidePartitionDown
		( v_femn_part2T__D_0__D_1__D_2__D_3,  v_femn_part2_0__D_0__D_1__D_2__D_3,
		       v_femn_part2_1__D_0__D_1__D_2__D_3,
		  /**/ /**/
		  v_femn_part2B__D_0__D_1__D_2__D_3, v_femn_part2_2__D_0__D_1__D_2__D_3, 2 );
		SlidePartitionDown
		( v_femn_part3T__D_0__D_1__D_2__D_3,  v_femn_part3_0__D_0__D_1__D_2__D_3,
		       v_femn_part3_1__D_0__D_1__D_2__D_3,
		  /**/ /**/
		  v_femn_part3B__D_0__D_1__D_2__D_3, v_femn_part3_2__D_0__D_1__D_2__D_3, 3 );
		SlidePartitionDown
		( H_temp1_part2T__D_0__D_1__D_2__D_3,  H_temp1_part2_0__D_0__D_1__D_2__D_3,
		       H_temp1_part2_1__D_0__D_1__D_2__D_3,
		  /**/ /**/
		  H_temp1_part2B__D_0__D_1__D_2__D_3, H_temp1_part2_2__D_0__D_1__D_2__D_3, 2 );

	}
	//****
	//**** (out of 1)
	//**** Is real	0 shadows
		//Outputs:
		//  H_me__D_0_1__D_2_3
	PartitionDown(H_temp1__D_0__D_1__D_2__D_3, H_temp1_part3T__D_0__D_1__D_2__D_3, H_temp1_part3B__D_0__D_1__D_2__D_3, 3, 0);
	PartitionDown(t_fj__D_0_1__D_2_3, t_fj_part1T__D_0_1__D_2_3, t_fj_part1B__D_0_1__D_2_3, 1, 0);
	while(H_temp1_part3T__D_0__D_1__D_2__D_3.Dimension(3) < H_temp1__D_0__D_1__D_2__D_3.Dimension(3))
	{
		RepartitionDown
		( H_temp1_part3T__D_0__D_1__D_2__D_3,  H_temp1_part3_0__D_0__D_1__D_2__D_3,
		  /**/ /**/
		       H_temp1_part3_1__D_0__D_1__D_2__D_3,
		  H_temp1_part3B__D_0__D_1__D_2__D_3, H_temp1_part3_2__D_0__D_1__D_2__D_3, 3, blkSize );
		RepartitionDown
		( t_fj_part1T__D_0_1__D_2_3,  t_fj_part1_0__D_0_1__D_2_3,
		  /**/ /**/
		       t_fj_part1_1__D_0_1__D_2_3,
		  t_fj_part1B__D_0_1__D_2_3, t_fj_part1_2__D_0_1__D_2_3, 1, blkSize );

		//**** (out of 1)
		//**** Is real	0 shadows
			//Outputs:
			//  H_me__D_0_1__D_2_3
		PartitionDown(H_temp1_part3_1__D_0__D_1__D_2__D_3, H_temp1_part3_1_part2T__D_0__D_1__D_2__D_3, H_temp1_part3_1_part2B__D_0__D_1__D_2__D_3, 2, 0);
		PartitionDown(H_me__D_0_1__D_2_3, H_me_part0T__D_0_1__D_2_3, H_me_part0B__D_0_1__D_2_3, 0, 0);
		while(H_me_part0T__D_0_1__D_2_3.Dimension(0) < H_me__D_0_1__D_2_3.Dimension(0))
		{
			RepartitionDown
			( H_temp1_part3_1_part2T__D_0__D_1__D_2__D_3,  H_temp1_part3_1_part2_0__D_0__D_1__D_2__D_3,
			  /**/ /**/
			       H_temp1_part3_1_part2_1__D_0__D_1__D_2__D_3,
			  H_temp1_part3_1_part2B__D_0__D_1__D_2__D_3, H_temp1_part3_1_part2_2__D_0__D_1__D_2__D_3, 2, blkSize );
			RepartitionDown
			( H_me_part0T__D_0_1__D_2_3,  H_me_part0_0__D_0_1__D_2_3,
			  /**/ /**/
			       H_me_part0_1__D_0_1__D_2_3,
			  H_me_part0B__D_0_1__D_2_3, H_me_part0_2__D_0_1__D_2_3, 0, blkSize );

			   // H_temp1_part3_1_part2_1[D2,D1,D0,D3] <- H_temp1_part3_1_part2_1[D0,D1,D2,D3]
			H_temp1_part3_1_part2_1__D_2__D_1__D_0__D_3.AlignModesWith( modes_0_2, H_me_part0_1__D_0_1__D_2_3, modes_1_0 );
			H_temp1_part3_1_part2_1__D_2__D_1__D_0__D_3.AllToAllRedistFrom( H_temp1_part3_1_part2_1__D_0__D_1__D_2__D_3, modes_0_2 );
			   // H_temp1_part3_1_part2_1[D23,D1,D0,*] <- H_temp1_part3_1_part2_1[D2,D1,D0,D3]
			H_temp1_part3_1_part2_1__D_2_3__D_1__D_0__S.AlignModesWith( modes_0_2, H_me_part0_1__D_0_1__D_2_3, modes_1_0 );
			H_temp1_part3_1_part2_1__D_2_3__D_1__D_0__S.AllToAllRedistFrom( H_temp1_part3_1_part2_1__D_2__D_1__D_0__D_3, modes_3 );
			H_temp1_part3_1_part2_1__D_2__D_1__D_0__D_3.EmptyData();
			   // t_fj_part1_1[*,*] <- t_fj_part1_1[D01,D23]
			t_fj_part1_1__S__S.AllGatherRedistFrom( t_fj_part1_1__D_0_1__D_2_3, modes_0_1_2_3 );
			   // H_temp1_part3_1_part2_1[D23,*,D01,*] <- H_temp1_part3_1_part2_1[D23,D1,D0,*]
			H_temp1_part3_1_part2_1_perm0213__D_2_3__D_0_1__S__S.AlignModesWith( modes_0_2, H_me_part0_1__D_0_1__D_2_3, modes_1_0 );
			H_temp1_part3_1_part2_1_perm0213__D_2_3__D_0_1__S__S.AllToAllRedistFrom( H_temp1_part3_1_part2_1__D_2_3__D_1__D_0__S, modes_1 );
			H_temp1_part3_1_part2_1__D_2_3__D_1__D_0__S.EmptyData();
			Permute( H_me_part0_1__D_0_1__D_2_3, H_me_part0_1_perm10__D_2_3__D_0_1 );
			   // 1.0 * H_temp1_part3_1_part2_1[D23,*,D01,*]_emfn * t_fj_part1_1[*,*]_fn + 1.0 * H_me_part0_1[D01,D23]_em
PROFILE_SECTION("COMPUTE");
PROFILE_FLOPS(prod(H_me_part0_1_perm10__D_2_3__D_0_1.Shape())*H_temp1_part3_1_part2_1_perm0213__D_2_3__D_0_1__S__S.Dimension(1)*H_temp1_part3_1_part2_1_perm0213__D_2_3__D_0_1__S__S.Dimension(3));
			LocalContractAndLocalEliminate(1.0, H_temp1_part3_1_part2_1_perm0213__D_2_3__D_0_1__S__S.LockedTensor(), indices_emfn, false,
				t_fj_part1_1__S__S.LockedTensor(), indices_fn, false,
				1.0, H_me_part0_1_perm10__D_2_3__D_0_1.Tensor(), indices_em, false);
PROFILE_STOP;
			t_fj_part1_1__S__S.EmptyData();
			H_temp1_part3_1_part2_1_perm0213__D_2_3__D_0_1__S__S.EmptyData();
			Permute( H_me_part0_1_perm10__D_2_3__D_0_1, H_me_part0_1__D_0_1__D_2_3 );
			H_me_part0_1_perm10__D_2_3__D_0_1.EmptyData();

			SlidePartitionDown
			( H_temp1_part3_1_part2T__D_0__D_1__D_2__D_3,  H_temp1_part3_1_part2_0__D_0__D_1__D_2__D_3,
			       H_temp1_part3_1_part2_1__D_0__D_1__D_2__D_3,
			  /**/ /**/
			  H_temp1_part3_1_part2B__D_0__D_1__D_2__D_3, H_temp1_part3_1_part2_2__D_0__D_1__D_2__D_3, 2 );
			SlidePartitionDown
			( H_me_part0T__D_0_1__D_2_3,  H_me_part0_0__D_0_1__D_2_3,
			       H_me_part0_1__D_0_1__D_2_3,
			  /**/ /**/
			  H_me_part0B__D_0_1__D_2_3, H_me_part0_2__D_0_1__D_2_3, 0 );

		}
		//****

		SlidePartitionDown
		( H_temp1_part3T__D_0__D_1__D_2__D_3,  H_temp1_part3_0__D_0__D_1__D_2__D_3,
		       H_temp1_part3_1__D_0__D_1__D_2__D_3,
		  /**/ /**/
		  H_temp1_part3B__D_0__D_1__D_2__D_3, H_temp1_part3_2__D_0__D_1__D_2__D_3, 3 );
		SlidePartitionDown
		( t_fj_part1T__D_0_1__D_2_3,  t_fj_part1_0__D_0_1__D_2_3,
		       t_fj_part1_1__D_0_1__D_2_3,
		  /**/ /**/
		  t_fj_part1B__D_0_1__D_2_3, t_fj_part1_2__D_0_1__D_2_3, 1 );

	}
	H_temp1__D_0__D_1__D_2__D_3.EmptyData();
	H_temp1__D_0__D_1__D_2__D_3.EmptyData();
	//****


H_temp1__D_0__D_1__D_2__D_3.EmptyData();
//****



    /*****************************************/
    mpi::Barrier(g.OwningComm());
    runTime = mpi::Time() - startTime;
    long long flops = Timer::nflops("COMPUTE");
    gflops = flops / (1e9 * runTime);
#ifdef CORRECTNESS
    DistTensor<double> diff_H(dist__D_0_1__D_2_3, g);
    diff_H.ResizeTo(check_H);
    Diff(check_H, H_me__D_0_1__D_2_3, diff_H);
   norm = 1.0;
   norm = Norm(diff_H);
   if (commRank == 0){
     std::cout << "NORM_H " << norm << std::endl;
   }
#endif

    //****

    //------------------------------------//

    //****

    if (commRank == 0)
        Timer::printTimers();

    //****
    if (commRank == 0) {
        std::cout << "RUNTIME " << runTime << std::endl;
        std::cout << "FLOPS " << flops << std::endl;
        std::cout << "GFLOPS " << gflops << std::endl;
    }
}

int main(int argc, char* argv[]) {
    Initialize(argc, argv);
    Unsigned i;
    mpi::Comm comm = mpi::COMM_WORLD;
    const Int commRank = mpi::CommRank(comm);
    const Int commSize = mpi::CommSize(comm);
    //    printf("My Rank: %d\n", commRank);
    try {
        Params args;

        ProcessInput(argc, argv, args);

        if (commRank == 0 && commSize != args.nProcs) {
            std::cerr
                    << "program not started with correct number of processes\n";
            std::cerr << commSize << " vs " << args.nProcs << std::endl;
            Usage();
            throw ArgException();
        }

        //        if(commRank == 0){
        //            printf("Creating %d", args.gridShape[0]);
        //            for(i = 1; i < GRIDORDER; i++)
        //                printf(" x %d", args.gridShape[i]);
        //            printf(" grid\n");
        //        }

        const Grid g(comm, args.gridShape);
        DistTensorTest<double>(g, args.n_o, args.n_v, args.blkSize, args.testIter);

    } catch (std::exception& e) {
        ReportException(e);
    }

    Finalize();
    //printf("Completed\n");
    return 0;
}


