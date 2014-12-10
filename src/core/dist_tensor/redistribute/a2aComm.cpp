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
void DistTensor<T>::AllToAllCommRedist(const DistTensor<T>& A, const ModeArray& commModes){
    //    if(!CheckAllToAllDoubleModeCommRedist(A, a2aModes, a2aCommGroups))
    //        LogicError("AllToAllDoubleModeRedist: Invalid redistribution request");

        const tmen::Grid& g = A.Grid();

        const mpi::Comm comm = GetCommunicatorForModes(commModes, g);

        if(!A.Participating())
            return;

        //Determine buffer sizes for communication
        Unsigned sendSize, recvSize;
        const Unsigned nRedistProcs = Max(1, prod(FilterVector(g.Shape(), commModes)));
        const ObjShape maxLocalShapeA = A.MaxLocalShape();
        const ObjShape maxLocalShapeB = MaxLocalShape();

        const tmen::GridView gvA = A.GetGridView();
        const tmen::GridView gvB = GetGridView();
        const ObjShape gvAShape = gvA.ParticipatingShape();
        const ObjShape gvBShape = gvB.ParticipatingShape();

        //For unaligned communications
        const std::vector<Unsigned> alignments = Alignments();
        const std::vector<Unsigned> alignmentsA = A.Alignments();
        const TensorDistribution tensorDist = A.TensorDist();

        std::vector<Unsigned> localPackStrides(maxLocalShapeA.size());
        localPackStrides = ElemwiseDivide(LCMs(gvBShape, gvAShape), gvAShape);
        ObjShape commDataShape(maxLocalShapeA.size());
        commDataShape = IntCeils(maxLocalShapeA, localPackStrides);

        sendSize = prod(commDataShape);
        recvSize = sendSize;

        T* auxBuf;
        PROFILE_SECTION("A2ARequire");
        PROFILE_FLOPS((sendSize + recvSize) * nRedistProcs);
        auxBuf = this->auxMemory_.Require((sendSize + recvSize) * nRedistProcs);
        PROFILE_STOP;

        T* sendBuf = &(auxBuf[0]);
        T* recvBuf = &(auxBuf[sendSize*nRedistProcs]);

        const T* dataBuf = A.LockedBuffer();
        PrintArray(dataBuf, A.LocalShape(), A.LocalStrides(), "srcBuf");

        //Pack the data
        PROFILE_SECTION("A2APack");
        PROFILE_FLOPS(prod(maxLocalShapeA));
        PackA2ACommSendBuf(A, commModes, commDataShape, sendBuf);
        PROFILE_STOP;

        ObjShape sendShape = commDataShape;
        sendShape.insert(sendShape.end(), nRedistProcs);
        PrintArray(sendBuf, sendShape, "sendBuf");

        //Communicate the data
        PROFILE_SECTION("A2AComm");

        Location firstOwnerA = GridViewLoc2GridLoc(A.Alignments(), gvA);
        Location firstOwnerB = GridViewLoc2GridLoc(Alignments(), gvB);
        if(AnyElemwiseNotEqual(firstOwnerA, firstOwnerB)){
            PrintVector(g.Loc(), "myGridLoc");
            PrintVector(firstOwnerA, "firstOwnerA");
            PrintVector(firstOwnerB, "firstOwnerB");
            T* alignSendBuf = &(sendBuf[0]);
            T* alignRecvBuf = &(recvBuf[0]);

            std::vector<Unsigned> alignDiff = ElemwiseSubtract(firstOwnerA, firstOwnerB);
            PrintVector(alignDiff, "alignDiff");
            PrintVector(ElemwiseSum(g.Loc(), alignDiff), "alignDiff + myLoc");
            Location sendGridLoc = ElemwiseMod(ElemwiseSum(ElemwiseSubtract(g.Loc(), alignDiff), g.Shape()), g.Shape());
            Location recvGridLoc = ElemwiseMod(ElemwiseSum(ElemwiseSum(g.Loc(), alignDiff), g.Shape()), g.Shape());

            //Create the communicator to involve all processes we need to fix misalignment
            ModeArray misalignedModes;
            for(Unsigned i = 0; i < firstOwnerB.size(); i++){
                if(firstOwnerB[i] != firstOwnerA[i]){
                    misalignedModes.insert(misalignedModes.end(), i);
                }
            }
            std::sort(misalignedModes.begin(), misalignedModes.end());
            PrintVector(misalignedModes, "misalignedModes");
            mpi::Comm sendRecvComm = GetCommunicatorForModes(misalignedModes, g);
            printf("myRank is: %d\n", mpi::CommRank(sendRecvComm));

            Location sendSliceLoc = FilterVector(sendGridLoc, misalignedModes);
            Location recvSliceLoc = FilterVector(recvGridLoc, misalignedModes);
            ObjShape gridSliceShape = FilterVector(g.Shape(), misalignedModes);

            Unsigned sendLinLoc = Loc2LinearLoc(sendSliceLoc, gridSliceShape);
            Unsigned recvLinLoc = Loc2LinearLoc(recvSliceLoc, gridSliceShape);


            PrintVector(sendGridLoc, "sendLoc");
            printf("sendLinloc: %d\n", sendLinLoc);
            PrintVector(recvGridLoc, "recvLoc");
            printf("recvLinloc: %d\n", recvLinLoc);

            PrintArray(alignSendBuf, sendShape, "sendBuf to SendRecv");
            mpi::SendRecv(alignSendBuf, sendSize * nRedistProcs, sendLinLoc,
                          alignRecvBuf, sendSize * nRedistProcs, recvLinLoc, sendRecvComm);
            sendBuf = &(alignRecvBuf[0]);
            recvBuf = &(alignSendBuf[0]);
            PrintArray(alignRecvBuf, sendShape, "recvBuf from SendRecv");
        }

        mpi::AllToAll(sendBuf, sendSize, recvBuf, recvSize, comm);
        //Perform a send/recv to realign the data (if needed)

        PROFILE_STOP;

        if(!(Participating())){
            this->auxMemory_.Release();
            return;
        }

        ObjShape recvShape = commDataShape;
        recvShape.insert(recvShape.end(), nRedistProcs);
        PrintArray(recvBuf, recvShape, "recvBuf");

        //Unpack the data (if participating)
        PROFILE_SECTION("A2AUnpack");
        PROFILE_FLOPS(prod(MaxLocalShape()));
        UnpackA2ACommRecvBuf(recvBuf, commModes, commDataShape, A);
        PROFILE_STOP;

        const T* myBuf = LockedBuffer();
        PrintArray(myBuf, LocalShape(), LocalStrides(), "myBuf");

        this->auxMemory_.Release();
}

template <typename T>
void DistTensor<T>::PackA2ACommSendBuf(const DistTensor<T>& A, const ModeArray& commModes, const ObjShape& sendShape, T * const sendBuf){
    Unsigned i,j;
    const Unsigned order = A.Order();
    const T* dataBuf = A.LockedBuffer();

    const Location zeros(order, 0);
    const Location ones(order, 1);

    //GridView information
    const tmen::GridView gvA = A.GetGridView();
    const tmen::GridView gvB = GetGridView();
    const ObjShape gvAShape = gvA.ParticipatingShape();
    const ObjShape gvBShape = gvB.ParticipatingShape();

    //Different striding information
    const std::vector<Unsigned> commLCMs = LCMs(gvAShape, gvBShape);
    const std::vector<Unsigned> modeStrideFactor = ElemwiseDivide(commLCMs, gvAShape);

    const Unsigned nElemsPerProc = prod(sendShape);

    //Grid information
    const tmen::Grid& g = Grid();
    const ObjShape gridShape = g.Shape();
    const Location myGridLoc = g.Loc();

    const Unsigned nRedistProcsAll = prod(FilterVector(gridShape, commModes));

    //Redistribute information
    ModeArray sortedCommModes = commModes;
    std::sort(sortedCommModes.begin(), sortedCommModes.end());
    const ObjShape commShape = FilterVector(gridShape, sortedCommModes);

    //For each process we send to, we need to determine the first element we need to send them
    PARALLEL_FOR
    for(i = 0; i < nRedistProcsAll; i++){

        //Invert the process order based on the communicator used, to the actual process location
        Location sortedCommLoc = LinearLoc2Loc(i, commShape);
        Location procGridLoc = myGridLoc;

        for(j = 0; j < sortedCommModes.size(); j++){
            procGridLoc[sortedCommModes[j]] = sortedCommLoc[j];
        }

        //This is the first element p_i needs using A's alignment for B.
        Location procGridViewLocB = GridLoc2ParticipatingGridViewLoc(procGridLoc, gridShape, TensorDist());
        Location firstElemOwnerA = GridViewLoc2GridLoc(A.Alignments(), gvA);
        Location alignAinB = ElemwiseMod(GridLoc2ParticipatingGridViewLoc(firstElemOwnerA, gridShape, TensorDist()), ModeStrides());
        Location procFirstElemLoc = DetermineFirstUnalignedElem(procGridViewLocB, alignAinB);
        //Determine where to pack this information as B's alignment may differ from A's (meaning we pack data in different process order).
        Location firstElemOwnerB = GridViewLoc2GridLoc(Alignments(), gvB);
        std::vector<Unsigned> alignDiff = ElemwiseSubtract(firstElemOwnerA, firstElemOwnerB);

        Location adjustedProcGridLoc = ElemwiseMod(ElemwiseSum(ElemwiseSubtract(procGridLoc, alignDiff), gridShape), gridShape);
        Location adjustedProcGridLocSlice = FilterVector(adjustedProcGridLoc, sortedCommModes);
        Unsigned adjustedProcLinLoc = Loc2LinearLoc(adjustedProcGridLocSlice, FilterVector(gridShape, sortedCommModes));
        printf("i: %d adjustedLoc: %d\n", i, adjustedProcLinLoc);
        PrintVector(sortedCommModes, "sortedModes");
        PrintVector(g.Loc(), "myLoc");
        PrintVector(procGridLoc, "procGridLoc");
        PrintVector(procFirstElemLoc, "sendBuf first elem is");


        //Determine the first element I need to send to p_i
        //The first element I own is
        Location myFirstLoc = A.DetermineFirstElem(A.GetGridView().ParticipatingLoc());


        //Iterate to figure out the first elem I need to send p_i
        Location firstSendLoc(order,-1);

        bool found = true;
        for(j = 0; j < myFirstLoc.size(); j++){
            Unsigned myFirstIndex = myFirstLoc[j];
            Unsigned sendFirstIndex = procFirstElemLoc[j];
            Unsigned myModeStride = A.ModeStride(j);
            Unsigned sendProcModeStride = ModeStride(j);

            while(myFirstIndex != sendFirstIndex && myFirstIndex < Dimension(j)){
                if(myFirstIndex < sendFirstIndex)
                    myFirstIndex += myModeStride;
                else
                    sendFirstIndex += sendProcModeStride;
            }
            if(myFirstIndex >= Dimension(j)){
                found &= false;
                break;
            }
            firstSendLoc[j] = myFirstIndex;
        }

        if(found)
            PrintVector(firstSendLoc, "haha sending first loc");
        else
            PrintVector(firstSendLoc, "nope sending first loc");
        //Pack the data if we need to send data to p_i
        if(found && ElemwiseLessThan(firstSendLoc, Shape())){
            //Determine where the initial piece of data is located.
            const Location localLoc = A.Global2LocalIndex(firstSendLoc);
            Unsigned dataBufPtr = LinearLocFromStrides(PermuteVector(localLoc, A.localPerm_), A.LocalStrides());

            PackData packData;
            packData.loopShape = ElemwiseSubtract(A.LocalShape(), PermuteVector(localLoc, A.localPerm_));
            packData.srcBufStrides = ElemwiseProd(A.LocalStrides(), PermuteVector(modeStrideFactor, A.localPerm_));

            //Pack into permuted form to minimize striding when unpacking
            ObjShape finalShape = PermuteVector(sendShape, localPerm_);
            std::vector<Unsigned> finalStrides = Dimensions2Strides(finalShape);

            //Determine permutation from local output to local input
            Permutation out2in = DetermineInversePermutation(DeterminePermutation(A.localPerm_, localPerm_));

            //Permute pack strides to match input local permutation (for correct packing)
            packData.dstBufStrides = PermuteVector(finalStrides, out2in);

            packData.loopStarts = zeros;
            //ModeStrideFactor is global information, we need to permute it to match locally
            packData.loopIncs = PermuteVector(modeStrideFactor, localPerm_);

            PackCommHelper(packData, order - 1, &(dataBuf[dataBufPtr]), &(sendBuf[adjustedProcLinLoc * nElemsPerProc]));
        }
    }
}

template<typename T>
void DistTensor<T>::UnpackA2ACommRecvBuf(const T * const recvBuf, const ModeArray& commModes, const ObjShape& recvShape, const DistTensor<T>& A){
    Unsigned i, j;
    const Unsigned order = A.Order();
    T* dataBuf = Buffer();

    const Location zeros(order, 0);
    const Location ones(order, 1);

    //GridView information
    const tmen::GridView gvA = A.GetGridView();
    const tmen::GridView gvB = GetGridView();
    const ObjShape gvAShape = gvA.ParticipatingShape();
    const ObjShape gvBShape = gvB.ParticipatingShape();

    //Different striding information
    std::vector<Unsigned> commLCMs = tmen::LCMs(gvAShape, gvBShape);
    std::vector<Unsigned> modeStrideFactor = ElemwiseDivide(commLCMs, gvBShape);

    const Unsigned nElemsPerProc = prod(recvShape);

    //Grid information
    const tmen::Grid& g = Grid();
    const ObjShape gridShape = g.Shape();
    const Location myGridLoc = g.Loc();

    const Unsigned nRedistProcsAll = prod(FilterVector(gridShape, commModes));

    //Redistribute information
    ModeArray sortedCommModes = commModes;
    std::sort(sortedCommModes.begin(), sortedCommModes.end());
    const ObjShape commShape = FilterVector(gridShape, sortedCommModes);

    //For each process we recv from, we need to determine the first element we get from them
    PARALLEL_FOR
    for(i = 0; i < nRedistProcsAll; i++){

        //Invert the process order based on the communicator used, to the actual process location
        Location sortedCommLoc = LinearLoc2Loc(i, commShape);
        Location procGridLoc = myGridLoc;

        for(j = 0; j < sortedCommModes.size(); j++){
            procGridLoc[sortedCommModes[j]] = sortedCommLoc[j];
        }

        //This is the first element p_i owns using B's alignment for A.
        Location procGridViewLocA = GridLoc2ParticipatingGridViewLoc(procGridLoc, gridShape, A.TensorDist());
        Location firstElemOwnerB = GridViewLoc2GridLoc(Alignments(), gvB);
        Location alignBinA = ElemwiseMod(GridLoc2ParticipatingGridViewLoc(firstElemOwnerB, gridShape, A.TensorDist()), A.ModeStrides());
        Location procFirstElemLoc = A.DetermineFirstUnalignedElem(procGridViewLocA, alignBinA);
        //Determine where to unpack this information from as B's alignment may differ from A's (meaning we packed data in different process order).
        Location firstElemOwnerA = GridViewLoc2GridLoc(A.Alignments(), gvA);
        std::vector<Unsigned> alignDiff = ElemwiseSubtract(firstElemOwnerB, firstElemOwnerA);

        Location adjustedProcGridLoc = ElemwiseMod(ElemwiseSubtract(procGridLoc, alignDiff), gridShape);
        Location adjustedProcGridLocSlice = FilterVector(adjustedProcGridLoc, sortedCommModes);
        Unsigned adjustedProcLinLoc = Loc2LinearLoc(adjustedProcGridLocSlice, FilterVector(gridShape, sortedCommModes));
//        printf("i: %d adjustedLoc: %d\n", i, adjustedProcLinLoc);
//        PrintVector(sortedCommModes, "sortedModes");
//        PrintVector(g.Loc(), "myLoc");
//        PrintVector(procGridLoc, "procGridLoc");
//        PrintVector(procFirstElemLoc, "sendBuf first elem is");

        //Determine the first element I need from p_i
        //The first element I own is
        Location myFirstLoc = DetermineFirstElem(GetGridView().ParticipatingLoc());

        //Iterate to figure out the first elem I need from p_i
        Location firstRecvLoc(order,-1);

        bool found = true;
        for(j = 0; j < myFirstLoc.size(); j++){
            Unsigned myFirstIndex = myFirstLoc[j];
            Unsigned recvFirstIndex = procFirstElemLoc[j];
            Unsigned myModeStride = ModeStride(j);
            Unsigned recvProcModeStride = A.ModeStride(j);

            while(myFirstIndex != recvFirstIndex && myFirstIndex < Dimension(j)){
                if(myFirstIndex < recvFirstIndex)
                    myFirstIndex += myModeStride;
                else
                    recvFirstIndex += recvProcModeStride;
            }
            if(myFirstIndex >= Dimension(j)){
                found &= false;
                break;
            }
            firstRecvLoc[j] = myFirstIndex;
        }

        //Unpack the data if we need to recv data from p_i
        if(found && ElemwiseLessThan(firstRecvLoc, Shape())){
            //Determine where to place the initial piece of data.
            const Location localLoc = Global2LocalIndex(firstRecvLoc);
            Unsigned dataBufPtr = LinearLocFromStrides(PermuteVector(localLoc, localPerm_), LocalStrides());

            PackData unpackData;
            unpackData.loopShape = ElemwiseSubtract(LocalShape(), PermuteVector(localLoc, localPerm_));
            unpackData.dstBufStrides = ElemwiseProd(LocalStrides(), PermuteVector(modeStrideFactor, localPerm_));

            //Recv data is permuted the same way our local data is permuted
            ObjShape actualRecvShape = PermuteVector(recvShape, localPerm_);
            unpackData.srcBufStrides = Dimensions2Strides(actualRecvShape);

            unpackData.loopStarts = zeros;
            //ModeStrideFactor is global information, we need to permute it to match locally
            unpackData.loopIncs = PermuteVector(modeStrideFactor, localPerm_);

            PackCommHelper(unpackData, order - 1, &(recvBuf[i * nElemsPerProc]), &(dataBuf[dataBufPtr]));
        }
    }
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
FULL(std::complex<float>);
#endif
FULL(std::complex<double>);
#endif

} //namespace tmen
