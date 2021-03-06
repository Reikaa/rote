/*
   Copyright (c) 2009-2013, Jack Poulson
                      2013, Jeff Hammond
                      2013, Jed Brown
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "rote.hpp"

namespace rote{

template <typename T>
bool DistTensor<T>::CheckAllReduceCommRedist(const DistTensor<T>& A){
	return CheckAllGatherCommRedist(A);
}

template <typename T>
void DistTensor<T>::AllReduceUpdateCommRedist(const T alpha, const DistTensor<T>& A, const T beta, const ModeArray& commModes){
  if(!CheckAllReduceCommRedist(A))
    LogicError("AllReduceRedist: Invalid redistribution request");
  const rote::Grid& g = A.Grid();

  const mpi::Comm comm = this->GetCommunicatorForModes(commModes, g);

  if(!A.Participating())
      return;

  //Determine buffer sizes for communication
  const ObjShape commDataShape = this->MaxLocalShape();
  const Unsigned sendSize = prod(commDataShape);
  const Unsigned recvSize = sendSize;

  T* auxBuf = this->auxMemory_.Require(sendSize + recvSize);
	MemZero(&(auxBuf[0]), sendSize + recvSize);

  T* sendBuf = &(auxBuf[0]);
  T* recvBuf = &(auxBuf[sendSize]);

 // PrintArray(A.LockedBuffer(), A.LocalShape(), A.LocalStrides(), "srcBuf");
 // PrintVector(commDataShape, "AR commDataShape");

  //Pack the data
  PROFILE_SECTION("ARPack");
  PackAGCommSendBuf(A, sendBuf);
  PROFILE_STOP;

  // PrintArray(sendBuf, commDataShape, "sendBuf");

  //Communicate the data
  PROFILE_SECTION("ARComm");
  //Realignment
  T* alignSendBuf = &(sendBuf[0]);
  T* alignRecvBuf = &(recvBuf[0]);

  bool didAlign = AlignCommBufRedist(A, alignSendBuf, sendSize, alignRecvBuf, sendSize);
  if(didAlign){
		sendBuf = &(alignRecvBuf[0]);
		recvBuf = &(alignSendBuf[0]);
  }

  mpi::AllReduce(sendBuf, recvBuf, recvSize, comm);
  PROFILE_STOP;

  // PrintArray(recvBuf, commDataShape, "recvBuf");

  //Unpack the data (if participating)
  PROFILE_SECTION("ARUnpack");
  UnpackRSUCommRecvBuf(recvBuf, alpha, beta);
  PROFILE_STOP;

  // PrintArray(this->LockedBuffer(), this->LocalShape(), this->LocalStrides(), "myBuf");

  this->auxMemory_.Release();
}

#define FULL(T) \
    template class DistTensor<T>;

FULL(Int)
#ifndef DISABLE_FLOAT
FULL(float)
#endif
FULL(double)

#ifndef DISABLE_COMPLEX
#ifndef DISABLE_FLOAT
FULL(std::complex<float>)
#endif
FULL(std::complex<double>)
#endif

} //namespace rote
