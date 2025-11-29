#pragma once
#include <cstdlib>
#include <limits>
#include <new>

template <typename T, std::size_t Alignment = 32>
struct AlignedAllocator {
    using value_type = T;

    AlignedAllocator() noexcept = default;

    template<typename U>
    AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {};

    T* allocate(std::size_t n) {

        // checks that size_t can handles up to n * sizeof(T) 
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
            throw std::bad_alloc();
        }

        void* ptr = nullptr;
        // posix_memalign allocates 'n * sizeof(T)' bytes
        // placed at an address that is a multiple of 'Alignment'.
        // Returns 0 on success.
        if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0) {
            throw std::bad_alloc();
        }

        return static_cast<T*>(ptr);
    }    
};
