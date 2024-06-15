#pragma once

#include <atomic>
#include <cassert>
#include <memory>

// TODO: Optimise for requires
template<auto V>
constexpr bool is_power_of_two = V && ((V & (V - 1)) == 0);


/// Threadsafe but flawed circular FIFO
template<typename T, const int N, typename Alloc = std::allocator<T>> requires is_power_of_two<N>
class BasicSPSCWithoutModulo : private Alloc
{
public:
    using value_type = T;
    using allocator_traits = std::allocator_traits<Alloc>;
    using size_type = typename allocator_traits::size_type;

    static constexpr int bit_mask = N - 1; 

    explicit BasicSPSCWithoutModulo(Alloc const& alloc = Alloc{})
        : Alloc{alloc}
        , capacity_{N}
        , ring_{allocator_traits::allocate(*this, N)}
    {}

    ~BasicSPSCWithoutModulo() {
        while(not empty()) {
            ring_[getPopCursor()].~T();
            ++popCursor_;
        }
        allocator_traits::deallocate(*this, ring_, capacity_);
    }

    /// Returns the number of elements in the fifo
    inline bool size() const noexcept {
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
        ring_[getPushCursor()] = value; 
        ++pushCursor_;
        return true;
    }

    /// Pop one object from the fifo.
    /// @return `true` if the pop operation is successful; `false` if fifo is empty.
    bool pop(T& value) {
        if (empty()) {
            return false;
        }
        const int index = getPopCursor();
        value = std::move(ring_[index]);
        ++popCursor_;
        return true;
    }

private:

    using CursorType = std::atomic<size_type>;
    static_assert(CursorType::is_always_lock_free);

    size_type capacity_;
    T* ring_;

    inline int getPushCursor() {
        return pushCursor_ & bit_mask;
    }

    inline int getPopCursor() {
        return popCursor_ & bit_mask;
    }

    /// Loaded and stored by the push thread; loaded by the pop thread
    CursorType pushCursor_;

    /// Loaded and stored by the pop thread; loaded by the push thread
    CursorType popCursor_;
};