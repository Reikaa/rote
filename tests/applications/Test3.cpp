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
using namespace tmen;
using namespace std;

#define GRIDORDER 4

template <typename T>
void PrintLocalSizes(const DistTensor<T>& A) 
{
  const Int commRank = mpi::CommRank( mpi::COMM_WORLD );
  if (commRank == 0) {
    for (Unsigned i = 0; i < A.Order(); ++i) {
      cout << i << " is " << A.LocalDimension(i) << endl;
    }
  }
}

template <typename T>
void GatherAllModes(const DistTensor<T>& A, DistTensor<T>& B)
{
  DistTensor<T> *tmp = NULL;
  //  DistTensor<T> *tmp = new DistTensor<T>(A.TensorDist(), A.Grid());
  //  *tmp = A;
  
  const TensorDistribution dist = A.TensorDist();

  for (Unsigned mode = 0; mode < A.Order(); ++mode) {
    ModeDistribution modeDist = dist[mode];
    if (!(modeDist.empty())) {
      TensorDistribution newDist = (tmp ? tmp->TensorDist() : A.TensorDist());
      ModeDistribution newIgnoreDist = newDist[newDist.size() - 1];
      newIgnoreDist.insert(newIgnoreDist.end(), modeDist.begin(), modeDist.end());
      newDist[newDist.size() - 1] = newIgnoreDist;
      modeDist.clear();
      newDist[mode] = modeDist;
      DistTensor<T> *tmp2 = new DistTensor<T>(newDist, A.Grid());
      if (!tmp) {
	tmp2->GatherToOneRedistFrom(A, mode);
      }
      else {
	tmp2->GatherToOneRedistFrom(*tmp, mode);
	delete tmp;
      }
      tmp = tmp2;
    }
  }

  if (tmp) {
    
    if (TensorDistToString(B.TensorDist()) != TensorDistToString(tmp->TensorDist())) {
      cout << TensorDistToString(B.TensorDist()) << endl;
      cout << TensorDistToString(tmp->TensorDist()) << endl;
      throw;
    }

    B = *tmp;
    delete tmp;
  }
  else {
    B = A;
  }
}

void Usage(){
  std::cout << "./DistTensor <gridDim0> <gridDim1> ... \n";
  std::cout << "<gridDimK>   : dimension of mode-K of grid\n";
}

typedef struct Arguments{
  ObjShape gridShape;
  Unsigned nProcs;
} Params;

void ProcessInput(int argc,  char** const argv, Params& args){
  Unsigned i;
  Unsigned argCount = 0;
  if(argCount + 1 >= argc){
    std::cerr << "Missing required gridOrder argument\n";
    Usage();
    throw ArgException();
  }

  if(argCount + GRIDORDER >= argc){
    std::cerr << "Missing required grid dimensions\n";
    Usage();
    throw ArgException();
  }

  args.gridShape.resize(GRIDORDER);
  args.nProcs = 1;
  for(int i = 0; i < GRIDORDER; i++){
    int gridDim = atoi(argv[++argCount]);
    if(gridDim <= 0){
      std::cerr << "Grid dim must be greater than 0\n";
      Usage();
      throw ArgException();
    }
    args.nProcs *= gridDim;
    args.gridShape[i] = gridDim;
  }
}


template<typename T>
void
DistTensorTest( const Grid& g )
{
#ifndef RELEASE
  CallStackEntry entry("DistTensorTest");
#endif
  Unsigned i;
  const Int commRank = mpi::CommRank( mpi::COMM_WORLD );
  const Unsigned gridOrder = 4;
  ObjShape tempShape;
  TensorDistribution dist____N_D_0_1_2_3 = tmen::StringToTensorDist("[]|(0,1,2,3)");
  TensorDistribution dist__S__S__D_2__D_3 = tmen::StringToTensorDist("[(),(),(2),(3)]");
  TensorDistribution dist__S__D_1__S__D_3 = tmen::StringToTensorDist("[(),(1),(),(3)]");
  TensorDistribution dist__S__D_1__D_2__D_3 = tmen::StringToTensorDist("[(),(1),(2),(3)]");
  TensorDistribution dist__D_0__S__D_2__S__D_1__D_3 = tmen::StringToTensorDist("[(0),(),(2),(),(1),(3)]");
  TensorDistribution dist__D_0__D_1__S__S__D_2__D_3 = tmen::StringToTensorDist("[(0),(1),(),(),(2),(3)]");
  TensorDistribution dist__D_0__D_1__S__D_3 = tmen::StringToTensorDist("[(0),(1),(),(3)]");
  TensorDistribution dist__D_0__D_1__D_2__S__D_3 = tmen::StringToTensorDist("[(0),(1),(2),(),(3)]");
  TensorDistribution dist__D_0__D_1__D_2__D_3 = tmen::StringToTensorDist("[(0),(1),(2),(3)]");
  TensorDistribution dist__D_2_0__D_1__S__D_3 = tmen::StringToTensorDist("[(2,0),(1),(),(3)]");
  TensorDistribution dist__D_1__D_2__D_3__N_D_0 = tmen::StringToTensorDist("[(1),(2),(3)]|(0)");
  TensorDistribution dist__D_2__D_1__S__D_3 = tmen::StringToTensorDist("[(2),(1),(),(3)]");
  TensorDistribution dist__D_2__D_3__S__S = tmen::StringToTensorDist("[(2),(3),(),()]");
  TensorDistribution dist__D_2__D_3__S__D_1 = tmen::StringToTensorDist("[(2),(3),(),(1)]");
  TensorDistribution dist__D_2__D_3__N_D_0_1 = tmen::StringToTensorDist("[(2),(3)]|(0,1)");
  TensorDistribution dist__D_3__N_D_0_1_2 = tmen::StringToTensorDist("[(3)]|(0,1,2)");
  TensorDistribution dist__D_0_2__D_1__S__D_3 = tmen::StringToTensorDist("[(0,2),(1),(),(3)]");
  //Accum[D0,D1,D2,D3]
  DistTensor<double> Accum__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
  //Accum[D0,D1,D2,*,D3]
  DistTensor<double> Accum__D_0__D_1__D_2__S__D_3( dist__D_0__D_1__D_2__S__D_3, g );
  //Accum[D0,D1,*,*,D2,D3]
  DistTensor<double> Accum__D_0__D_1__S__S__D_2__D_3( dist__D_0__D_1__S__S__D_2__D_3, g );
  //Accum[D0,*,D2,*,D1,D3]
  DistTensor<double> Accum__D_0__S__D_2__S__D_1__D_3( dist__D_0__S__D_2__S__D_1__D_3, g );
  //T[D02,D1,*,D3]
  DistTensor<double> T__D_0_2__D_1__S__D_3( dist__D_0_2__D_1__S__D_3, g );
  //T[D0,D1,D2,D3]
  DistTensor<double> T__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
  //T[D0,D1,*,D3]
  DistTensor<double> T__D_0__D_1__S__D_3( dist__D_0__D_1__S__D_3, g );
  //T[D20,D1,*,D3]
  DistTensor<double> T__D_2_0__D_1__S__D_3( dist__D_2_0__D_1__S__D_3, g );
  //T[D2,D1,*,D3]
  DistTensor<double> T__D_2__D_1__S__D_3( dist__D_2__D_1__S__D_3, g );
  //T[D2,D3,*,D1]
  DistTensor<double> T__D_2__D_3__S__D_1( dist__D_2__D_3__S__D_1, g );
  //T[D2,D3,*,*]
  DistTensor<double> T__D_2__D_3__S__S( dist__D_2__D_3__S__S, g );
  //T[*,D1,*,D3]
  DistTensor<double> T__S__D_1__S__D_3( dist__S__D_1__S__D_3, g );
  //U[D0,D1,D2,D3]
  DistTensor<double> U__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
  //V[D0,D1,D2,D3]
  DistTensor<double> V__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
  //W[D0,D1,D2,D3]
  DistTensor<double> W__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
  //W[*,D1,D2,D3]
  DistTensor<double> W__S__D_1__D_2__D_3( dist__S__D_1__D_2__D_3, g );
  //W[*,*,D2,D3]
  DistTensor<double> W__S__S__D_2__D_3( dist__S__S__D_2__D_3, g );
  //epsilon[D0,D1,D2,D3]
  DistTensor<double> epsilon__D_0__D_1__D_2__D_3( dist__D_0__D_1__D_2__D_3, g );
  //epsilon[D1,D2,D3] | {0}
  DistTensor<double> epsilon__D_1__D_2__D_3__N_D_0( dist__D_1__D_2__D_3__N_D_0, g );
  //epsilon[D2,D3] | {0,1}
  DistTensor<double> epsilon__D_2__D_3__N_D_0_1( dist__D_2__D_3__N_D_0_1, g );
  //epsilon[D3] | {0,1,2}
  DistTensor<double> epsilon__D_3__N_D_0_1_2( dist__D_3__N_D_0_1_2, g );
  //epsilon[] | {0,1,2,3}
  DistTensor<double> epsilon____N_D_0_1_2_3( dist____N_D_0_1_2_3, g );
  ModeArray modes_0;
  modes_0.push_back(0);
  ModeArray modes_0_2;
  modes_0_2.push_back(0);
  modes_0_2.push_back(2);
  ModeArray modes_1;
  modes_1.push_back(1);
  ModeArray modes_2;
  modes_2.push_back(2);
  std::pair<Index,Index> indexPair_1_3(1,3);
  std::pair<ModeArray,ModeArray> modeArray_1__3;
  modeArray_1__3.first.push_back(1);
  modeArray_1__3.second.push_back(3);
  IndexArray indices_abcd( 4 );
  indices_abcd[0] = 'a';
  indices_abcd[1] = 'b';
  indices_abcd[2] = 'c';
  indices_abcd[3] = 'd';
  IndexArray indices_abij( 4 );
  indices_abij[0] = 'a';
  indices_abij[1] = 'b';
  indices_abij[2] = 'i';
  indices_abij[3] = 'j';
  IndexArray indices_abijcd( 6 );
  indices_abijcd[0] = 'a';
  indices_abijcd[1] = 'b';
  indices_abijcd[2] = 'i';
  indices_abijcd[3] = 'j';
  indices_abijcd[4] = 'c';
  indices_abijcd[5] = 'd';
  IndexArray indices_abijck( 6 );
  indices_abijck[0] = 'a';
  indices_abijck[1] = 'b';
  indices_abijck[2] = 'i';
  indices_abijck[3] = 'j';
  indices_abijck[4] = 'c';
  indices_abijck[5] = 'k';
  IndexArray indices_abijkl( 6 );
  indices_abijkl[0] = 'a';
  indices_abijkl[1] = 'b';
  indices_abijkl[2] = 'i';
  indices_abijkl[3] = 'j';
  indices_abijkl[4] = 'k';
  indices_abijkl[5] = 'l';
  IndexArray indices_abkl( 4 );
  indices_abkl[0] = 'a';
  indices_abkl[1] = 'b';
  indices_abkl[2] = 'k';
  indices_abkl[3] = 'l';
  IndexArray indices_acik( 4 );
  indices_acik[0] = 'a';
  indices_acik[1] = 'c';
  indices_acik[2] = 'i';
  indices_acik[3] = 'k';
  IndexArray indices_bcjk( 4 );
  indices_bcjk[0] = 'b';
  indices_bcjk[1] = 'c';
  indices_bcjk[2] = 'j';
  indices_bcjk[3] = 'k';
  IndexArray indices_cdij( 4 );
  indices_cdij[0] = 'c';
  indices_cdij[1] = 'd';
  indices_cdij[2] = 'i';
  indices_cdij[3] = 'j';
  IndexArray indices_ijkl( 4 );
  indices_ijkl[0] = 'i';
  indices_ijkl[1] = 'j';
  indices_ijkl[2] = 'k';
  indices_ijkl[3] = 'l';
  // T input has 4 dims
  //	Starting distribution: [D0,D1,D2,D3] or _D_0__D_1__D_2__D_3
  unsigned int ten = 40;
  unsigned int twenty = 40;
  ObjShape T__D_0__D_1__D_2__D_3_tempShape;
  T__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  T__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  T__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  T__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  T__D_0__D_1__D_2__D_3.ResizeTo( T__D_0__D_1__D_2__D_3_tempShape );
  MakeUniform( T__D_0__D_1__D_2__D_3 );
  DistTensor<T> T_local( tmen::StringToTensorDist("[(),(),(),()]|(0,1,2,3)"), g );
  GatherAllModes( T__D_0__D_1__D_2__D_3, T_local );
  // W input has 4 dims
  //	Starting distribution: [D0,D1,D2,D3] or _D_0__D_1__D_2__D_3
  ObjShape W__D_0__D_1__D_2__D_3_tempShape;
  W__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  W__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  W__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  W__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  W__D_0__D_1__D_2__D_3.ResizeTo( W__D_0__D_1__D_2__D_3_tempShape );
  MakeUniform( W__D_0__D_1__D_2__D_3 );
  DistTensor<T> W_local( tmen::StringToTensorDist("[(),(),(),()]|(0,1,2,3)"), g );
  GatherAllModes( W__D_0__D_1__D_2__D_3, W_local );
  // V input has 4 dims
  //	Starting distribution: [D0,D1,D2,D3] or _D_0__D_1__D_2__D_3
  ObjShape V__D_0__D_1__D_2__D_3_tempShape;
  V__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  V__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  V__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  V__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  V__D_0__D_1__D_2__D_3.ResizeTo( V__D_0__D_1__D_2__D_3_tempShape );
  MakeUniform( V__D_0__D_1__D_2__D_3 );
  DistTensor<T> V_local( tmen::StringToTensorDist("[(),(),(),()]|(0,1,2,3)"), g );
  GatherAllModes( V__D_0__D_1__D_2__D_3, V_local );
  // U input has 4 dims
  //	Starting distribution: [D0,D1,D2,D3] or _D_0__D_1__D_2__D_3
  ObjShape U__D_0__D_1__D_2__D_3_tempShape;
  U__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  U__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  U__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  U__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  U__D_0__D_1__D_2__D_3.ResizeTo( U__D_0__D_1__D_2__D_3_tempShape );
  MakeUniform( U__D_0__D_1__D_2__D_3 );
  DistTensor<T> U_local( tmen::StringToTensorDist("[(),(),(),()]|(0,1,2,3)"), g );
  GatherAllModes( U__D_0__D_1__D_2__D_3, U_local );
  // Temp input has 4 dims
  //	Starting distribution: [D0,D1,D2,D3] or _D_0__D_1__D_2__D_3
  ObjShape Accum__D_0__D_1__D_2__D_3_tempShape;
  Accum__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  Accum__D_0__D_1__D_2__D_3_tempShape.push_back( twenty );
  Accum__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  Accum__D_0__D_1__D_2__D_3_tempShape.push_back( ten );
  Accum__D_0__D_1__D_2__D_3.ResizeTo( Accum__D_0__D_1__D_2__D_3_tempShape );
  MakeUniform( Accum__D_0__D_1__D_2__D_3 );
  DistTensor<T> Accum_local( tmen::StringToTensorDist("[(),(),(),()]|(0,1,2,3)"), g );
  GatherAllModes( Accum__D_0__D_1__D_2__D_3, Accum_local );
  // ep input has 0 dims
  //	Starting distribution: [] | {0,1,2,3} or ___N_D_0_1_2_3
  ObjShape epsilon____N_D_0_1_2_3_tempShape;
  epsilon____N_D_0_1_2_3.ResizeTo( epsilon____N_D_0_1_2_3_tempShape );
  MakeUniform( epsilon____N_D_0_1_2_3 );
  DistTensor<T> epsilon_local( tmen::StringToTensorDist("[]|(0,1,2,3)"), g );
  GatherAllModes( epsilon____N_D_0_1_2_3, epsilon_local );

  double gflops;
  double startTime;
  double runTime;
  if(commRank == 0)
	std::cout << "starting\n";
  mpi::Barrier(g.OwningComm());
  startTime = mpi::Time();
  //**** (out of 27)
  //------------------------------------//

  tempShape = Accum__D_0__D_1__D_2__D_3.Shape();
  tempShape.push_back( g.Shape()[2] );
  tempShape.push_back( g.Shape()[3] );
  Accum__D_0__D_1__S__S__D_2__D_3.ResizeTo( tempShape );
  //**** (out of 2)
  //**** Is a shadow
  //------------------------------------//

  //PrintData(T__D_0__D_1__D_2__D_3, "T init");
  //Print(T__D_0__D_1__D_2__D_3, "T init");
//  Print(W__D_0__D_1__D_2__D_3, "W init");
//  Print(V__D_0__D_1__D_2__D_3, "V init");
//  Print(U__D_0__D_1__D_2__D_3, "U init");

  // W[*,D1,D2,D3] <- W[D0,D1,D2,D3]
  W__S__D_1__D_2__D_3.AllGatherRedistFrom( W__D_0__D_1__D_2__D_3, 0, modes_0 );
  // W[*,*,D2,D3] <- W[*,D1,D2,D3]
  W__S__S__D_2__D_3.AllGatherRedistFrom( W__S__D_1__D_2__D_3, 1, modes_1 );

  //------------------------------------//

  //****
  //**** (out of 48)
  //**** Is real
  //------------------------------------//

  // T[D0,D1,*,D3] <- T[D0,D1,D2,D3]
  T__D_0__D_1__S__D_3.AllGatherRedistFrom( T__D_0__D_1__D_2__D_3, 2, modes_2 );
  //Print(T__D_0__D_1__S__D_3, tmen::TensorDistToString(T__D_0__D_1__S__D_3.TensorDist()));
  // T[*,D1,*,D3] <- T[D0,D1,*,D3]
  T__S__D_1__S__D_3.AllGatherRedistFrom( T__D_0__D_1__S__D_3, 0, modes_0 );
  //Print(T__S__D_1__S__D_3, tmen::TensorDistToString(T__S__D_1__S__D_3.TensorDist()));
  // T[D02,D1,*,D3] <- T[D0,D1,*,D3]
  T__D_0_2__D_1__S__D_3.LocalRedistFrom( T__D_0__D_1__S__D_3, 0, modes_2 );
  //Print(T__D_0_2__D_1__S__D_3, tmen::TensorDistToString(T__D_0_2__D_1__S__D_3.TensorDist()));
  // T[D20,D1,*,D3] <- T[D02,D1,*,D3]
  T__D_2_0__D_1__S__D_3.PermutationRedistFrom( T__D_0_2__D_1__S__D_3, 0, modes_0_2 );
  //Print(T__D_2_0__D_1__S__D_3, tmen::TensorDistToString(T__D_2_0__D_1__S__D_3.TensorDist()));
  // T[D2,D1,*,D3] <- T[D20,D1,*,D3]
  T__D_2__D_1__S__D_3.AllGatherRedistFrom( T__D_2_0__D_1__S__D_3, 0, modes_0 );
  //Print(T__D_2__D_1__S__D_3, tmen::TensorDistToString(T__D_2__D_1__S__D_3.TensorDist()));
  // T[D2,D3,*,D1] <- T[D2,D1,*,D3]
  T__D_2__D_3__S__D_1.AllToAllDoubleModeRedistFrom( T__D_2__D_1__S__D_3, indexPair_1_3, modeArray_1__3 );
  //Print(T__D_2__D_3__S__D_1, tmen::TensorDistToString(T__D_2__D_3__S__D_1.TensorDist()));
  // T[D2,D3,*,*] <- T[D2,D3,*,D1]
  T__D_2__D_3__S__S.AllGatherRedistFrom( T__D_2__D_3__S__D_1, 3, modes_1 );
  //Print(T__D_2__D_3__S__S, tmen::TensorDistToString(T__D_2__D_3__S__S.TensorDist()));

  //------------------------------------//

//  PrintData(U__D_0__D_1__D_2__D_3, "U");
//  Print(U__D_0__D_1__D_2__D_3, "U elem");
  //PrintData(T__D_2__D_3__S__S, "T");
  //Print(T__D_2__D_3__S__S, "T elem");
//  PrintData(Accum__D_0__D_1__S__S__D_2__D_3, "Accum");
//  Print(Accum__D_0__D_1__S__S__D_2__D_3, "Accum elem");
  //****
  // 1.0 * U[D0,D1,D2,D3]_abcd * T[D2,D3,*,*]_cdij + 0.0 * Accum[D0,D1,*,*,D2,D3]_abijcd
  LocalContract(1.0, U__D_0__D_1__D_2__D_3.LockedTensor(), indices_abcd,
		T__D_2__D_3__S__S.LockedTensor(), indices_cdij,
		0.0, Accum__D_0__D_1__S__S__D_2__D_3.Tensor(), indices_abijcd);
//  Print(Accum__D_0__D_1__S__S__D_2__D_3, "Post Accum elem");
  //**** (out of 1)
  //**** Is a shadow
  //------------------------------------//

  tempShape = Accum__D_0__D_1__D_2__D_3.Shape();
  tempShape.push_back( g.Shape()[3] );
  Accum__D_0__D_1__D_2__S__D_3.ResizeTo( tempShape );
  // Accum[D0,D1,D2,*,D3] <- Accum[D0,D1,*,*,D2,D3] (with SumScatter on D2)
  Accum__D_0__D_1__D_2__S__D_3.ReduceScatterRedistFrom( Accum__D_0__D_1__S__S__D_2__D_3, 4, 2 );
//  Print(Accum__D_0__D_1__D_2__S__D_3, "Post RTO D2");
  // Accum[D0,D1,D2,D3] <- Accum[D0,D1,D2,*,D3] (with SumScatter on D3)
  Accum__D_0__D_1__D_2__D_3.ReduceScatterRedistFrom( Accum__D_0__D_1__D_2__S__D_3, 4, 3 );
//  Print(Accum__D_0__D_1__D_2__D_3, "Post RTO D3");

  //------------------------------------//

  //****
  tempShape = Accum__D_0__D_1__D_2__D_3.Shape();
  tempShape.push_back( g.Shape()[1] );
  tempShape.push_back( g.Shape()[3] );
  Accum__D_0__S__D_2__S__D_1__D_3.ResizeTo( tempShape );
//  PrintData(V__D_0__D_1__D_2__D_3, "V");
//  Print(V__D_0__D_1__D_2__D_3, "V elem");
//  PrintData(T__S__D_1__S__D_3, "T");
//  Print(T__S__D_1__S__D_3, "T elem");
//  PrintData(Accum__D_0__S__D_2__S__D_1__D_3, "Accum");
//  Print(Accum__D_0__S__D_2__S__D_1__D_3, "Accum elem");
  // 1.0 * V[D0,D1,D2,D3]_acik * T[*,D1,*,D3]_bcjk + 0.0 * Accum[D0,*,D2,*,D1,D3]_abijck
  LocalContract(1.0, V__D_0__D_1__D_2__D_3.LockedTensor(), indices_acik,
		T__S__D_1__S__D_3.LockedTensor(), indices_bcjk,
		0.0, Accum__D_0__S__D_2__S__D_1__D_3.Tensor(), indices_abijck);
//  Print(Accum__D_0__S__D_2__S__D_1__D_3, "Post Accum elem");
  //**** (out of 1)
  //**** Is a shadow
  //------------------------------------//

  tempShape = Accum__D_0__D_1__D_2__D_3.Shape();
  tempShape.push_back( g.Shape()[3] );
  Accum__D_0__D_1__D_2__S__D_3.ResizeTo( tempShape );
  // Accum[D0,D1,D2,*,D3] <- Accum[D0,*,D2,*,D1,D3] (with SumScatter on D1)
  Accum__D_0__D_1__D_2__S__D_3.ReduceScatterRedistFrom( Accum__D_0__S__D_2__S__D_1__D_3, 4, 1 );
//  Print(Accum__D_0__D_1__D_2__S__D_3, "RTO D1");
  // Accum[D0,D1,D2,D3] <- Accum[D0,D1,D2,*,D3] (with SumScatter on D3)
  Accum__D_0__D_1__D_2__D_3.ReduceScatterUpdateRedistFrom( Accum__D_0__D_1__D_2__S__D_3, 1.0, 4, 3 );
//  Print(Accum__D_0__D_1__D_2__D_3, "RTO D3");

  //------------------------------------//

  //****
  tempShape = Accum__D_0__D_1__D_2__D_3.Shape();
  tempShape.push_back( g.Shape()[2] );
  tempShape.push_back( g.Shape()[3] );
  Accum__D_0__D_1__S__S__D_2__D_3.ResizeTo( tempShape );
//  PrintData(W__S__S__D_2__D_3, "W");
//  Print(W__S__S__D_2__D_3, "W elem");
//  PrintData(T__D_0__D_1__D_2__D_3, "T");
//  Print(T__D_0__D_1__D_2__D_3, "T elem");
//  PrintData(Accum__D_0__D_1__S__S__D_2__D_3, "Accum");
//  Print(Accum__D_0__D_1__S__S__D_2__D_3, "Accum elem");
  // 1.0 * W[*,*,D2,D3]_ijkl * T[D0,D1,D2,D3]_abkl + 0.0 * Accum[D0,D1,*,*,D2,D3]_abijkl
  LocalContract(1.0, W__S__S__D_2__D_3.LockedTensor(), indices_ijkl,
		T__D_0__D_1__D_2__D_3.LockedTensor(), indices_abkl,
		0.0, Accum__D_0__D_1__S__S__D_2__D_3.Tensor(), indices_abijkl);
//  Print(Accum__D_0__D_1__S__S__D_2__D_3, "post Accum elem");
  //**** (out of 1)
  //**** Is a shadow
  //------------------------------------//

  tempShape = Accum__D_0__D_1__D_2__D_3.Shape();
  tempShape.push_back( g.Shape()[3] );
  Accum__D_0__D_1__D_2__S__D_3.ResizeTo( tempShape );
  // Accum[D0,D1,D2,*,D3] <- Accum[D0,D1,*,*,D2,D3] (with SumScatter on D2)
  Accum__D_0__D_1__D_2__S__D_3.ReduceScatterRedistFrom( Accum__D_0__D_1__S__S__D_2__D_3, 4, 2 );
//  Print(Accum__D_0__D_1__D_2__S__D_3, "post RTO D2");
  // Accum[D0,D1,D2,D3] <- Accum[D0,D1,D2,*,D3] (with SumScatter on D3)
  Accum__D_0__D_1__D_2__D_3.ReduceScatterUpdateRedistFrom( Accum__D_0__D_1__D_2__S__D_3, 1.0, 4, 3 );
//  Print(Accum__D_0__D_1__D_2__D_3, "post RTO D3");

  //------------------------------------//

  //****

  //------------------------------------//

  //****
  //**** (out of 1)
  //------------------------------------//

  tempShape = epsilon____N_D_0_1_2_3.Shape();
  tempShape.push_back( g.Shape()[0] );
  tempShape.push_back( g.Shape()[1] );
  tempShape.push_back( g.Shape()[2] );
  tempShape.push_back( g.Shape()[3] );
  epsilon__D_0__D_1__D_2__D_3.ResizeTo( tempShape );
//  PrintData(T__D_0__D_1__D_2__D_3, "T");
//  Print(T__D_0__D_1__D_2__D_3, "T elem");
//  PrintData(Accum__D_0__D_1__D_2__D_3, "Accum");
//  Print(Accum__D_0__D_1__D_2__D_3, "Accum elem");
//  PrintData(epsilon__D_0__D_1__D_2__D_3, "eps");
//  Print(epsilon__D_0__D_1__D_2__D_3, "eps elem");
  // 1.0 * T[D0,D1,D2,D3]_abij * Accum[D0,D1,D2,D3]_abij + 0.0 * epsilon[D0,D1,D2,D3]_abij
  LocalContract(1.0, T__D_0__D_1__D_2__D_3.LockedTensor(), indices_abij,
		Accum__D_0__D_1__D_2__D_3.LockedTensor(), indices_abij,
		0.0, epsilon__D_0__D_1__D_2__D_3.Tensor(), indices_abij);
//  Print(epsilon__D_0__D_1__D_2__D_3, "post eps elem");
  tempShape = epsilon____N_D_0_1_2_3.Shape();
  tempShape.push_back( g.Shape()[1] );
  tempShape.push_back( g.Shape()[2] );
  tempShape.push_back( g.Shape()[3] );
  epsilon__D_1__D_2__D_3__N_D_0.ResizeTo( tempShape );
  // epsilon[D1,D2,D3] | {0} <- epsilon[D0,D1,D2,D3] (with SumScatter on D0)
  epsilon__D_1__D_2__D_3__N_D_0.ReduceToOneRedistFrom( epsilon__D_0__D_1__D_2__D_3, 0 );
//  Print(epsilon__D_1__D_2__D_3__N_D_0, "post RTO D0");
  tempShape = epsilon____N_D_0_1_2_3.Shape();
  tempShape.push_back( g.Shape()[2] );
  tempShape.push_back( g.Shape()[3] );
  epsilon__D_2__D_3__N_D_0_1.ResizeTo( tempShape );
  // epsilon[D2,D3] | {0,1} <- epsilon[D1,D2,D3] | {0} (with SumScatter on D1)
  epsilon__D_2__D_3__N_D_0_1.ReduceToOneRedistFrom( epsilon__D_1__D_2__D_3__N_D_0, 0 );
//  Print(epsilon__D_2__D_3__N_D_0_1, "post RTO D1");
  tempShape = epsilon____N_D_0_1_2_3.Shape();
  tempShape.push_back( g.Shape()[3] );
  epsilon__D_3__N_D_0_1_2.ResizeTo( tempShape );
  // epsilon[D3] | {0,1,2} <- epsilon[D2,D3] | {0,1} (with SumScatter on D2)
  epsilon__D_3__N_D_0_1_2.ReduceToOneRedistFrom( epsilon__D_2__D_3__N_D_0_1, 0 );
//  Print(epsilon__D_3__N_D_0_1_2, "post RTO D2");
  // epsilon[] | {0,1,2,3} <- epsilon[D3] | {0,1,2} (with SumScatter on D3)
  epsilon____N_D_0_1_2_3.ReduceToOneRedistFrom( epsilon__D_3__N_D_0_1_2, 0 );
//  Print(epsilon____N_D_0_1_2_3, "post RTO D3");

  mpi::Barrier(g.OwningComm());
  runTime = mpi::Time() - startTime;
  if(commRank == 0)
    std::cout << "done\n";
  gflops = 2*((pow(twenty, 4)*pow(ten,2)) + (pow(ten, 3) * pow(twenty,3)) + (pow(ten, 4)*pow(twenty, 2)) + (pow(twenty, 2)*pow(ten, 2)))/(1.e9*runTime);
  //------------------------------------//




  LocalContractAndLocalEliminate(1.0, U_local.LockedTensor(), indices_abcd,
				 T_local.LockedTensor(), indices_cdij,
				 0.0, Accum_local.Tensor(), indices_abij);



  LocalContractAndLocalEliminate(1.0, V_local.LockedTensor(), indices_acik,
				 T_local.LockedTensor(), indices_bcjk,
				 1.0, Accum_local.Tensor(), indices_abij);



  LocalContractAndLocalEliminate(1.0, W_local.LockedTensor(), indices_ijkl,
				 T_local.LockedTensor(), indices_abkl,
				 1.0, Accum_local.Tensor(), indices_abij);



  IndexArray blank_indices;

  LocalContractAndLocalEliminate(1.0, T_local.LockedTensor(), indices_abij,
				 Accum_local.LockedTensor(), indices_abij,
				 0.0, epsilon_local.Tensor(), blank_indices);

  DistTensor<T> epsilon_local_comparison( tmen::StringToTensorDist("[]|(0,1,2,3)"), g );    
  GatherAllModes(epsilon____N_D_0_1_2_3, epsilon_local_comparison);

  DistTensor<T> diffTensor( tmen::StringToTensorDist("[]|(0,1,2,3)"), g );    
  diffTensor.ResizeTo(epsilon_local_comparison);
  Diff( epsilon_local_comparison.LockedTensor(), epsilon_local.LockedTensor(), diffTensor.Tensor() );


  if (commRank == 0) {
    cout << "Norm of distributed is " << Norm(epsilon_local_comparison.LockedTensor()) << endl;
    cout << "Norm of local is " << Norm(epsilon_local.LockedTensor()) << endl;
    cout << "Norm is " << Norm(diffTensor.LockedTensor()) << endl;
    cout << "Time is " << runTime << endl;
    cout << "GFlops is " << gflops << endl;
  }
}

int 
main( int argc, char* argv[] )
{
  Initialize( argc, argv );
  Unsigned i;
  mpi::Comm comm = mpi::COMM_WORLD;
  const Int commRank = mpi::CommRank( comm );
  const Int commSize = mpi::CommSize( comm );
  printf("My Rank: %d\n", commRank);
  try
    {
      Params args;

      ProcessInput(argc, argv, args);

      if(commRank == 0 && commSize != args.nProcs){
	std::cerr << "program not started with correct number of processes\n";
	std::cerr << commSize << " vs " << args.nProcs << std::endl;
	Usage();
	throw ArgException();
      }

      if(commRank == 0){
	printf("Creating %d", args.gridShape[0]);
	for(i = 1; i < GRIDORDER; i++)
	  printf(" x %d", args.gridShape[i]);
	printf(" grid\n");
      }

      const Grid g( comm, args.gridShape );

      if( commRank == 0 )
        {
	  std::cout << "------------------" << std::endl
		    << "Testing with doubles:" << std::endl
		    << "------------------" << std::endl;
        }
      DistTensorTest<double>( g );

    }
  catch( std::exception& e ) { ReportException(e); }

  Finalize();
  //printf("Completed\n");
  return 0;
}
