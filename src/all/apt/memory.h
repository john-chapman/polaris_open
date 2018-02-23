#pragma once

#include <apt/apt.h>

#include <cstdlib>
#include <cstring>

namespace apt {

// Allocate _size bytes with _align aligment.
void* malloc_aligned(uint _size, uint _align);

// Reallocate memory previously allocated via malloc_aligned (or allocate a new block if _p is null). The alignment of a previously 
// allocated block may not be changed.
void* realloc_aligned(void* _p, uint _size, uint _align);

// Free memory previous allocated via malloc_aligned().
void free_aligned(void* _p);


namespace internal {
	template <uint kAlignment> struct aligned_base;
		template<> struct alignas(1)   aligned_base<1>   {};
		template<> struct alignas(2)   aligned_base<2>   {};
		template<> struct alignas(4)   aligned_base<4>   {};
		template<> struct alignas(8)   aligned_base<8>   {};
		template<> struct alignas(16)  aligned_base<16>  {};
		template<> struct alignas(32)  aligned_base<32>  {};
		template<> struct alignas(64)  aligned_base<64>  {};
		template<> struct alignas(128) aligned_base<128> {};
}

////////////////////////////////////////////////////////////////////////////////
// aligned
// Mixin class, provides template-based memory alignment for deriving classes.
// Use cautiously, especially with multiple inheritance.
// Usage:
//
//    class Foo: public aligned<Foo, 16>
//    { // ...
//
// Alignment can only be increased. If the deriving class has a higher natural 
// alignment than kAlignment, the higher alignment is used.
////////////////////////////////////////////////////////////////////////////////
template <typename tType, uint kAlignment>
struct aligned: private internal::aligned_base<kAlignment>
{
	aligned()                                           { APT_STRICT_ASSERT((uint)this % kAlignment == 0); }

	// malloc_aligned is called with alignof(tType) which will be the min of the natural alignment of tType and kAlignment
	void* operator new(std::size_t _size)               { return malloc_aligned(_size, alignof(tType)); }
	void  operator delete(void* _ptr)                   { free_aligned(_ptr); }
	void* operator new[](std::size_t _size)             { return malloc_aligned(_size, alignof(tType)); }
	void  operator delete[](void* _ptr)                 { free_aligned(_ptr); }
	void* operator new(std::size_t _size, void* _ptr)   { APT_STRICT_ASSERT((uint)_ptr % kAlignment == 0); return _ptr; }
	void  operator delete(void*, void*)                 { ; } // dummy, matches placement new

};

////////////////////////////////////////////////////////////////////////////////
// storage
// Provides aligned storage for kCount objects of type tType. Suitable for 
// allocating static blocks of uninitialized memory for use with placement
// new.
////////////////////////////////////////////////////////////////////////////////
template <typename tType, uint kCount>
class storage: private aligned< storage<tType, kCount>, alignof(tType) >
{
	char m_buf[sizeof(tType) * kCount];
public:
	storage(): aligned< storage<tType, kCount>, alignof(tType) >() {}
	operator tType*() { return (tType*)m_buf; }
};

} // namespace apt
