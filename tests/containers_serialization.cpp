#include <ecpp/serializer.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace ecpp;
using namespace testing;
TEST(Serializer, CArraySerialization) {
    std::uint32_t to_serialize[] = {0x11223344, 0x55667788, 0x99AABBCC};
    using type                   = std::remove_cvref_t<decltype(to_serialize)>;

    EXPECT_TRUE(ecpp::serializer<type>::has_fixed_size);
    EXPECT_EQ(ecpp::serializer<type>::fixed_size, sizeof(to_serialize));
    std::span<std::byte> empty{};

    EXPECT_EQ(serialize(to_serialize, empty).data(), nullptr);


    std::array<std::byte, 11> too_small{};
    EXPECT_EQ(serialize(to_serialize, too_small).data(), nullptr);
    EXPECT_THAT(too_small, Each(Eq(std::byte{0})));

    std::array<std::byte, 15> too_large{};

    auto remaining = serialize(to_serialize, too_large);
    EXPECT_NE(remaining.data(), nullptr);

    EXPECT_THAT(too_large,
                ElementsAre(std::byte(0x44),
                            std::byte(0x33),
                            std::byte(0x22),
                            std::byte(0x11),
                            std::byte(0x88),
                            std::byte(0x77),
                            std::byte(0x66),
                            std::byte(0x55),
                            std::byte(0xCC),
                            std::byte(0xBB),
                            std::byte(0xAA),
                            std::byte(0x99),
                            std::byte(0),
                            std::byte(0),
                            std::byte(0)));
}

TEST(Serializer, StdArraySerialization) {
    std::array<std::uint32_t, 3> to_serialize{0x11223344, 0x55667788, 0x99AABBCC};

    using type = std::remove_cvref_t<decltype(to_serialize)>;

    EXPECT_TRUE(ecpp::serializer<type>::has_fixed_size);
    EXPECT_EQ(ecpp::serializer<type>::fixed_size, sizeof(to_serialize));
    std::span<std::byte> empty{};

    EXPECT_EQ(serialize(to_serialize, empty).data(), nullptr);


    std::array<std::byte, 11> too_small{};
    EXPECT_EQ(serialize(to_serialize, too_small).data(), nullptr);
    EXPECT_THAT(too_small, Each(Eq(std::byte{0})));

    std::array<std::byte, 15> too_large{};

    auto remaining = serialize(to_serialize, too_large);
    EXPECT_NE(remaining.data(), nullptr);

    EXPECT_THAT(too_large,
                ElementsAre(std::byte(0x44),
                            std::byte(0x33),
                            std::byte(0x22),
                            std::byte(0x11),
                            std::byte(0x88),
                            std::byte(0x77),
                            std::byte(0x66),
                            std::byte(0x55),
                            std::byte(0xCC),
                            std::byte(0xBB),
                            std::byte(0xAA),
                            std::byte(0x99),
                            std::byte(0),
                            std::byte(0),
                            std::byte(0)));
}

TEST(Serializer, StdVectorSerialization) {
    std::vector<std::uint32_t> to_serialize{0x11223344, 0x55667788, 0x99AABBCC};

    using type = std::remove_cvref_t<decltype(to_serialize)>;

    EXPECT_FALSE(ecpp::serializer<type>::has_fixed_size);
    EXPECT_EQ(ecpp::serializer<type>::fixed_size, 0);
    std::span<std::byte> empty{};

    EXPECT_EQ(serialize(to_serialize, empty).data(), nullptr);


    std::array<std::byte, 11> too_small{};
    EXPECT_EQ(serialize(to_serialize, too_small).data(), nullptr);
    EXPECT_THAT(too_small, Each(Eq(std::byte{0})));

    std::array<std::byte, 15> too_large{};

    auto remaining = serialize(to_serialize, too_large);
    EXPECT_NE(remaining.data(), nullptr);

    EXPECT_THAT(too_large,
                ElementsAre(std::byte(0x44),
                            std::byte(0x33),
                            std::byte(0x22),
                            std::byte(0x11),
                            std::byte(0x88),
                            std::byte(0x77),
                            std::byte(0x66),
                            std::byte(0x55),
                            std::byte(0xCC),
                            std::byte(0xBB),
                            std::byte(0xAA),
                            std::byte(0x99),
                            std::byte(0),
                            std::byte(0),
                            std::byte(0)));

    too_large.fill(std::byte{0});

    to_serialize.push_back(0xDDEEFF11);
    EXPECT_EQ(serialize(to_serialize, too_large).data(), nullptr);

    std::array<std::byte, 16> just_right{};

    auto remaining2 = serialize(to_serialize, just_right);
    EXPECT_NE(remaining.data(), nullptr);

    EXPECT_THAT(just_right,
                ElementsAre(std::byte(0x44),
                            std::byte(0x33),
                            std::byte(0x22),
                            std::byte(0x11),
                            std::byte(0x88),
                            std::byte(0x77),
                            std::byte(0x66),
                            std::byte(0x55),
                            std::byte(0xCC),
                            std::byte(0xBB),
                            std::byte(0xAA),
                            std::byte(0x99),
                            std::byte(0x11),
                            std::byte(0xFF),
                            std::byte(0xEE),
                            std::byte(0xDD)));
}


TEST(Serializer, StdSpanSerialization) {
    std::vector<std::uint32_t> to_serialize_data{0x11223344, 0x55667788, 0x99AABBCC};

    std::span<std::uint32_t, 3> to_serialize_fixed{to_serialize_data};
    using t1 = std::remove_cvref_t<decltype(to_serialize_fixed)>;
    EXPECT_TRUE(ecpp::serializer<t1>::has_fixed_size);
    EXPECT_EQ(ecpp::serializer<t1>::fixed_size, to_serialize_fixed.size_bytes());

    std::span<std::byte> empty{};
    EXPECT_EQ(serialize(to_serialize_fixed, empty).data(), nullptr);

    std::array<std::byte, 11> too_small{};
    EXPECT_EQ(serialize(to_serialize_fixed, too_small).data(), nullptr);
    EXPECT_THAT(too_small, Each(Eq(std::byte{0})));

    std::array<std::byte, 15> too_large{};

    auto remaining = serialize(to_serialize_fixed, too_large);
    EXPECT_NE(remaining.data(), nullptr);

    EXPECT_THAT(too_large,
                ElementsAre(std::byte(0x44),
                            std::byte(0x33),
                            std::byte(0x22),
                            std::byte(0x11),
                            std::byte(0x88),
                            std::byte(0x77),
                            std::byte(0x66),
                            std::byte(0x55),
                            std::byte(0xCC),
                            std::byte(0xBB),
                            std::byte(0xAA),
                            std::byte(0x99),
                            std::byte(0),
                            std::byte(0),
                            std::byte(0)));


    std::span<std::uint32_t> to_serialize_dynamic{to_serialize_data};
    using t2 = std::remove_cvref_t<decltype(to_serialize_dynamic)>;
    EXPECT_FALSE(ecpp::serializer<t2>::has_fixed_size);
    EXPECT_EQ(ecpp::serializer<t2>::fixed_size, 0);

    EXPECT_EQ(serialize(to_serialize_dynamic, empty).data(), nullptr);

    EXPECT_EQ(serialize(to_serialize_dynamic, too_small).data(), nullptr);
    EXPECT_THAT(too_small, Each(Eq(std::byte{0})));

    std::array<std::byte, 12> just_right{};

    auto remaining2 = serialize(to_serialize_dynamic, just_right);
    EXPECT_NE(remaining2.data(), nullptr);

    EXPECT_THAT(just_right,
                ElementsAre(std::byte(0x44),
                            std::byte(0x33),
                            std::byte(0x22),
                            std::byte(0x11),
                            std::byte(0x88),
                            std::byte(0x77),
                            std::byte(0x66),
                            std::byte(0x55),
                            std::byte(0xCC),
                            std::byte(0xBB),
                            std::byte(0xAA),
                            std::byte(0x99)));
}
