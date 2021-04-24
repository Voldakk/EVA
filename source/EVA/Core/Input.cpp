#include "Input.hpp"

namespace EVA
{
    bool operator==(const int& lhs, const KeyCode& rhs) { return lhs == static_cast<int>(rhs); }
} // namespace EVA
