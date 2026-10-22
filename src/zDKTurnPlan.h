#pragma once
#include <stdexcept>
#include <vector>

namespace zDK
{
enum class zDKTurnAttackKind
{
    zDKTurnNormal,
    zDKTurnSuper,
    zDKTurnUltra
};

enum class zDKTurnBuildUpTrigger
{
    zDKTurnNone,
    zDKTurnAnyAttack,
    zDKTurnSuperAttack,
    zDKTurnUltraSuperAttack
};

inline constexpr int zDKTurnMaximumAttacks = 16;
inline const char* zDKTurnAttackNames[] = {"Normal Attack", "Super Attack", "Ultra Super Attack"};
inline const char* zDKTurnBuildUpNames[] = {
    "Fixed boost",
    "With each attack performed",
    "With each Super Attack performed",
    "With each Ultra Super Attack performed"
};

struct zDKTurnAttackPlan
{
    std::vector<zDKTurnAttackKind> zDKTurnAttacks;
};

inline void zDKTurnValidate(const zDKTurnAttackPlan& zDKTurnPlan)
{
    if (zDKTurnPlan.zDKTurnAttacks.size() > zDKTurnMaximumAttacks)
        throw std::invalid_argument("An attack plan supports up to 16 attacks.");
    for (auto zDKTurnKind : zDKTurnPlan.zDKTurnAttacks)
        if (int(zDKTurnKind) < 0 || int(zDKTurnKind) > 2)
            throw std::invalid_argument("Invalid attack in the turn plan.");
}

inline bool zDKTurnMatches(zDKTurnBuildUpTrigger zDKTurnTrigger, zDKTurnAttackKind zDKTurnKind)
{
    return zDKTurnTrigger == zDKTurnBuildUpTrigger::zDKTurnAnyAttack ||
        (zDKTurnTrigger == zDKTurnBuildUpTrigger::zDKTurnSuperAttack &&
         zDKTurnKind != zDKTurnAttackKind::zDKTurnNormal) ||
        (zDKTurnTrigger == zDKTurnBuildUpTrigger::zDKTurnUltraSuperAttack &&
         zDKTurnKind == zDKTurnAttackKind::zDKTurnUltra);
}
} // namespace zDK
