/*
   Copyright (c) 2009-2013, Jack Poulson
                      2013, Jeff Hammond
                      2013, Jed Brown
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "tensormental.hpp"
#include <algorithm>
#include "tensormental/core/tensor.hpp"
namespace tmen{

//TODO: Check that allToAllIndices and commGroups are valid
template <typename T>
Int DistTensor<T>::CheckAllToAllDoubleModeCommRedist(const DistTensor<T>& A, const std::pair<Mode, Mode>& allToAllModes, const std::pair<ModeArray, ModeArray >& a2aCommGroups){
    if(A.Order() != Order())
        LogicError("CheckAllToAllDoubleModeRedist: Objects being redistributed must be of same order");
    Unsigned i;
    for(i = 0; i < A.Order(); i++){
        if(i != allToAllModes.first && i != allToAllModes.second){
            if(ModeDist(i) != A.ModeDist(i))
                LogicError("CheckAllToAllDoubleModeRedist: Non-redist modes must have same distribution");
        }
    }
    return 1;
}

template <typename T>
void DistTensor<T>::AllToAllCommRedist(const DistTensor<T>& A, const ModeArray& changedA2AModes, const ModeArray& commModes){
//    if(!CheckAllToAllDoubleModeCommRedist(A, a2aModes, a2aCommGroups))
//        LogicError("AllToAllDoubleModeRedist: Invalid redistribution request");

    const tmen::Grid& g = A.Grid();

//    PrintVector(commModes, "commModes");
    const mpi::Comm comm = GetCommunicatorForModes(commModes, g);

    if(!A.Participating())
        return;
//    printf("Participating\n");
//    printf("commRank: %d\n", mpi::CommRank(comm));
//    printf("commSize: %d\n", mpi::CommSize(comm));
    Unsigned sendSize, recvSize;

    const Unsigned nRedistProcs = Max(1, prod(FilterVector(g.Shape(), commModes)));
    const ObjShape maxLocalShapeA = A.MaxLocalShape();
    const ObjShape maxLocalShapeB = MaxLocalShape();
    const ObjShape commDataShape = prod(maxLocalShapeA) < prod(maxLocalShapeB) ? maxLocalShapeA : maxLocalShapeB;

    sendSize = prod(commDataShape);
    recvSize = sendSize;

    Memory<T> auxMemory;
    T* auxBuf = auxMemory.Require((sendSize + recvSize) * nRedistProcs);
    MemZero(&(auxBuf[0]), (sendSize + recvSize) * nRedistProcs);

    T* sendBuf = &(auxBuf[0]);
    T* recvBuf = &(auxBuf[sendSize*nRedistProcs]);

    PackA2ACommSendBuf(A, changedA2AModes, commModes, commDataShape, sendBuf);

    mpi::AllToAll(sendBuf, sendSize, recvBuf, recvSize, comm);

    if(!(Participating()))
        return;
    UnpackA2ACommRecvBuf(recvBuf, changedA2AModes, commModes, commDataShape, A);
}

template <typename T>
void DistTensor<T>::PackA2ACommSendBuf(const DistTensor<T>& A, const ModeArray& changedA2AModes, const ModeArray& commModes, const ObjShape& sendShape, T * const sendBuf){
    const Unsigned order = A.Order();
    const T* dataBuf = A.LockedBuffer();

//    std::cout << "dataBuf:";
//    for(Unsigned i = 0; i < prod(A.LocalShape()); i++){
//        std::cout << " " <<  dataBuf[i];
//    }
//    std::cout << std::endl;

    const Location zeros(order, 0);

    //----------------------------------------
    //----------------------------------------
    //----------------------------------------

    const tmen::GridView gvA = A.GetGridView();
    const tmen::GridView gvB = GetGridView();
    const ObjShape gvAShape = gvA.ParticipatingShape();
    const ObjShape gvBShape = gvB.ParticipatingShape();

    const std::vector<Unsigned> commLCMs = LCMs(gvAShape, gvBShape);
    const std::vector<Unsigned> modeStrideFactor = ElemwiseDivide(commLCMs, gvAShape);

    PackData packData;
    packData.loopShape = A.LocalShape();
    packData.srcBufStrides = ElemwiseProd(A.LocalStrides(), modeStrideFactor);

    packData.dstBufStrides = Dimensions2Strides(sendShape);

    packData.loopStarts = zeros;
    packData.loopIncs = modeStrideFactor;

    const Location myFirstElemLoc = A.ModeShifts();

//    PrintVector(packElem, "packElem");
//    PrintVector(changedA2AModes, "changedA2AModes");
//    PrintVector(A.Shape(), "shapeA");
//    PrintVector(A.LocalShape(), "localShapeA");
//    PrintVector(modeStrideFactor, "modeStrideFactor");

    if(ElemwiseLessThan(myFirstElemLoc, A.Shape())){
//        ElemSelectHelper(packData, changedA2AModes.size() - 1, commModes, changedA2AModes, myFirstElemLoc, modeStrideFactor, prod(sendShape), A, &(dataBuf[0]), &(sendBuf[0]));
        ElemSelectData elemData;
        elemData.commModes = commModes;
        elemData.changedModes = changedA2AModes;
        elemData.packElem = myFirstElemLoc;
        elemData.loopShape = modeStrideFactor;
        elemData.nElemsPerProc = prod(sendShape);
        elemData.srcElem = zeros;
        elemData.srcStrides = A.LocalStrides();

        ElemSelectPackHelper(packData, elemData, changedA2AModes.size() - 1, A, &(dataBuf[0]), &(sendBuf[0]));
    }
}

template<typename T>
void DistTensor<T>::UnpackA2ACommRecvBuf(const T * const recvBuf, const ModeArray& changedA2AModes, const ModeArray& commModesAll, const ObjShape& recvShape, const DistTensor<T>& A){
    const Unsigned order = A.Order();
    T* dataBuf = Buffer();

    const tmen::GridView gvA = A.GetGridView();
    const tmen::GridView gvB = GetGridView();

    const Location zeros(order, 0);
    const Location ones(order, 1);

    //------------------------------------
    //------------------------------------
    //------------------------------------

//    ModeArray changedA2AModes = a2aModesFrom;
//    changedA2AModes.insert(changedA2AModes.end(), a2aModesTo.begin(), a2aModesTo.end());
//    std::sort(changedA2AModes.begin(), changedA2AModes.end());
//    changedA2AModes.erase(std::unique(changedA2AModes.begin(), changedA2AModes.end()), changedA2AModes.end());

//    printf("unpack size changedModes: %d\n", changedA2AModes.size());
//    ModeArray commModesAll;
//    for(i = 0; i < commGroups.size(); i++)
//        commModesAll.insert(commModesAll.end(), commGroups[i].begin(), commGroups[i].end());
//    std::sort(commModesAll.begin(), commModesAll.end());

    std::vector<Unsigned> commLCMs = tmen::LCMs(gvA.ParticipatingShape(), gvB.ParticipatingShape());
    std::vector<Unsigned> modeStrideFactor = ElemwiseDivide(commLCMs, gvB.ParticipatingShape());

//    const ObjShape maxLocalShapeA = A.MaxLocalShape();
//    const ObjShape maxLocalShapeB = MaxLocalShape();
//    const ObjShape recvShape = prod(maxLocalShapeA) < prod(maxLocalShapeB) ? maxLocalShapeA : maxLocalShapeB;

    const tmen::Grid& g = Grid();
    const Unsigned nRedistProcsAll = prod(FilterVector(g.Shape(), commModesAll));
//    std::cout << "recvBuf:";
//    for(Unsigned i = 0; i < nRedistProcsAll * prod(recvShape); i++){
//        std::cout << " " << recvBuf[i];
//    }
//    std::cout << std::endl;

//    PrintVector(modeStrideFactor, "modeStrideFactor");
    PackData unpackData;
    unpackData.loopShape = LocalShape();
    unpackData.srcBufStrides = Dimensions2Strides(recvShape);
    unpackData.dstBufStrides = ElemwiseProd(LocalStrides(), modeStrideFactor);

    unpackData.loopStarts = zeros;
    unpackData.loopIncs = modeStrideFactor;

//    Unsigned a2aMode2Stride = LocalModeStride(a2aMode2);
//    Unsigned a2aMode1Stride = LocalModeStride(a2aMode1);

    const Location myFirstElemLoc = ModeShifts();

//    PrintVector(unpackElem, "unpackElem");
//    PrintVector(changedA2AModes, "uniqueA2AModesTo");
//    PrintVector(Shape(), "shapeA");
//    PrintVector(LocalShape(), "localShapeA");

    if(ElemwiseLessThan(myFirstElemLoc, A.Shape())){
//        A2AUnpackTestHelper(unpackData, changedA2AModes.size() - 1, commModesAll, changedA2AModes, myFirstElemLoc, myFirstElemLoc, modeStrideFactor, prod(recvShape), A, &(recvBuf[0]), &(dataBuf[0]));
        ElemSelectData elemData;
        elemData.commModes = commModesAll;
        elemData.changedModes = changedA2AModes;
        elemData.packElem = myFirstElemLoc;
        elemData.loopShape = modeStrideFactor;
        elemData.nElemsPerProc = prod(recvShape);
        elemData.dstElem = zeros;
        elemData.dstStrides = LocalStrides();

        ElemSelectUnpackHelper(unpackData, elemData, changedA2AModes.size() - 1, A, &(recvBuf[0]), &(dataBuf[0]));
    }
    Unsigned i;
//    printf("dataBuf:");
//    for(i = 0; i < prod(LocalShape()); i++)
//        printf(" %d", dataBuf[i]);
//    printf("\n");
}

///////////////////////
///////////////////////
///////////////////////

template <typename T>
void DistTensor<T>::AllToAllCommRedistWithPermutation(const DistTensor<T>& A, const ModeArray& changedA2AModes, const ModeArray& commModes){
    //    if(!CheckAllToAllDoubleModeCommRedist(A, a2aModes, a2aCommGroups))
    //        LogicError("AllToAllDoubleModeRedist: Invalid redistribution request");

        const tmen::Grid& g = A.Grid();

    //    PrintVector(commModes, "commModes");
        const mpi::Comm comm = GetCommunicatorForModes(commModes, g);

        if(!A.Participating())
            return;
    //    printf("Participating\n");
    //    printf("commRank: %d\n", mpi::CommRank(comm));
    //    printf("commSize: %d\n", mpi::CommSize(comm));
        Unsigned sendSize, recvSize;

        const Unsigned nRedistProcs = Max(1, prod(FilterVector(g.Shape(), commModes)));
        const ObjShape maxLocalShapeA = A.MaxLocalShape();
        const ObjShape maxLocalShapeB = MaxLocalShape();
        const ObjShape commDataShape = prod(maxLocalShapeA) < prod(maxLocalShapeB) ? maxLocalShapeA : maxLocalShapeB;

        sendSize = prod(commDataShape);
        recvSize = sendSize;

        Memory<T> auxMemory;
        T* auxBuf = auxMemory.Require((sendSize + recvSize) * nRedistProcs);
        MemZero(&(auxBuf[0]), (sendSize + recvSize) * nRedistProcs);

        T* sendBuf = &(auxBuf[0]);
        T* recvBuf = &(auxBuf[sendSize*nRedistProcs]);

        PackA2ACommSendBuf(A, changedA2AModes, commModes, commDataShape, sendBuf);

        mpi::AllToAll(sendBuf, sendSize, recvBuf, recvSize, comm);

        if(!(Participating()))
            return;
        UnpackA2ACommRecvBufWithPermutation(recvBuf, changedA2AModes, commModes, commDataShape, A);
}

template<typename T>
void DistTensor<T>::UnpackA2ACommRecvBufWithPermutation(const T * const recvBuf, const ModeArray& changedA2AModes, const ModeArray& commModesAll, const ObjShape& recvShape, const DistTensor<T>& A){
    const Unsigned order = A.Order();
    T* dataBuf = Buffer();

    const tmen::GridView gvA = A.GetGridView();
    const tmen::GridView gvB = GetGridView();

    const Location zeros(order, 0);
    const Location ones(order, 1);

    //------------------------------------
    //------------------------------------
    //------------------------------------

//    ModeArray changedA2AModes = a2aModesFrom;
//    changedA2AModes.insert(changedA2AModes.end(), a2aModesTo.begin(), a2aModesTo.end());
//    std::sort(changedA2AModes.begin(), changedA2AModes.end());
//    changedA2AModes.erase(std::unique(changedA2AModes.begin(), changedA2AModes.end()), changedA2AModes.end());

//    printf("unpack size changedModes: %d\n", changedA2AModes.size());
//    ModeArray commModesAll;
//    for(i = 0; i < commGroups.size(); i++)
//        commModesAll.insert(commModesAll.end(), commGroups[i].begin(), commGroups[i].end());
//    std::sort(commModesAll.begin(), commModesAll.end());

    std::vector<Unsigned> commLCMs = tmen::LCMs(gvA.ParticipatingShape(), gvB.ParticipatingShape());
    std::vector<Unsigned> modeStrideFactor = ElemwiseDivide(commLCMs, gvB.ParticipatingShape());

//    const ObjShape maxLocalShapeA = A.MaxLocalShape();
//    const ObjShape maxLocalShapeB = MaxLocalShape();
//    const ObjShape recvShape = prod(maxLocalShapeA) < prod(maxLocalShapeB) ? maxLocalShapeA : maxLocalShapeB;

//    const tmen::Grid& g = Grid();
//    const Unsigned nRedistProcsAll = prod(FilterVector(g.Shape(), commModesAll));
//    std::cout << "recvBuf:";
//    for(Unsigned i = 0; i < nRedistProcsAll * prod(recvShape); i++){
//        std::cout << " " << recvBuf[i];
//    }
//    std::cout << std::endl;

//    PrintVector(modeStrideFactor, "modeStrideFactor");
//    PackData unpackData;
//    unpackData.loopShape = LocalShape();
//    unpackData.srcBufStrides = Dimensions2Strides(recvShape);
//    unpackData.dstBufStrides = ElemwiseProd(LocalStrides(), modeStrideFactor);
//
//    unpackData.loopStarts = zeros;
//    unpackData.loopIncs = modeStrideFactor;

//    Unsigned a2aMode2Stride = LocalModeStride(a2aMode2);
//    Unsigned a2aMode1Stride = LocalModeStride(a2aMode1);

    const Location myFirstElemLoc = ModeShifts();

//    PrintVector(unpackElem, "unpackElem");
//    PrintVector(changedA2AModes, "uniqueA2AModesTo");
//    PrintVector(Shape(), "shapeA");
//    PrintVector(LocalShape(), "localShapeA");

    //Local storage is permuted.  That is why tmpStrides not LocalStrides()

    Permutation invPerm = DetermineInversePermutation(localPerm_);
    PackData unpackData;
    unpackData.loopShape = LocalShape();
//    PrintVector(localPerm_, "localPerm_");
    unpackData.srcBufStrides = PermuteVector(Dimensions2Strides(recvShape), localPerm_);
//    unpackData.dstBufStrides = FilterVector(tmpStrides, invPerm);
    unpackData.dstBufStrides = ElemwiseProd(LocalStrides(), PermuteVector(modeStrideFactor, localPerm_));
//    unpackData.dstBufStrides = FilterVector(ElemwiseProd(LocalStrides(), modeStrideFactor), invPerm);

    unpackData.loopStarts = zeros;
    unpackData.loopIncs = PermuteVector(modeStrideFactor, localPerm_);

//    PrintVector(recvShape, "srcShape");
//    PrintVector(unpackData.loopShape, "unpackShape");
//    PrintVector(unpackData.loopIncs, "loop incs");
//    PrintVector(unpackData.srcBufStrides, "srcStrides");
//    PrintVector(unpackData.dstBufStrides, "dstStrides");
//    PrintVector(LocalStrides(), "Bs local strides");

    if(ElemwiseLessThan(myFirstElemLoc, A.Shape())){
//        A2AUnpackTestHelper(unpackData, changedA2AModes.size() - 1, commModesAll, changedA2AModes, myFirstElemLoc, myFirstElemLoc, modeStrideFactor, prod(recvShape), A, &(recvBuf[0]), &(dataBuf[0]));
        ElemSelectData elemData;
        elemData.commModes = commModesAll;
//        elemData.changedModes = FilterVector(localPerm_, changedA2AModes);
        elemData.packElem = myFirstElemLoc;
        elemData.loopShape = modeStrideFactor;
        elemData.nElemsPerProc = prod(recvShape);
        elemData.dstElem = zeros;
        elemData.dstStrides = LocalStrides();
        elemData.permutation = localPerm_;

        ElemSelectUnpackHelperWithPermutation(unpackData, elemData, order - 1, A, &(recvBuf[0]), &(dataBuf[0]));
    }

//    Unsigned i;
//    printf("dataBuf:");
//    for(i = 0; i < prod(LocalShape()); i++)
//        printf(" %d", dataBuf[i]);
//    printf("\n");

//    if(ElemwiseLessThan(myFirstElemLoc, A.Shape())){
////        A2AUnpackTestHelper(unpackData, changedA2AModes.size() - 1, commModesAll, changedA2AModes, myFirstElemLoc, myFirstElemLoc, modeStrideFactor, prod(recvShape), A, &(recvBuf[0]), &(dataBuf[0]));
//        ElemSelectData elemData;
//        elemData.commModes = commModesAll;
//        elemData.changedModes = changedA2AModes;
//        elemData.packElem = myFirstElemLoc;
//        elemData.loopShape = modeStrideFactor;
//        elemData.nElemsPerProc = prod(recvShape);
//        elemData.dstElem = zeros;
//        elemData.dstStrides = LocalStrides();
//
//        ElemSelectUnpackHelperWithPermutation(unpackData, elemData, changedA2AModes.size() - 1, A, &(recvBuf[0]), &(dataBuf[0]));
//    }

}

#define PROTO(T) template class DistTensor<T>
#define COPY(T) \
  template DistTensor<T>::DistTensor( const DistTensor<T>& A )
#define FULL(T) \
  PROTO(T);


FULL(Int);
#ifndef DISABLE_FLOAT
FULL(float);
#endif
FULL(double);

#ifndef DISABLE_COMPLEX
#ifndef DISABLE_FLOAT
FULL(Complex<float>);
#endif
FULL(Complex<double>);
#endif

} //namespace tmen
