#include <ecpp/serializer.hpp>
#include <gtest/gtest.h>


TEST(Serializer, FundamentalTypesSerialization) {
    using namespace ecpp;

    std::uint8_t  v8  = 0x11;
    std::uint16_t v16 = 0x2233;
    std::uint32_t v32 = 0x44556677;
    std::uint64_t v64 = 0x8899AABBCCDDEEFF;

    EXPECT_TRUE(serializer<std::uint8_t>::has_fixed_size);
    EXPECT_EQ(serializer<std::uint8_t>::fixed_size, sizeof(std::uint8_t));
    EXPECT_TRUE(serializer<std::uint16_t>::has_fixed_size);
    EXPECT_EQ(serializer<std::uint16_t>::fixed_size, sizeof(std::uint16_t));
    EXPECT_TRUE(serializer<std::uint32_t>::has_fixed_size);
    EXPECT_EQ(serializer<std::uint32_t>::fixed_size, sizeof(std::uint32_t));
    EXPECT_TRUE(serializer<std::uint64_t>::has_fixed_size);
    EXPECT_EQ(serializer<std::uint64_t>::fixed_size, sizeof(std::uint64_t));


    std::span<std::byte> empty{};
    EXPECT_EQ(serialize(v8, empty).data(), nullptr);
    EXPECT_EQ(serialize(v16, empty).data(), nullptr);
    EXPECT_EQ(serialize(v32, empty).data(), nullptr);
    EXPECT_EQ(serialize(v64, empty).data(), nullptr);


    std::array<std::byte, 1> too_small{};
    EXPECT_EQ(serialize(v16, too_small).data(), nullptr);
    EXPECT_EQ(serialize(v32, too_small).data(), nullptr);
    EXPECT_EQ(serialize(v64, too_small).data(), nullptr);


    std::array<std::byte, 15> just_right{};
    just_right.fill(std::byte{0xFE});

    auto buf = std::span<std::byte>(just_right);

    auto res8 = serialize(v8, buf);
    EXPECT_NE(res8.data(), nullptr);
    EXPECT_EQ(buf[0], std::byte(0x11));
    buf = buf.subspan(sizeof(v8));

    auto res16 = serialize(v16, buf);
    EXPECT_NE(res16.data(), nullptr);
    EXPECT_EQ(buf[0], std::byte(0x33));
    EXPECT_EQ(buf[1], std::byte(0x22));
    buf = buf.subspan(sizeof(v16));

    auto res32 = serialize(v32, buf);
    EXPECT_NE(res32.data(), nullptr);
    EXPECT_EQ(buf[0], std::byte(0x77));
    EXPECT_EQ(buf[1], std::byte(0x66));
    EXPECT_EQ(buf[2], std::byte(0x55));
    EXPECT_EQ(buf[3], std::byte(0x44));
    buf = buf.subspan(sizeof(v32));

    auto res64 = serialize(v64, buf);
    EXPECT_NE(res64.data(), nullptr);
    EXPECT_EQ(buf[0], std::byte(0xFF));
    EXPECT_EQ(buf[1], std::byte(0xEE));
    EXPECT_EQ(buf[2], std::byte(0xDD));
    EXPECT_EQ(buf[3], std::byte(0xCC));
    EXPECT_EQ(buf[4], std::byte(0xBB));
    EXPECT_EQ(buf[5], std::byte(0xAA));
    EXPECT_EQ(buf[6], std::byte(0x99));
    EXPECT_EQ(buf[7], std::byte(0x88));
    buf = buf.subspan(sizeof(v64));
}