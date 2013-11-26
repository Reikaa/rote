/*
   Copyright (c) 2009-2013, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "tensormental.hpp"

namespace elem {

//
// Assertions
//

template<typename T>
void
Tensor<T>::AssertValidDimensions( Int height, Int width ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::AssertValidDimensions");
#endif
    if( height < 0 || width < 0 )
        LogicError("Height and width must be non-negative");
}

template<typename T>
void
Tensor<T>::AssertValidDimensions( Int height, Int width, Int ldim ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::AssertValidDimensions");
#endif
    AssertValidDimensions( height, width );
    if( ldim < height )
        LogicError("Leading dimension must be no less than height");
    if( ldim == 0 )
        LogicError("Leading dimension cannot be zero (for BLAS compatibility)");
}

template<typename T>
void
Tensor<T>::AssertValidEntry( Int i, Int j ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::AssertValidEntry");
#endif
    if( i < 0 || j < 0 )
        LogicError("Indices must be non-negative");
    if( i > this->Height() || j > this->Width() )
    {
        std::ostringstream msg;
        msg << "Out of bounds: "
            << "(" << i << "," << j << ") of " << this->Height()
            << " x " << this->Width() << " Tensor.";
        LogicError( msg.str() );
    }
}

//
// Constructors
//

template<typename T>
Tensor<T>::Tensor( bool fixed )
: viewType_( fixed ? OWNER_FIXED : OWNER ),
  height_(0), width_(0), ldim_(1), 
  data_(nullptr)
{ }

template<typename T>
Tensor<T>::Tensor( Int height, Int width, bool fixed )
: viewType_( fixed ? OWNER_FIXED : OWNER ),
  height_(height), width_(width), ldim_(Max(height,1))
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor");
    AssertValidDimensions( height, width );
#endif
    memory_.Require( ldim_ * width );
    data_ = memory_.Buffer();
    // TODO: Consider explicitly zeroing
}

template<typename T>
Tensor<T>::Tensor
( Int height, Int width, Int ldim, bool fixed )
: viewType_( fixed ? OWNER_FIXED : OWNER ),
  height_(height), width_(width), ldim_(ldim)
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor");
    AssertValidDimensions( height, width, ldim );
#endif
    memory_.Require( ldim*width );
    data_ = memory_.Buffer();
}

template<typename T>
Tensor<T>::Tensor
( Int height, Int width, const T* buffer, Int ldim, bool fixed )
: viewType_( fixed ? LOCKED_VIEW_FIXED: LOCKED_VIEW ),
  height_(height), width_(width), ldim_(ldim), 
  data_(buffer)
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor");
    AssertValidDimensions( height, width, ldim );
#endif
}

template<typename T>
Tensor<T>::Tensor
( Int height, Int width, T* buffer, Int ldim, bool fixed )
: viewType_( fixed ? VIEW_FIXED: VIEW ),
  height_(height), width_(width), ldim_(ldim), 
  data_(buffer)
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Tensor");
    AssertValidDimensions( height, width, ldim );
#endif
}

template<typename T>
Tensor<T>::Tensor( const Tensor<T>& A )
: viewType_( OWNER ),
  height_(0), width_(0), ldim_(1), 
  data_(nullptr)
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
Tensor<T>::Tensor( Tensor<T>&& A )
: viewType_(A.viewType_),
  height_(A.height_), width_(A.width_), ldim_(A.ldim_),
  data_(nullptr), memory_(std::move(A.memory_))
{ std::swap( data_, A.data_ ); }

template<typename T>
Tensor<T>&
Tensor<T>::operator=( Tensor<T>&& A )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::operator=( Tensor&& )");
#endif
    if( this == &A )
        LogicError("Tried to move to self");
    memory_.Swap( A.memory_ );
    std::swap( data_, A.data_ );
    viewType_ = A.viewType_;
    height_ = A.height_;
    width_ = A.width_;
    ldim_ = A.ldim_;

    return *this;
}

template<typename T>
void
Tensor<T>::Swap( Tensor<T>& A )
{
    memory_.Swap( A.memory_ );
    std::swap( data_, A.data_ );
    std::swap( viewType_, A.viewType_ );
    std::swap( height_, A.height_ );
    std::swap( width_, A.width_ );
    std::swap( ldim_, A.ldim_ );
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
Int 
Tensor<T>::Height() const
{ return height_; }

template<typename T>
Int
Tensor<T>::Width() const
{ return width_; }

template<typename T>
Int
Tensor<T>::DiagonalLength( Int offset ) const
{ return 0;/*return elem::DiagonalLength(height_,width_,offset);*/ }

template<typename T>
Int
Tensor<T>::LDim() const
{ return ldim_; }

template<typename T>
Int
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
const T*
Tensor<T>::LockedBuffer() const
{ return data_; }

template<typename T>
T*
Tensor<T>::Buffer( Int i, Int j )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Buffer");
    if( Locked() )
        LogicError("Cannot return non-const buffer of locked Tensor");
#endif
    // NOTE: This const_cast has been carefully considered and should be safe
    //       since the underlying data should be non-const if this is called.
    return &const_cast<T*>(data_)[i+j*ldim_];
}

template<typename T>
const T*
Tensor<T>::LockedBuffer( Int i, Int j ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::LockedBuffer");
#endif
    return &data_[i+j*ldim_];
}

//
// Entry manipulation
//

template<typename T>
const T&
Tensor<T>::Get_( Int i, Int j ) const
{ return data_[i+j*ldim_]; }

template<typename T>
T&
Tensor<T>::Set_( Int i, Int j ) 
{
    // NOTE: This const_cast has been carefully considered and should be safe
    //       since the underlying data should be non-const if this is called.
    return (const_cast<T*>(data_))[i+j*ldim_];
}

template<typename T>
T
Tensor<T>::Get( Int i, Int j ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Get");
    AssertValidEntry( i, j );
#endif
    return Get_( i, j );
}

template<typename T>
void
Tensor<T>::Set( Int i, Int j, T alpha ) 
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Set");
    AssertValidEntry( i, j );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    Set_( i, j ) = alpha;
}

template<typename T>
void
Tensor<T>::Update( Int i, Int j, T alpha ) 
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Update");
    AssertValidEntry( i, j );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    Set_( i, j ) += alpha;
}

template<typename T>
void
Tensor<T>::GetDiagonal( Tensor<T>& d, Int offset ) const
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::GetDiagonal");
    if( d.Locked() )
        LogicError("d must not be a locked view");
#endif
    const Int diagLength = DiagonalLength(offset);
    d.ResizeTo( diagLength, 1 );
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            d.Set_( j, 0 ) = Get_(j,j+offset);
    else
        for( Int j=0; j<diagLength; ++j )
            d.Set_( j, 0 ) = Get_(j-offset,j);
}

template<typename T>
Tensor<T>
Tensor<T>::GetDiagonal( Int offset ) const
{ 
    Tensor<T> d;
    GetDiagonal( d, offset );
    return d;
}

template<typename T>
void
Tensor<T>::SetDiagonal( const Tensor<T>& d, Int offset )
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::SetDiagonal");
    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
        LogicError("d is not a column-vector of the right length");
#endif
    const Int diagLength = DiagonalLength(offset);
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            Set_( j, j+offset ) = d.Get_(j,0);
    else
        for( Int j=0; j<diagLength; ++j )
            Set_( j-offset, j ) = d.Get_(j,0);
}

template<typename T>
void
Tensor<T>::UpdateDiagonal( const Tensor<T>& d, Int offset )
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::UpdateDiagonal");
    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
        LogicError("d is not a column-vector of the right length");
#endif
    const Int diagLength = DiagonalLength(offset);
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            Set_( j, j+offset ) += d.Get(j,0);
    else
        for( Int j=0; j<diagLength; ++j )
            Set_( j-offset, j ) += d.Get(j,0);
}

template<typename T>
void
Tensor<T>::ComplainIfReal() const
{ 
    if( !IsComplex<T>::val )
        LogicError("Called complex-only routine with real data");
}

template<typename T>
BASE(T)
Tensor<T>::GetRealPart( Int i, Int j ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::GetRealPart");
    AssertValidEntry( i, j );
#endif
    return elem::RealPart( Get_( i, j ) );
}

template<typename T>
BASE(T)
Tensor<T>::GetImagPart( Int i, Int j ) const
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::GetImagPart");
    AssertValidEntry( i, j );
#endif
    return elem::ImagPart( Get_( i, j ) );
}

template<typename T>
void 
Tensor<T>::SetRealPart( Int i, Int j, BASE(T) alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::SetRealPart");
    AssertValidEntry( i, j );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    elem::SetRealPart( Set_( i, j ), alpha );
}

template<typename T>
void 
Tensor<T>::SetImagPart( Int i, Int j, BASE(T) alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::SetImagPart");
    AssertValidEntry( i, j );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    ComplainIfReal();
    elem::SetImagPart( Set_( i, j ), alpha );
}

template<typename T>
void 
Tensor<T>::UpdateRealPart( Int i, Int j, BASE(T) alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::UpdateRealPart");
    AssertValidEntry( i, j );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    elem::UpdateRealPart( Set_( i, j ), alpha );
}

template<typename T>
void 
Tensor<T>::UpdateImagPart( Int i, Int j, BASE(T) alpha )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::UpdateImagPart");
    AssertValidEntry( i, j );
    if( Locked() )
        LogicError("Cannot modify data of locked matrices");
#endif
    ComplainIfReal();
    elem::UpdateImagPart( Set_( i, j ), alpha );
}
   
template<typename T>
void
Tensor<T>::GetRealPartOfDiagonal( Tensor<BASE(T)>& d, Int offset ) const
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::GetRealPartOfDiagonal");
    if( d.Locked() )
        LogicError("d must not be a locked view");
#endif
    const Int diagLength = DiagonalLength(offset);
    d.ResizeTo( diagLength, 1 );
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            d.Set_( j, 0 ) = elem::RealPart( Get_(j,j+offset) );
    else
        for( Int j=0; j<diagLength; ++j )
            d.Set_( j, 0 ) = elem::RealPart( Get_(j-offset,j) );
}

template<typename T>
void
Tensor<T>::GetImagPartOfDiagonal( Tensor<BASE(T)>& d, Int offset ) const
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::GetImagPartOfDiagonal");
    if( d.Locked() )
        LogicError("d must not be a locked view");
#endif
    const Int diagLength = DiagonalLength(offset);
    d.ResizeTo( diagLength, 1 );
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            d.Set_( j, 0 ) = elem::ImagPart( Get_(j,j+offset) );
    else
        for( Int j=0; j<diagLength; ++j )
            d.Set_( j, 0 ) = elem::ImagPart( Get_(j-offset,j) );
}

template<typename T>
Tensor<BASE(T)>
Tensor<T>::GetRealPartOfDiagonal( Int offset ) const
{ 
    Tensor<BASE(T)> d;
    GetRealPartOfDiagonal( d, offset );
    return d;
}

template<typename T>
Tensor<BASE(T)>
Tensor<T>::GetImagPartOfDiagonal( Int offset ) const
{ 
    Tensor<BASE(T)> d;
    GetImagPartOfDiagonal( d, offset );
    return d;
}

template<typename T>
void
Tensor<T>::SetRealPartOfDiagonal( const Tensor<BASE(T)>& d, Int offset )
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::SetRealPartOfDiagonal");
    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
        LogicError("d is not a column-vector of the right length");
#endif
    const Int diagLength = DiagonalLength(offset);
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            elem::SetRealPart( Set_(j,j+offset), d.Get_(j,0) );
    else
        for( Int j=0; j<diagLength; ++j )
            elem::SetRealPart( Set_(j-offset,j), d.Get_(j,0) );
}

template<typename T>
void
Tensor<T>::SetImagPartOfDiagonal( const Tensor<BASE(T)>& d, Int offset )
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::SetImagPartOfDiagonal");
    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
        LogicError("d is not a column-vector of the right length");
#endif
    ComplainIfReal();
    const Int diagLength = DiagonalLength(offset);
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            elem::SetImagPart( Set_(j,j+offset), d.Get_(j,0) );
    else
        for( Int j=0; j<diagLength; ++j )
            elem::SetImagPart( Set_(j-offset,j), d.Get_(j,0) );
}

template<typename T>
void
Tensor<T>::UpdateRealPartOfDiagonal( const Tensor<BASE(T)>& d, Int offset )
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::UpdateRealPartOfDiagonal");
    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
        LogicError("d is not a column-vector of the right length");
#endif
    const Int diagLength = DiagonalLength(offset);
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            elem::UpdateRealPart( Set_(j,j+offset), d.Get_(j,0) );
    else
        for( Int j=0; j<diagLength; ++j )
            elem::UpdateRealPart( Set_(j-offset,j), d.Get_(j,0) );
}

template<typename T>
void
Tensor<T>::UpdateImagPartOfDiagonal( const Tensor<BASE(T)>& d, Int offset )
{ 
#ifndef RELEASE
    CallStackEntry cse("Tensor::UpdateImagPartOfDiagonal");
    if( d.Height() != DiagonalLength(offset) || d.Width() != 1 )
        LogicError("d is not a column-vector of the right length");
#endif
    ComplainIfReal();
    const Int diagLength = DiagonalLength(offset);
    if( offset >= 0 )
        for( Int j=0; j<diagLength; ++j )
            elem::UpdateImagPart( Set_(j,j+offset), d.Get_(j,0) );
    else
        for( Int j=0; j<diagLength; ++j )
            elem::UpdateImagPart( Set_(j-offset,j), d.Get_(j,0) );
}

template<typename T>
void
Tensor<T>::Control_( Int height, Int width, T* buffer, Int ldim )
{
    memory_.Empty();
    height_ = height;
    width_ = width;
    ldim_ = ldim;
    data_ = buffer;
    viewType_ = (ViewType)( viewType_ & ~LOCKED_VIEW );
}

template<typename T>
void
Tensor<T>::Control( Int height, Int width, T* buffer, Int ldim )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Control");
    if( FixedSize() )
        LogicError( "Cannot attach a new buffer to a view with fixed size" );
#endif
    Control_( height, width, buffer, ldim );
}

template<typename T>
void
Tensor<T>::Attach_( Int height, Int width, T* buffer, Int ldim )
{
    memory_.Empty();
    height_ = height;
    width_ = width;
    ldim_ = ldim;
    data_ = buffer;
    viewType_ = (ViewType)( ( viewType_ & ~LOCKED_OWNER ) | VIEW );
}

template<typename T>
void
Tensor<T>::Attach( Int height, Int width, T* buffer, Int ldim )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::Attach");
    if( FixedSize() )
        LogicError( "Cannot attach a new buffer to a view with fixed size" );
#endif
    Attach_( height, width, buffer, ldim );
}

template<typename T>
void
Tensor<T>::LockedAttach_( Int height, Int width, const T* buffer, Int ldim )
{
    memory_.Empty();
    height_ = height;
    width_ = width;
    ldim_ = ldim;
    data_ = buffer;
    viewType_ = (ViewType)( viewType_ | VIEW );
}

template<typename T>
void
Tensor<T>::LockedAttach
( Int height, Int width, const T* buffer, Int ldim )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::LockedAttach");
    if( FixedSize() )
        LogicError( "Cannot attach a new buffer to a view with fixed size" );
#endif
    LockedAttach_( height, width, buffer, ldim );
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
    if( viewType_ != OWNER && (A.Height() != Height() || A.Width() != Width()) )
        LogicError
        ("Cannot assign to a view of different dimensions");
#endif
    if( viewType_ == OWNER )
        ResizeTo( A.Height(), A.Width() );
    const Int height = Height();
    const Int width = Width();
    const Int ldim = LDim();
    const Int ldimOfA = A.LDim();
    const T* src = A.LockedBuffer();
    T* dst = this->Buffer();
    PARALLEL_FOR
    for( Int j=0; j<width; ++j )
        MemCopy( &dst[j*ldim], &src[j*ldimOfA], height );
    return *this;
}

template<typename T>
void
Tensor<T>::Empty_()
{
    memory_.Empty();
    height_ = 0;
    width_ = 0;
    ldim_ = 1;
    data_ = nullptr;
    viewType_ = (ViewType)( viewType_ & ~LOCKED_VIEW );
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
Tensor<T>::ResizeTo_( Int height, Int width )
{
    bool reallocate = height > ldim_ || width > width_;
    height_ = height;
    width_ = width;
    // Only change the ldim when necessary. Simply 'shrink' our view if 
    // possible.
    if( reallocate )
    {
        ldim_ = Max( height, 1 );
        memory_.Require( ldim_ * width );
        data_ = memory_.Buffer();
    }
}

template<typename T>
void
Tensor<T>::ResizeTo( Int height, Int width )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::ResizeTo(height,width)");
    AssertValidDimensions( height, width );
    if ( FixedSize() && ( height != height_ || width != width_ ) )
        LogicError("Cannot change the size of this matrix");
    if ( Viewing() && ( height > height_ || width > width_ ) )
        LogicError("Cannot increase the size of this matrix");
#endif
    ResizeTo_( height, width );
}

template<typename T>
void
Tensor<T>::ResizeTo_( Int height, Int width, Int ldim )
{
    bool reallocate = height > ldim_ || width > width_ || ldim != ldim_;
    height_ = height;
    width_ = width;
    if( reallocate )
    {
        ldim_ = ldim;
        memory_.Require(ldim*width);
        data_ = memory_.Buffer();
    }
}

template<typename T>
void
Tensor<T>::ResizeTo( Int height, Int width, Int ldim )
{
#ifndef RELEASE
    CallStackEntry cse("Tensor::ResizeTo(height,width,ldim)");
    AssertValidDimensions( height, width, ldim );
    if( FixedSize() && 
        ( height != height_ || width != width_ || ldim != ldim_ ) )
        LogicError("Cannot change the size of this matrix");
    if( Viewing() && ( height > height_ || width > width_ || ldim != ldim_ ) )
        LogicError("Cannot increase the size of this matrix");
#endif
    ResizeTo_( height, width, ldim );
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

} // namespace elem
