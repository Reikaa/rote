/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "tensormental.hpp"
#include "tensormental/util/vec_util.hpp"

namespace tmen {

//
// Assertions
//

template<typename T>
void
Tensor<T>::AssertValidDimensions( const ObjShape& shape ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::AssertValidDimensions");
#endif
}

template<typename T>
void
Tensor<T>::AssertValidDimensions( const ObjShape& shape, const std::vector<Unsigned>& ldims ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::AssertValidDimensions");
#endif
    AssertValidDimensions( shape );
    if(shape.size() != ldims.size())
        LogicError("shape order must match ldims order");
    if( !ElemwiseLessThan(shape, ldims) )
        LogicError("Leading dimensions must be no less than dimensions");
    if( AnyZeroElem(ldims) )
        LogicError("Leading dimensions cannot be zero (for BLAS compatibility)");
}

template<typename T>
void
Tensor<T>::AssertValidEntry( const Location& loc ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::AssertValidEntry");
#endif
    const Unsigned order = this->Order();
    if(order != loc.size())
        LogicError("Index must be of same order as object");

     if( !ElemwiseLessThan(loc, shape_) )
    {
        Unsigned i;
        std::ostringstream msg;
        msg << "Out of bounds: "
            << "(";
        if(order > 0)
            msg << loc[0];
        for(i = 1; i < loc.size(); i++)
            msg << ", " << loc[i];
        msg << ") of ";
        if(loc.size() > 0)
            msg << loc[0];
        for(i = 1; i < order; i++)
            msg << " x " << shape_[i];
        msg << "Tensor.";
            LogicError( msg.str() );
    }
}

template<typename T>
void
Tensor<T>::AssertMergeableModes(const std::vector<ModeArray>& oldModes) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::AssertMergeableModes");
#endif
    Unsigned i, j;
    for(i = 0; i < oldModes.size(); i++){
        for(j = 0; j < oldModes[i].size(); j++){
            if(oldModes[i][j] >= Order())
                LogicError("Specified mode out of range");
        }
    }
}

template<typename T>
void
Tensor<T>::AssertSplittableModes(const ModeArray& oldModes, const std::vector<ObjShape>& newShape) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::AssertSplittableIndices");
#endif
    Unsigned i;

    //NOTE: FIX THIS FOR SCALARS
//    for(i = 0; i < oldModes.size(); i++){
//        if(prod(newShape[i]) != Dimension(oldModes[i])){
//            LogicError("newShape dimensions must be splittable from old mode dimension");
//        }
//    }
}

//
// Constructors
//

template<typename T>
Tensor<T>::Tensor( bool fixed )
: shape_(), strides_(), ldims_(),
  viewType_( fixed ? OWNER_FIXED : OWNER ),
  data_(nullptr), memory_()
{ data_ = memory_.Buffer(); }

template<typename T>
Tensor<T>::Tensor( const Unsigned order, bool fixed )
: shape_(order, 0), strides_(order, 1), ldims_(order, 1),
  viewType_( fixed ? OWNER_FIXED : OWNER ),
  data_(nullptr), memory_()
{ data_ = memory_.Buffer(); }

//TODO: Check for valid set of indices
template<typename T>
Tensor<T>::Tensor( const ObjShape& shape, bool fixed )
: shape_(shape), strides_(Dimensions2Strides(shape)), ldims_(shape.size(), 1),
  viewType_( fixed ? OWNER_FIXED : OWNER )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor");
    AssertValidDimensions( shape );
#endif
    SetLDims(shape_);
    const Unsigned order = this->Order();
    Unsigned numElem = order > 0 ? ldims_[order-1] * shape_[order-1] : 1;

    memory_.Require( numElem );
    data_ = memory_.Buffer();
}

//TODO: Check for valid set of indices
template<typename T>
Tensor<T>::Tensor
( const ObjShape& shape, const std::vector<Unsigned>& ldims, bool fixed )
: shape_(shape), strides_(Dimensions2Strides(shape)), ldims_(ldims),
  viewType_( fixed ? OWNER_FIXED : OWNER )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor");
    AssertValidDimensions( shape, ldims );
#endif
    SetLDims(shape);
    const Unsigned order = this->Order();
    Unsigned numElem = order > 0 ? ldims_[order-1] * shape_[order-1] : 1;

    memory_.Require( numElem );
    data_ = memory_.Buffer();
}

//TODO: Check for valid set of indices
template<typename T>
Tensor<T>::Tensor
( const ObjShape& shape, const T* buffer, const std::vector<Unsigned>& ldims, bool fixed )
: shape_(shape), strides_(Dimensions2Strides(shape)), ldims_(ldims),
  viewType_( fixed ? LOCKED_VIEW_FIXED: LOCKED_VIEW ),
  data_(buffer), memory_()
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor");
    AssertValidDimensions( shape, ldims );
#endif
}

//TODO: Check for valid set of indices
template<typename T>
Tensor<T>::Tensor
( const ObjShape& shape, T* buffer, const std::vector<Unsigned>& ldims, bool fixed )
: shape_(shape), strides_(Dimensions2Strides(shape)), ldims_(ldims),
  viewType_( fixed ? VIEW_FIXED: VIEW ),
  data_(buffer), memory_()
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor");
    AssertValidDimensions( shape, ldims );
#endif
}

template<typename T>
Tensor<T>::Tensor( const Tensor<T>& A )
: shape_(), strides_(), ldims_(),
  viewType_( OWNER ),
  data_(nullptr), memory_()
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor( const Tensor& )");
#endif
    if( &A != this )
        *this = A;
    else
        LogicError("You just tried to construct a Tensor with itself!");
}

template<typename T>
void
Tensor<T>::Swap( Tensor<T>& A )
{
    std::swap( shape_, A.shape_ );
    std::swap( strides_, A.strides_ );
    std::swap( ldims_, A.ldims_ );
    std::swap( viewType_, A.viewType_ );
    std::swap( data_, A.data_ );
    memory_.Swap( A.memory_ );
}

//
// Destructor
//

template<typename T>
Tensor<T>::~Tensor()
{ }

//
// Basic information
//

template<typename T>
void
Tensor<T>::SetLDims(const ObjShape& shape)
{
  Unsigned i;
  const int order = this->Order();
  if(shape.size() != order){
      LogicError("SetLDims requires that shape order matches object order");
  }
  if(order > 0){
    ldims_[0] = 1;
    for(i = 1; i < order; i++)
      ldims_[i] = ldims_[i-1]*shape[i-1];
  }
}

template<typename T>
Unsigned
Tensor<T>::Order() const
{
	return shape_.size();
}

template<typename T>
ObjShape
Tensor<T>::Shape() const
{
	return shape_;
}

template<typename T>
Unsigned
Tensor<T>::Dimension(Mode mode) const
{ 
  const Unsigned order = this->Order();
  if( mode >= order){
    LogicError("Requested mode dimension out of range.");
    return 0;
  }

  return shape_[mode];
}

template<typename T>
Unsigned
Tensor<T>::ModeStride(Mode mode) const
{
    const Unsigned order = this->Order();
    if( mode >= order){
      LogicError("Requested mode dimension out of range.");
      return 0;
    }

    return strides_[mode];
}

template<typename T>
std::vector<Unsigned>
Tensor<T>::LDims() const
{
    return ldims_;
}

template<typename T>
Unsigned
Tensor<T>::LDim(Mode mode) const
{ 
  const Unsigned order = this->Order();
  if( mode >= order){
    LogicError("Requested mode leading dimension out of range.");
    return 0;
  }else
    return ldims_[mode]; 
}

template<typename T>
Unsigned
Tensor<T>::MemorySize() const
{ return memory_.Size(); }

template<typename T>
bool
Tensor<T>::Owner() const
{ return IsOwner( viewType_ ); }

template<typename T>
bool
Tensor<T>::Viewing() const
{ return !IsOwner( viewType_ ); }

template<typename T>
bool
Tensor<T>::Shrinkable() const
{ return IsShrinkable( viewType_ ); }

template<typename T>
bool
Tensor<T>::FixedSize() const
{ return !IsShrinkable( viewType_ ); }

template<typename T>
bool
Tensor<T>::Locked() const
{ return IsLocked( viewType_ ); }

template<typename T>
T*
Tensor<T>::Buffer()
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Buffer");
    if( Locked() )
        LogicError("Cannot return non-const buffer of locked Tensor");
#endif
    // NOTE: This const_cast has been carefully considered and should be safe
    //       since the underlying data should be non-const if this is called.
    return const_cast<T*>(data_);
}

template<typename T>
Unsigned
Tensor<T>::LinearOffset(const Location& loc) const
{
  Unsigned i;
  const Unsigned order = this->Order();
  if(loc.size() != order){
      LogicError("index must be of same order as tensor");
  }
  Unsigned offset = 0;
  for(i = 0; i < order; i++)
    offset += loc[i] * strides_[i];
  return offset;
}

template<typename T>
const T*
Tensor<T>::LockedBuffer() const
{ return data_; }

template<typename T>
T*
Tensor<T>::Buffer( const Location& loc )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Buffer");
    if( Locked() )
        LogicError("Cannot return non-const buffer of locked Tensor");
#endif
    // NOTE: This const_cast has been carefully considered and should be safe
    //       since the underlying data should be non-const if this is called.
    Unsigned linearOffset = LinearOffset(loc);
    return &const_cast<T*>(data_)[linearOffset];
}

template<typename T>
const T*
Tensor<T>::LockedBuffer( const Location& loc ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::LockedBuffer");
#endif
    Unsigned linearOffset = LinearOffset(loc);
    return &data_[linearOffset];
}

//
// Unit mode info
//

template<typename T>
void
Tensor<T>::RemoveUnitModes(const ModeArray& modes){
#ifndef RELEASE
    CallStackEntry cse("Tensor::LockedBuffer");
#endif
    Unsigned i;
    ModeArray sorted = modes;
    std::sort(sorted.begin(), sorted.end());
    for(i = sorted.size() - 1; i < sorted.size(); i--){
        shape_.erase(shape_.begin() + sorted[i]);
        strides_.erase(strides_.begin() + sorted[i]);
        ldims_.erase(ldims_.begin() + sorted[i]);
    }
}
//
//template<typename T>
//void
//Tensor<T>::RemoveUnitMode(const Mode& mode){
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::RemoveUnitMode");
//#endif
//    shape_.erase(shape_.begin() + mode);
//    strides_.erase(strides_.begin() + mode);
//    ldims_.erase(ldims_.begin() + mode);
//}
//
//template<typename T>
//void
//Tensor<T>::IntroduceUnitMode(const Unsigned& modePosition)
//{
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::IntroduceUnitMode");
//#endif
//    shape_.insert(shape_.begin() + modePosition, 1);
//    strides_.insert(strides_.begin() + modePosition, strides_[modePosition]);
//    ldims_.insert(ldims_.begin() + modePosition, ldims_[modePosition]);
//}

//
// Entry manipulation
//

template<typename T>
const T&
Tensor<T>::Get_( const Location& loc ) const
{ 
    Unsigned linearOffset = LinearOffset(loc);
    return data_[linearOffset]; 
}

template<typename T>
T&
Tensor<T>::Set_( const Location& loc )
{
    // NOTE: This const_cast has been carefully considered and should be safe
    //       since the underlying data should be non-const if this is called.
    Unsigned linearOffset = LinearOffset(loc);
    return (const_cast<T*>(data_))[linearOffset];
}

template<typename T>
T
Tensor<T>::Get( const Location& loc ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Get");
    AssertValidEntry( loc );
#endif
    return Get_( loc );
}

template<typename T>
void
Tensor<T>::Set( const Location& loc, T alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Set");
    AssertValidEntry( loc );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    Set_( loc ) = alpha;
}

template<typename T>
void
Tensor<T>::Update( const Location& loc, T alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Update");
    AssertValidEntry( loc );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    Set_( loc ) += alpha;
}

//template<typename T>
//void
//Tensor<T>::GetDiagonal( Tensor<T>& d, Int offset ) const
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::GetDiagonal");
//    if( d.Locked() )
//        LogicError("d must not be a locked view");
//#endif
//    const Int diagLength = DiagonalLength(offset);
//    d.ResizeTo( diagLength, 1 );
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            d.Set_( j, 0 ) = Get_(j,j+offset);
//    else
//        for( Int j=0; j<diagLength; ++j )
//            d.Set_( j, 0 ) = Get_(j-offset,j);
//}

//template<typename T>
//Tensor<T>
//Tensor<T>::GetDiagonal( Int offset ) const
//{ 
//    Tensor<T> d;
//    GetDiagonal( d, offset );
//    return d;
//}

//template<typename T>
//void
//Tensor<T>::SetDiagonal( const Tensor<T>& d, Int offset )
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::SetDiagonal");
//    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
//        LogicError("d is not a column-vector of the right length");
//#endif
//    const Int diagLength = DiagonalLength(offset);
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            Set_( j, j+offset ) = d.Get_(j,0);
//    else
//        for( Int j=0; j<diagLength; ++j )
//            Set_( j-offset, j ) = d.Get_(j,0);
//}
//
//template<typename T>
//void
//Tensor<T>::UpdateDiagonal( const Tensor<T>& d, Int offset )
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::UpdateDiagonal");
//    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
//        LogicError("d is not a column-vector of the right length");
//#endif
//    const Int diagLength = DiagonalLength(offset);
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            Set_( j, j+offset ) += d.Get(j,0);
//    else
//        for( Int j=0; j<diagLength; ++j )
//            Set_( j-offset, j ) += d.Get(j,0);
//}

template<typename T>
void
Tensor<T>::ComplainIfReal() const
{ 
    if( !IsComplex<T>::val )
        LogicError("Called complex-only routine with real data");
}

template<typename T>
BASE(T)
Tensor<T>::GetRealPart( const Location& loc ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::GetRealPart");
    AssertValidEntry( loc );
#endif
    return tmen::RealPart( Get_( loc ) );
}

template<typename T>
BASE(T)
Tensor<T>::GetImagPart( const Location& loc ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::GetImagPart");
    AssertValidEntry( loc );
#endif
    return tmen::ImagPart( Get_( loc ) );
}

template<typename T>
void 
Tensor<T>::SetRealPart( const Location& loc, BASE(T) alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::SetRealPart");
    AssertValidEntry( loc );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    tmen::SetRealPart( Set_( loc ), alpha );
}

template<typename T>
void 
Tensor<T>::SetImagPart( const Location& loc, BASE(T) alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::SetImagPart");
    AssertValidEntry( loc );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    ComplainIfReal();
    tmen::SetImagPart( Set_( loc ), alpha );
}

template<typename T>
void 
Tensor<T>::UpdateRealPart( const Location& loc, BASE(T) alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::UpdateRealPart");
    AssertValidEntry( loc );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    tmen::UpdateRealPart( Set_( loc ), alpha );
}

template<typename T>
void 
Tensor<T>::UpdateImagPart( const Location& loc, BASE(T) alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::UpdateImagPart");
    AssertValidEntry( loc );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    ComplainIfReal();
    tmen::UpdateImagPart( Set_( loc ), alpha );
}
   
//template<typename T>
//void
//Tensor<T>::GetRealPartOfDiagonal( Tensor<BASE(T)>& d, Int offset ) const
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::GetRealPartOfDiagonal");
//    if( d.Locked() )
//        LogicError("d must not be a locked view");
//#endif
//    const Int diagLength = DiagonalLength(offset);
//    d.ResizeTo( diagLength, 1 );
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            d.Set_( j, 0 ) = tmen::RealPart( Get_(j,j+offset) );
//    else
//        for( Int j=0; j<diagLength; ++j )
//            d.Set_( j, 0 ) = tmen::RealPart( Get_(j-offset,j) );
//}
//
//template<typename T>
//void
//Tensor<T>::GetImagPartOfDiagonal( Tensor<BASE(T)>& d, Int offset ) const
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::GetImagPartOfDiagonal");
//    if( d.Locked() )
//        LogicError("d must not be a locked view");
//#endif
//    const Int diagLength = DiagonalLength(offset);
//    d.ResizeTo( diagLength, 1 );
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            d.Set_( j, 0 ) = tmen::ImagPart( Get_(j,j+offset) );
//    else
//        for( Int j=0; j<diagLength; ++j )
//            d.Set_( j, 0 ) = tmen::ImagPart( Get_(j-offset,j) );
//}
//
//template<typename T>
//Tensor<BASE(T)>
//Tensor<T>::GetRealPartOfDiagonal( Int offset ) const
//{ 
//    Tensor<BASE(T)> d;
//    GetRealPartOfDiagonal( d, offset );
//    return d;
//}
//
//template<typename T>
//Tensor<BASE(T)>
//Tensor<T>::GetImagPartOfDiagonal( Int offset ) const
//{ 
//    Tensor<BASE(T)> d;
//    GetImagPartOfDiagonal( d, offset );
//    return d;
//}
//
//template<typename T>
//void
//Tensor<T>::SetRealPartOfDiagonal( const Tensor<BASE(T)>& d, Int offset )
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::SetRealPartOfDiagonal");
//    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
//        LogicError("d is not a column-vector of the right length");
//#endif
//    const Int diagLength = DiagonalLength(offset);
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            tmen::SetRealPart( Set_(j,j+offset), d.Get_(j,0) );
//    else
//        for( Int j=0; j<diagLength; ++j )
//            tmen::SetRealPart( Set_(j-offset,j), d.Get_(j,0) );
//}
//
//template<typename T>
//void
//Tensor<T>::SetImagPartOfDiagonal( const Tensor<BASE(T)>& d, Int offset )
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::SetImagPartOfDiagonal");
//    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
//        LogicError("d is not a column-vector of the right length");
//#endif
//    ComplainIfReal();
//    const Int diagLength = DiagonalLength(offset);
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            tmen::SetImagPart( Set_(j,j+offset), d.Get_(j,0) );
//    else
//        for( Int j=0; j<diagLength; ++j )
//            tmen::SetImagPart( Set_(j-offset,j), d.Get_(j,0) );
//}
//
//template<typename T>
//void
//Tensor<T>::UpdateRealPartOfDiagonal( const Tensor<BASE(T)>& d, Int offset )
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::UpdateRealPartOfDiagonal");
//    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
//        LogicError("d is not a column-vector of the right length");
//#endif
//    const Int diagLength = DiagonalLength(offset);
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            tmen::UpdateRealPart( Set_(j,j+offset), d.Get_(j,0) );
//    else
//        for( Int j=0; j<diagLength; ++j )
//            tmen::UpdateRealPart( Set_(j-offset,j), d.Get_(j,0) );
//}
//
//template<typename T>
//void
//Tensor<T>::UpdateImagPartOfDiagonal( const Tensor<BASE(T)>& d, Int offset )
//{ 
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::UpdateImagPartOfDiagonal");
//    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
//        LogicError("d is not a column-vector of the right length");
//#endif
//    ComplainIfReal();
//    const Int diagLength = DiagonalLength(offset);
//    if( offset >= 0 )
//        for( Int j=0; j<diagLength; ++j )
//            tmen::UpdateImagPart( Set_(j,j+offset), d.Get_(j,0) );
//    else
//        for( Int j=0; j<diagLength; ++j )
//            tmen::UpdateImagPart( Set_(j-offset,j), d.Get_(j,0) );
//}
//
//template<typename T>
//void
//Tensor<T>::Control_( Int height, Int width, T* buffer, Int ldim )
//{
//    memory_.Empty();
//    height_ = height;
//    width_ = width;
//    ldims_ = ldim;
//    data_ = buffer;
//    viewType_ = (ViewType)( viewType_ & ~LOCKED_VIEW );
//}
//
//template<typename T>
//void
//Tensor<T>::Control( Int height, Int width, T* buffer, Int ldim )
//{
//#ifndef RELEASE
//    CallStackEntry cse("Tensor::Control");
//    if( FixedSize() )
//        LogicError( "Cannot attach a new buffer to a view with fixed size" );
//#endif
//    Control_( height, width, buffer, ldim );
//}

template<typename T>
void
Tensor<T>::Attach_( const ObjShape& shape, T* buffer, const std::vector<Unsigned>& ldims )
{
    memory_.Empty();
    shape_ = shape;
    ldims_ = ldims;
    data_ = buffer;
    viewType_ = (ViewType)( ( viewType_ & ~LOCKED_OWNER ) | VIEW );
}

template<typename T>
void
Tensor<T>::Attach( const ObjShape& shape, T* buffer, const std::vector<Unsigned>& ldims )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Attach");
    if( FixedSize() )
        LogicError( "Cannot attach a new buffer to a view with fixed size" );
#endif
    Attach_( shape, buffer, ldims );
}

template<typename T>
void
Tensor<T>::LockedAttach_( const ObjShape& shape, const T* buffer, const std::vector<Unsigned>& ldims )
{
    memory_.Empty();
    shape_ = shape;
    ldims_ = ldims;
    data_ = buffer;
    viewType_ = (ViewType)( viewType_ | VIEW );
}

template<typename T>
void
Tensor<T>::LockedAttach
( const ObjShape& shape, const T* buffer, const std::vector<Unsigned>& ldims )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::LockedAttach");
    if( FixedSize() )
        LogicError( "Cannot attach a new buffer to a view with fixed size" );
#endif
    LockedAttach_( shape, buffer, ldims );
}

//
// Utilities
//

template<typename T>
const Tensor<T>&
Tensor<T>::operator=( const Tensor<T>& A )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::operator=");
    if( Locked() )
        LogicError("Cannot assign to a locked view");
    if( viewType_ != OWNER && AnyElemwiseNotEqual(A.shape_, shape_) )
        LogicError
        ("Cannot assign to a view of different dimensions");
#endif
    if( viewType_ == OWNER )
        ResizeTo( A );
    T* dst = this->Buffer();
    const T* src = A.LockedBuffer();
    //Only copy single element if we know this is a scalar
    if(this->Order() == 0){
        MemCopy(&(dst[0]), &(src[0]), 1);
    }
    //Otherwise check if 0 tensor
    else{
        MemCopy(&(dst[0]), &(src[0]), prod(shape_));
    }

    return *this;
}

template<typename T>
void
Tensor<T>::Empty_()
{
    std::fill(shape_.begin(), shape_.end(), 0);
    std::fill(strides_.begin(), strides_.end(), 0);
    std::fill(ldims_.begin(), ldims_.end(), 0);

    viewType_ = (ViewType)( viewType_ & ~LOCKED_VIEW );

    data_ = nullptr;
    memory_.Empty();
}

template<typename T>
void
Tensor<T>::Empty()
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Empty()");
    if ( FixedSize() )
        LogicError("Cannot empty a fixed-size matrix" );
#endif
    Empty_();
}

template<typename T>
void
Tensor<T>::ResizeTo_( const ObjShape& shape )
{
	//TODO: Implement general stride
	bool reallocate = AnyElemwiseGreaterThan(shape, shape_);
	shape_ = shape;
	if(reallocate){
		ldims_ = Dimensions2Strides(shape);
		strides_ = Dimensions2Strides(shape);
		memory_.Require(Max(1,prod(shape)));
		data_ = memory_.Buffer();
	}
    //TODO: IMPLEMENT CORRECTLY
//    bool reallocate = height > ldims_ || width > width_;
//    height_ = height;
//    width_ = width;
//    // Only change the ldim when necessary. Simply 'shrink' our view if 
//    // possible.
//    if( reallocate )
//    {
//        ldims_ = Max( height, 1 );
//        memory_.Require( ldims_ * width );
//        data_ = memory_.Buffer();
//    }
}

template<typename T>
void
Tensor<T>::ResizeTo( const Tensor<T>& A )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::ResizeTo(Tensor)");
    AssertValidDimensions( A.Shape() );

    if ( FixedSize() && AnyElemwiseNotEqual(A.shape_, shape_) )
        LogicError("Cannot change the size of this tensor");
    if ( Viewing() && AnyElemwiseNotEqual(A.shape_, shape_) )
        LogicError("Cannot increase the size of this tensor");
#endif
    ResizeTo(A.shape_);
}

template<typename T>
void
Tensor<T>::ResizeTo( const ObjShape& shape )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::ResizeTo(dimensions)");
    AssertValidDimensions( shape );

    if ( FixedSize() && AnyElemwiseNotEqual(shape, shape_) )
        LogicError("Cannot change the size of this tensor");
    if ( Viewing() && AnyElemwiseNotEqual(shape, shape_) )
        LogicError("Cannot increase the size of this tensor");
#endif
    ResizeTo_( shape );
}

template<typename T>
void
Tensor<T>::ResizeTo_( const ObjShape& shape, const std::vector<Unsigned>& ldims )
{
	//TODO: Implement general stride
	bool reallocate = AnyElemwiseGreaterThan(shape, shape_) || AnyElemwiseGreaterThan(ldims, ldims_);
	shape_ = shape;
	if(reallocate){
		ldims_ = ldims;
		memory_.Require(Max(1,prod(shape)));
		data_ = memory_.Buffer();
	}
    //TODO: IMPLEMENT CORRECTLY
//    bool reallocate = height > ldims_ || width > width_ || ldim != ldims_;
//    height_ = height;
//    width_ = width;
//    if( reallocate )
//    {
//        ldims_ = ldim;
//        memory_.Require(ldim*width);
//        data_ = memory_.Buffer();
//    }
}

template<typename T>
void
Tensor<T>::ResizeTo( const ObjShape& shape, const std::vector<Unsigned>& ldims )
{
    //TODO: IMPLEMENT CORRECTLY
#ifndef RELEASE
    CallStackEntry cse("Tensor::ResizeTo(dims,ldims)");
    AssertValidDimensions( shape, ldims );
    if( FixedSize() &&
        ( AnyElemwiseNotEqual(shape, shape_) || AnyElemwiseNotEqual(ldims, ldims_) ) )
        LogicError("Cannot change the size of this tensor");
    if( Viewing() && ( AnyElemwiseNotEqual(shape, shape_) || AnyElemwiseNotEqual(ldims, ldims_) ) )
        LogicError("Cannot increase the size of this matrix");
#endif
    ResizeTo_( shape, ldims );
}

template class Tensor<Int>;
#ifndef DISABLE_FLOAT
template class Tensor<float>;
#endif // ifndef DISABLE_FLOAT
template class Tensor<double>;
#ifndef DISABLE_COMPLEX
#ifndef DISABLE_FLOAT
template class Tensor<Complex<float> >;
#endif // ifndef DISABLE_FLOAT
template class Tensor<Complex<double> >;
#endif // ifndef DISABLE_COMPLEX

} // namespace tmen
