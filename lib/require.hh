/**
 * TODO: Update requirment of power of two size using concept and requires
 */
static constexpr bool is_power_of_two(size_t V) {
    return V && ((V & (V - 1)) == 0);
}

template <typename T>
concept power_of_two = std::integral<T> && requires(T value) {
    { is_power_of_2(value) } -> std::convertible_to<bool>;
    { get_bit_mask(value) } -> std::convertible_to<size_t>;
    { get_value(value) } -> std::convertible_to<size_t>;
};

template <power_of_two T>
constexpr size_t get_bit_mask(T t) {
    return (size_t)t - 1;
}

template <power_of_two T>
constexpr size_t get_value(T t) {
    return static_cast<size_t>(t);
}