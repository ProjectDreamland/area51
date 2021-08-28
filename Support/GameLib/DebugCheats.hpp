#ifndef DEBUG_CHEATS_HPP
#define DEBUG_CHEATS_HPP

#include "x_types.hpp"

//=============================================================================

#ifndef CONFIG_RETAIL

extern xbool DEBUG_INFINITE_AMMO;
extern xbool DEBUG_INVULNERABLE;
extern xbool DEBUG_EXPERT_JUMPINGBEAN;

#else

#define DEBUG_INFINITE_AMMO 0
#define DEBUG_INVULNERABLE  0
#define DEBUG_EXPERT_JUMPINGBEAN 0
#endif

//=============================================================================

#endif // DEBUG_CHEATS_HPP
