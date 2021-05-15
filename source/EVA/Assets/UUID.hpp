#pragma once

#include "crossguid/guid.hpp"
#include "Json.hpp"

namespace EVA
{
	using uuid = xg::Guid;
    inline uuid NewUUID() { return xg::newGuid(); };
}

namespace xg
{
    inline void to_json(json& j, const Guid& p)
    {
        j = p.str();
    }

    inline void from_json(const json& j, Guid& p)
    {
        p = Guid(j.get<std::string>());
    }
}
