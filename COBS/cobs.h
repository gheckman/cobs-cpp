#pragma once

#include <algorithm>
#include <iterator>

namespace
{
    constexpr int MAX_BYTES = 254;

    // iterator points to 0 if found
    // iterator distance is <MAX_BYTES and !last if found, MAX_BYTES or last if not found
    template <typename InputIter>
    InputIter find_delimiter_(InputIter first, InputIter last)
    {
        // Keep going until you hit a 0 or the byte limit, whichever comes first
        for (int i = 0; first != last && *first != 0 && i < MAX_BYTES; ++first, ++i);
        return first;
    }
}

template <typename InputIter>
size_t get_max_size(InputIter first, InputIter last)
{
    auto dist = std::distance(first, last);
    return dist + 2 + dist / MAX_BYTES;
}

template <typename FwdIter, typename OutputIter>
OutputIter encode(FwdIter first, FwdIter last, OutputIter out)
{
    while (first != last)
    {
        // Find the next delimiter (either find a 0 or hit the byte limit) and write the distance to 
        // the delimiter and then copy out the data between our current position and the delimiter
        auto it = find_delimiter_(first, last);
        auto dist = std::distance(first, it);
        *out++ = dist + 1;
        out = std::copy(first, it, out);
        first = it;

        // If a 0 was found (as opposed to just being at the byte limit by chance), skip over it
        if (first != last && *first == 0 && dist != MAX_BYTES)
        {
            ++first;

            // If the last byte is a 0, then write the distance to the next 0 (which is 1)
            if (first == last)
            {
                *out++ = 1;
            }
        }
    }

    *out++ = 0;
    return out;
}

template <typename FwdIter, typename OutputIter>
OutputIter decode(FwdIter first, FwdIter last, OutputIter out)
{
    auto it = first;
    auto message_end = std::find(first, last, 0);

    if (message_end != last)
    {
        while (first != message_end)
        {
            bool found_zero = (uint8_t)*it <= MAX_BYTES;
            std::advance(it, *it);
            std::advance(first, 1);
            out = std::copy(first, it, out);
            first = it;

            // If a 0 was found (and it wasn't the one at the end of the message) write a 0
            if (first != message_end && found_zero)
            {
                *out++ = 0;
            }
        }
    }

    return out;
}