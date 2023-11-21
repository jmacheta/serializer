#include <ecpp/serial.hpp>

using namespace ecpp;

// #include <iostream>
struct foo
{
    char a;
    int b[3];

    using fields = serializable_fields<&foo::a, &foo::b>;
};

// std::span<std::byte> ecpp::serializer<class foo, std::byte>::serialize(
//     foo const& v, std::span<std::byte> ar, opts o) noexcept {
//     return {};
// }

struct bar
{
    unsigned a;
    unsigned b;
    foo c;
    unsigned d;
    uint8_t e[2];
    std::vector<uint8_t> f{4, 5, 6, 7};

    using fields = serializable_fields<&bar::a, &bar::b, &bar::d, &bar::c,
                                       &bar::e, &bar::f>;
};

[[gnu::noinline]] auto do_serialize(auto const &x, std::span<std::byte> ar)
{
    return serialize(x, ar);
}

auto do_serialize2(std::span<std::byte> ar, bar const &x)
{
    auto aro = ar;

    std::memcpy(ar.data(), &x.a, sizeof(x.a));
    ar = ar.subspan(sizeof(x.a));

    std::memcpy(ar.data(), &x.b, sizeof(x.b));
    ar = ar.subspan(sizeof(x.b));

    std::memcpy(ar.data(), &x.d, sizeof(x.d));
    ar = ar.subspan(sizeof(x.d));

    std::memcpy(ar.data(), &x.c.a, sizeof(x.c.a));
    ar = ar.subspan(sizeof(x.c.a));

    std::memcpy(ar.data(), &x.c.b, sizeof(x.c.b));
    ar = ar.subspan(sizeof(x.c.b));

    std::memcpy(ar.data(), &x.e, sizeof(x.e));
    ar = ar.subspan(sizeof(x.e));
    return aro.first(aro.size() - ar.size());
}

int main()
{
    bar aaa{0xAABBCCDD, 0x11223344, {1, 2}, 0x88887777};

    foo ff{1, {2, 3}};

    int f1[16]{};
    // f1[0].push_back(0xAABBCCDD);
    // f1[0].push_back(0x11223344);
    // f1[0].push_back(0xFFEEDDCC);
    // f1[1].push_back(0x55667788);

    std::array<uint8_t, 400> ar{};
    auto sp = std::as_writable_bytes(std::span<uint8_t>{ar});

    auto serialized = do_serialize(f1, sp);
    // std::cout << "sizex: " << serialized.size() << "\n";
    // for (auto r : serialized) {
    //     std::cout << std::hex << (unsigned)r << ", ";
    // }
    // std::cout << "\n";

    return serialized.size();
}
