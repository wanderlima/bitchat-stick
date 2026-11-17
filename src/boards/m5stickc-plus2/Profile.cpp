#include "BoardProfile.h"

namespace stick {
namespace {
constexpr BoardProfile kProfile{"m5stickc-plus2", true, true, true, true, true, true};
static_assert(boardProfileValid(kProfile), "Plus2 profile");
static_assert(!boardProfileValid(BoardProfile{"bare", true, false, false, false, false, false}), "display requires input");
}
const BoardProfile& boardProfile() { return kProfile; }
}
