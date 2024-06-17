#include "BasicSPSC.hh"
#include "BasicSPSCWithoutModulo.hh"

#include <gtest/gtest.h>

#include <type_traits>


extern "C" {
    void __ubsan_on_report() {
          FAIL() << "Encountered an undefined behavior sanitizer error";
    }
    void __asan_on_error() {
          FAIL() << "Encountered an address sanitizer error";
    }
    void __tsan_on_report() {
          FAIL() << "Encountered a thread sanitizer error";
    }
}  // extern "C"


template<typename FifoT>
class FifoTestBase : public testing::Test {
public:
    using FifoType = FifoT;
    using value_type = typename FifoType::value_type;

    FifoType fifo;
};

using test_type = unsigned int;

template<typename FifoT> using FifoTest = FifoTestBase<FifoT>;
using FifoTypes = ::testing::Types<
    BasicSPSC<test_type, 4>,
    BasicSPSCWithoutModulo<test_type, 4>
    >;
TYPED_TEST_SUITE(FifoTest, FifoTypes);

// TYPED_TEST(FifoTest, properties) {
//     EXPECT_FALSE(std::is_default_constructible_v<typename TestFixture::FifoType>);
//     EXPECT_TRUE((std::is_constructible_v<typename TestFixture::FifoType, unsigned long>));
//     EXPECT_TRUE((std::is_constructible_v<typename TestFixture::FifoType, unsigned long, std::allocator<typename TestFixture::value_type>>));
//     EXPECT_FALSE(std::is_copy_constructible_v<typename TestFixture::FifoType>);
//     EXPECT_FALSE(std::is_move_constructible_v<typename TestFixture::FifoType>);
//     EXPECT_FALSE(std::is_copy_assignable_v<typename TestFixture::FifoType>);
//     EXPECT_FALSE(std::is_move_assignable_v<typename TestFixture::FifoType>);
//     EXPECT_TRUE(std::is_destructible_v<typename TestFixture::FifoType>);
// }

TYPED_TEST(FifoTest, initialConditions) {
    EXPECT_EQ(4u, this->fifo.capacity());
    EXPECT_EQ(0, this->fifo.size());
    EXPECT_TRUE(this->fifo.empty());
    EXPECT_FALSE(this->fifo.full());
}

TYPED_TEST(FifoTest, push) {
    ASSERT_EQ(4u, this->fifo.capacity());

    EXPECT_TRUE(this->fifo.push(42));
    EXPECT_EQ(1u, this->fifo.size());
    EXPECT_FALSE(this->fifo.empty());
    EXPECT_FALSE(this->fifo.full());

    EXPECT_TRUE(this->fifo.push(42));
    EXPECT_EQ(2u, this->fifo.size());
    EXPECT_FALSE(this->fifo.empty());
    EXPECT_FALSE(this->fifo.full());

    EXPECT_TRUE(this->fifo.push(42));
    EXPECT_EQ(3u, this->fifo.size());
    EXPECT_FALSE(this->fifo.empty());
    EXPECT_FALSE(this->fifo.full());

    EXPECT_TRUE(this->fifo.push(42));
    EXPECT_EQ(4u, this->fifo.size());
    EXPECT_FALSE(this->fifo.empty());
    EXPECT_TRUE(this->fifo.full());

    EXPECT_FALSE(this->fifo.push(42));
    EXPECT_EQ(4u, this->fifo.size());
    EXPECT_FALSE(this->fifo.empty());
    EXPECT_TRUE(this->fifo.full());
}

TYPED_TEST(FifoTest, pop) {
    auto value = typename TestFixture::value_type{};
    EXPECT_FALSE(this->fifo.pop(value));

    for (auto i = 0u; i < this->fifo.capacity(); ++i) {
        this->fifo.push(42 + i);
    }

    for (auto i = 0u; i < this->fifo.capacity(); ++i) {
        EXPECT_EQ(this->fifo.capacity() - i, this->fifo.size());
        auto value = typename TestFixture::value_type{};
        EXPECT_TRUE(this->fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }
    EXPECT_EQ(0, this->fifo.size());
    EXPECT_TRUE(this->fifo.empty());
    EXPECT_FALSE(this->fifo.pop(value));
}

TYPED_TEST(FifoTest, popFullFifo) {
    auto value = typename TestFixture::value_type{};
    EXPECT_FALSE(this->fifo.pop(value));

    for (auto i = 0u; i < this->fifo.capacity(); ++i) {
        this->fifo.push(42 + i);
    }
    EXPECT_TRUE(this->fifo.full());

    for (auto i = 0u; i < this->fifo.capacity()*4; ++i) {
        EXPECT_TRUE(this->fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    EXPECT_FALSE(this->fifo.full());

        EXPECT_TRUE(this->fifo.push(42 + 4 + i));
        EXPECT_TRUE(this->fifo.full());
    }
}

TYPED_TEST(FifoTest, popEmpty) {
    auto value = typename TestFixture::value_type{};
    EXPECT_FALSE(this->fifo.pop(value));

    for (auto i = 0u; i < this->fifo.capacity()*4; ++i) {
        EXPECT_TRUE(this->fifo.empty());
        EXPECT_TRUE(this->fifo.push(42 + i));
        EXPECT_TRUE(this->fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }

    EXPECT_TRUE(this->fifo.empty());
    EXPECT_FALSE(this->fifo.pop(value));
}

TYPED_TEST(FifoTest, wrap) {
    auto value = typename TestFixture::value_type{};
    for (auto i = 0u; i < this->fifo.capacity() * 2 + 1; ++i) {
        this->fifo.push(42 + i);
        EXPECT_TRUE(this->fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }

    for (auto i = 0u; i < 8u; ++i) {
        this->fifo.push(42 + i);
        EXPECT_TRUE(this->fifo.pop(value));
        EXPECT_EQ(42 + i, value);
    }
}