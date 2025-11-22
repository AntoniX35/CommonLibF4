#pragma once
#include <fmt/format.h>
#include "REL/Version.h"

template <>
struct fmt::formatter<REL::Version>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const REL::Version& v, FormatContext& ctx)
    {
        // подстроено под структуру REL::Version
        return fmt::format_to(
            ctx.out(),
            "{}.{}.{}",
            v.major(),    // если такие методы есть
            v.minor(),
            v.patch()
        );
    }
};
