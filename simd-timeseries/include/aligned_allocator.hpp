#pragma once
#include <cstdlib> // for posix_memalign
#include <new>     // for std::bad_alloc
#include <limits>  // for std::numeric_limits

/**
 * A standard-compliant allocator that ensures memory is aligned to N bytes.
 * Default alignment is 32 bytes (AVX2 width).
 */
template <typename T, std::size_t Alignment = 32>
struct AlignedAllocator {
    using value_type = T;

    // Tell compiler how to convert AlignedAllocator<T>
    // to AlignedAllocator<U>
    template <typename U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };

    // 1. Default constructor
    AlignedAllocator() noexcept = default;

    // 2. Copy constructor for type U (boilerplate for STL containers)
    template <typename U>
    AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

    // 3. Allocate: The core logic
    T* allocate(std::size_t n) {
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

    // 4. Deallocate: Free the memory
    void deallocate(T* p, std::size_t) noexcept {
        free(p);
    }
};

// Boilerplate comparison operators
template <typename T, std::size_t AlignT, typename U, std::size_t AlignU>
bool operator==(const AlignedAllocator<T, AlignT>&, const AlignedAllocator<U, AlignU>&) { 
    return AlignT == AlignU; 
}

template <typename T, std::size_t AlignT, typename U, std::size_t AlignU>
bool operator!=(const AlignedAllocator<T, AlignT>&, const AlignedAllocator<U, AlignU>&) { 
    return AlignT != AlignU; 
}