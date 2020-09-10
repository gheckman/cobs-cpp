#include "cobs.h"

#include <iostream>
#include <iomanip>
#include <vector>

using buffer_t = std::vector<uint8_t>;

template <typename FwdIter>
void print_vector(FwdIter first, FwdIter last)
{
    std::ios_base::fmtflags f(std::cout.flags());
    int i = 0;
    std::cout << std::hex << std::setfill('0');
    for (; first != last; ++first)
    {
        std::cout << std::setw(2) << (uint32_t)*first;

        ++i;
        if (i < 16)
        {
            std::cout << ' ';
        }
        else
        {
            i = 0;
            std::cout << '\n';
        }
    }
    std::cout.flags(f);
}

buffer_t full_vector()
{
    buffer_t v;
    for (int i = 1; i <= 0xFF; ++i)
    {
        v.push_back(i);
    }
    return v;
}

bool test_encode(const buffer_t& input, const buffer_t& expected)
{
    buffer_t encoded;

    encoded.reserve(get_max_size(input.cbegin(), input.cend()));
    encode(input.cbegin(), input.cend(), std::back_inserter(encoded));

    return encoded == expected;
}

bool test_decode(const buffer_t& input, const buffer_t&)
{
    buffer_t encoded, decoded;

    encoded.reserve(get_max_size(input.cbegin(), input.cend()));
    encode(input.cbegin(), input.cend(), std::back_inserter(encoded));

    decoded.reserve(input.size());
    decode(encoded.cbegin(), encoded.cend(), std::back_inserter(decoded));

    return decoded == input;
}

void test(const buffer_t& input, const buffer_t& expected, const std::string& test_name)
{
    auto blah = [&input, &expected, &test_name](auto tester, const std::string test_type)
    {
        if (tester(input, expected))
        {
            std::cout << "pass: ";
        }
        else
        {
            std::cout << "FAIL: ";
        }
        std::cout << test_type << " - " << test_name << '\n';
    };

    blah(test_encode, "Encode");
    blah(test_decode, "Decode");
}

int main()
{
    buffer_t v_in, v_out;

    test({}, { 0 }, "Trivial");
    test({ 0 }, { 1, 1, 0 }, "Single 0");
    test({ 0, 0 }, { 1, 1, 1, 0 }, "Double 0");
    test({ 11, 22, 0, 33 }, { 3, 11, 22, 2, 33, 0 }, "Short with 0");
    test({ 11, 22, 33, 44 }, { 5, 11, 22, 33, 44, 0 }, "Short without 0");
    test({ 11, 00, 00, 00 }, { 2, 11, 1, 1, 1, 0 }, "Many 0s");

    auto in = full_vector();
    in.pop_back();
    auto out = in;
    out.insert(out.begin(), 0xFF);
    out.push_back(0x00);
    test(in, out, "Almost full buffer");

    in = full_vector();
    in.pop_back();
    in.insert(in.begin(), 0x00);
    out = in;
    out.insert(out.begin(), 0x01);
    out[1] = 0xFF;
    out.push_back(0x00);
    test(in, out, "Almost full buffer starting with 0");

    in = full_vector();
    out = in;
    out.insert(out.begin(), 0xFF);
    out.back() = 0x02;
    out.push_back(0xFF);
    out.push_back(0x00);
    test(in, out, "Full buffer");

    in = full_vector();
    std::rotate(in.begin(), in.begin() + 1, in.end());
    in.back() = 0;
    out = in;
    out.insert(out.begin(), 0xFF);
    out.back() = 0x01;
    out.push_back(0x1);
    out.push_back(0x00);
    test(in, out, "Rotated buffer ending with 0");

    in = full_vector();
    std::rotate(in.begin(), in.begin() + 2, in.end());
    *(in.end() - 2) = 0x00;
    *(in.end() - 1) = 0x01;
    out = in;
    out.insert(out.begin(), 0xFE);
    *(out.end() - 2) = 0x02;
    out.push_back(0x00);
    test(in, out, "Rotated buffer ending with 0 1");

    return 0;
}