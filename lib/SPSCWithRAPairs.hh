#pragma once

#include <atomic>
#include <cassert>
#include <memory>

/// Threadsafe but flawed circular FIFO
template<typename T, const int N = 1 << 17, typename Alloc = std::allocator<T>>
class SPSCWithRAPairs : private Alloc
{
public:
    using value_type = T;
    using allocator_traits = std::allocator_traits<Alloc>;
    using size_type = typename allocator_traits::size_type;


    explicit SPSCWithRAPairs(Alloc const& alloc = Alloc{})
        : Alloc{alloc}
        , capacity_{N}
        , ring_{allocator_traits::allocate(*this, N)}
    {}

    ~SPSCWithRAPairs() {
        while(not empty()) {
            ring_[popCursor_ & bit_mask].~T();
            ++popCursor_;
        }
        allocator_traits::deallocate(*this, ring_, capacity_);
    }

    /// Returns the number of elements in the fifo
    inline size_type size() const noexcept {
        size_type pushCursor = pushCursor_.load(std::memory_order_relaxed);
        size_type popCursor = popCursor_.load(std::memory_order_relaxed);

        return pushCursor - popCursor;
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
        size_type pushCur = pushCursor_.load(std::memory_order_relaxed);
        size_type popCur = popCursor_.load(std::memory_order_acquire);
        if (full(pushCur, popCur)) {
            return false;
        }
        new (&ring_[pushCur & bit_mask]) T(value);
        pushCursor_.store(pushCur + 1, std::memory_order_release);
        return true;
    }

    /// Pop one object from the fifo.
    /// @return `true` if the pop operation is successful; `false` if fifo is empty.
    bool pop(T& value) {
        size_type popCur = popCursor_.load(std::memory_order_relaxed);
        size_type pushCur = pushCursor_.load(std::memory_order_acquire);
        if (empty(pushCur, popCur)) {
            return false;
        }
        value = *element(popCur);
        element(popCur)->~T();
        popCursor_.store(popCur + 1, std::memory_order_release);
        return true;
    }

private:
    inline size_type full(size_type &pushCursor, size_type &popCursor) const noexcept {
        return (pushCursor - popCursor) == capacity_;
    }
    inline bool empty(size_type &pushCursor, size_type &popCursor) noexcept {
        return pushCursor == popCursor;
    }
    T* element(size_type &cursor) noexcept {
        return &ring_[cursor & bit_mask];
    }

private:

    static constexpr int bit_mask = N - 1; 

    using CursorType = std::atomic<size_type>;
    static_assert(CursorType::is_always_lock_free);

    size_type capacity_;
    T* ring_;


    /// Loaded and stored by the push thread; loaded by the pop thread
    CursorType pushCursor_;

    /// Loaded and stored by the pop thread; loaded by the push thread
    CursorType popCursor_;
};