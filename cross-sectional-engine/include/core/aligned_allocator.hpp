#pragma once
#include <cstdlib> // for posix_memalign
#include <new>     // for std::bad_alloc
#include <limits>  // for std::numeric_limits

// 32-byte aligned allocator for AVX2
template <typename T, std::size_t Alignment = 32>
struct AlignedAllocator {
    static_assert(Alignment >= sizeof(void*) && (Alignment & (Alignment - 1)) == 0,
                  "Alignment must be a power of 2 and >= sizeof(void*)");
    
    using value_type = T;

    template <typename U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };

    AlignedAllocator() noexcept = default;

    template <typename U>
    AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
            throw std::bad_alloc();
        }

        void* ptr = nullptr;
        if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0) {
            throw std::bad_alloc();
        }

        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t) noexcept {
        free(p);
    }
};

template <typename T, std::size_t AlignT, typename U, std::size_t AlignU>
bool operator==(const AlignedAllocator<T, AlignT>&, const AlignedAllocator<U, AlignU>&) noexcept { 
    return AlignT == AlignU; 
}

template <typename T, std::size_t AlignT, typename U, std::size_t AlignU>
bool operator!=(const AlignedAllocator<T, AlignT>&, const AlignedAllocator<U, AlignU>&) noexcept { 
    return AlignT != AlignU; 
}