#pragma once

#include <atomic>
#include <cassert>
#include <memory>


template<typename T, const int N = 1 << 17, typename Alloc = std::allocator<T>>
class BasicSPSC : private Alloc
{
public:
    using value_type = T;
    using allocator_traits = std::allocator_traits<Alloc>;
    using size_type = typename allocator_traits::size_type;

    explicit BasicSPSC(Alloc const& alloc = Alloc{})
        : Alloc{alloc}
        , capacity_{N}
        , ring_{allocator_traits::allocate(*this, N)}
    {}

    ~BasicSPSC() {
        while(not empty()) {
            ring_[popCursor_ % capacity_].~T();
            ++popCursor_;
        }
        allocator_traits::deallocate(*this, ring_, capacity_);
    }

    /// Returns the number of elements in the fifo
    inline size_type size() const noexcept {
        assert(popCursor_ <= pushCursor_);
        return pushCursor_ - popCursor_;
    }

    /// Returns whether the container has no elements
    inline bool empty() const noexcept { return size() == 0; }

    /// Returns whether the container has capacity_() elements
    inline bool full() const noexcept { return size() == capacity(); }

    /// Returns the number of elements that can be held in the fifo
    inline size_type capacity() const noexcept { return capacity_; }



    /// Push one object onto the fifo.
    /// @return `true` if the operation is successful; `false` if fifo is full.
    bool push(T const& value) {
        if (full()) {
            return false;
        }
        new (&ring_[pushCursor_ % capacity_]) T(value);
        ++pushCursor_;
        return true;
    }

    /// Pop one object from the fifo.
    /// @return `true` if the pop operation is successful; `false` if fifo is empty.
    bool pop(T& value) {
        if (empty()) {
            return false;
        }
        value = ring_[popCursor_ % capacity_];
        ring_[popCursor_ % capacity_].~T();
        ++popCursor_;
        return true;
    }

private:
    size_type capacity_;
    T* ring_;

    using CursorType = std::atomic<size_type>;
    static_assert(CursorType::is_always_lock_free);

    /// Loaded and stored by the push thread; loaded by the pop thread
    CursorType pushCursor_;

    /// Loaded and stored by the pop thread; loaded by the push thread
    CursorType popCursor_;
};