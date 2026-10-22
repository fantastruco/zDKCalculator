#pragma once
#include "zDKCalculator.h"
#include "zDKTurnPlan.h"
#include <vector>
#include <sstream>
#include <iomanip>

namespace zDK
{
enum class zDKEffectCategory
{
    zDKEffectBasic,
    zDKEffectActive,
    zDKEffectEntrance,
    zDKEffectSuperAttackStack,
    zDKEffectGuard, // Legacy file identifier; guard is now a selectable skill effect.
    zDKEffectSupport,
    zDKEffectWhenAttacking,
    zDKEffectWhenReceiving,
    zDKEffectUltraSuperAttack
};
enum class zDKEffectStat
{
    zDKEffectDefense,
    zDKEffectDamageReduction,
    zDKEffectNormalDamageReduction,
    zDKEffectSuperDamageReduction,
    zDKEffectTypeDefenseBoost,
    zDKEffectDefenseDebuff,
    zDKEffectAttack,
    zDKEffectBossDefense,
    zDKEffectNormalAttack,
    zDKEffectSuperAttack,
    zDKEffectPenetration,
    zDKEffectDamageReceived,
    zDKEffectGuard
};
enum class zDKEffectDefensePhase
{
    zDKEffectStartOfTurn,
    zDKEffectDomain,
    zDKEffectItem,
    zDKEffectLinks,
    zDKEffectActive,
    zDKEffectWhenAttacking,
    zDKEffectSuperAttack,
    zDKEffectMidBattle,
    zDKEffectWhenReceiving,
    zDKEffectWhenReceivingSuper,
    zDKEffectWhenReceivingNormal
};
enum class zDKEffectHealthRule
{
    zDKEffectAlways,
    zDKEffectBelow,
    zDKEffectAbove
};
enum class zDKEffectAttackTiming
{
    zDKEffectAlways,
    zDKEffectBeforeAttack,
    zDKEffectAfterAttack
};
inline const char* zDKEffectCategoryNames[] = {
    "Basic Effect",
    "Active Skill",
    "Entrance Animation",
    "Super Attack",
    "Guard",
    "Ally Support",
    "When Attacking",
    "When Receiving Attack",
    "Ultra Super Attack"
};
inline const char* zDKEffectGroupNames[] = {
    "Start of turn (SoT)",
    "Domain",
    "Support Item",
    "Link Skills",
    "Active Skill",
    "When attacking",
    "Super Attack Effect",
    "Mid-battle passive (Phase 2)",
    "When receiving an attack",
    "When receiving a Super Attack",
    "When receiving a normal attack"
};
inline const char* zDKEffectStatNames[] = {
    "DEF",
    "Damage Reduction (DR)",
    "DR against normals",
    "DR against Super Attacks",
    "Type DEF Boost",
    "DEF Down",
    "ATK",
    "DEF",
    "Normal ATK",
    "SA ATK",
    "DEF ignored",
    "Damage received increase (Domain)",
    "Guards all attacks"
};

struct zDKEffectDefinition
{
    std::string zDKEffectName;
    zDKEffectCategory zDKEffectCategory = zDKEffectCategory::zDKEffectBasic;
    zDKEffectStat zDKEffectStat = zDKEffectStat::zDKEffectDefense;
    zDKEffectDefensePhase zDKEffectGroup = zDKEffectDefensePhase::zDKEffectStartOfTurn;
    double zDKEffectValue = 0;
    int zDKEffectCount = 1;
    bool zDKEffectEnabled = true, zDKEffectMultiplicative = false;
    int zDKEffectFirstTurn = 1, zDKEffectLastTurn = 0;
    zDKEffectHealthRule zDKEffectHealthRule = zDKEffectHealthRule::zDKEffectAlways;
    double zDKEffectHealthThresholdPercent = 0;
    zDKEffectAttackTiming zDKEffectTiming = zDKEffectAttackTiming::zDKEffectAlways;
    zDKTurnBuildUpTrigger zDKEffectBuildUpTrigger = zDKTurnBuildUpTrigger::zDKTurnNone;
    int zDKEffectBuildUpCount = 0, zDKEffectBuildUpLimit = 0;
};

enum class zDKEffectSection
{
    zDKEffectEntrance,
    zDKEffectBasic,
    zDKEffectAttacking,
    zDKEffectReceiving,
    zDKEffectSuperAttack,
    zDKEffectActive
};

inline const char* zDKEffectSectionNames[] = {
    "Entrance Animation",
    "Basic Effect",
    "When Attacking",
    "When Receiving Attack",
    "Super Attack / Ultra Super Attack",
    "Active Skill"
};

inline bool zDKEffectIsGuard(const zDKEffectDefinition& zDKEffectEntry)
{
    return zDKEffectEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectGuard ||
        zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectGuard;
}

inline bool zDKEffectUsesSuperCount(const zDKEffectDefinition& zDKEffectEntry)
{
    return !zDKEffectIsGuard(zDKEffectEntry) &&
        (zDKEffectEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectSuperAttackStack ||
         zDKEffectEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack ||
         (zDKEffectEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectSupport &&
          zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectSuperAttack));
}

inline zDKEffectSection zDKEffectGetSection(const zDKEffectDefinition& zDKEffectEntry)
{
    if (zDKEffectEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectEntrance)
        return zDKEffectSection::zDKEffectEntrance;
    if (zDKEffectEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectActive ||
        zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectActive)
        return zDKEffectSection::zDKEffectActive;
    if (zDKEffectEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectWhenReceiving ||
        int(zDKEffectEntry.zDKEffectGroup) >= 8)
        return zDKEffectSection::zDKEffectReceiving;
    if (zDKEffectEntry.zDKEffectCategory == zDKEffectCategory::zDKEffectWhenAttacking ||
        zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectWhenAttacking ||
        zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectMidBattle)
        return zDKEffectSection::zDKEffectAttacking;
    if (zDKEffectUsesSuperCount(zDKEffectEntry) ||
        zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectSuperAttack)
        return zDKEffectSection::zDKEffectSuperAttack;
    return zDKEffectSection::zDKEffectBasic;
}

inline std::vector<int> zDKEffectDisplayOrder(const std::vector<zDKEffectDefinition>& zDKEffectEntries)
{
    std::vector<int> zDKEffectOrder;
    for (int zDKEffectIndex = 0; zDKEffectIndex < int(zDKEffectEntries.size()); ++zDKEffectIndex)
        zDKEffectOrder.push_back(zDKEffectIndex);
    std::stable_sort(
        zDKEffectOrder.begin(),
        zDKEffectOrder.end(),
        [&](int zDKEffectLeft, int zDKEffectRight)
        {
            const auto zDKEffectLeftSection = zDKEffectGetSection(zDKEffectEntries[zDKEffectLeft]);
            const auto zDKEffectRightSection = zDKEffectGetSection(zDKEffectEntries[zDKEffectRight]);
            if (zDKEffectLeftSection != zDKEffectRightSection)
                return int(zDKEffectLeftSection) < int(zDKEffectRightSection);
            if (zDKEffectLeftSection == zDKEffectSection::zDKEffectSuperAttack)
                return (zDKEffectEntries[zDKEffectLeft].zDKEffectCategory ==
                        zDKEffectCategory::zDKEffectUltraSuperAttack) <
                    (zDKEffectEntries[zDKEffectRight].zDKEffectCategory ==
                     zDKEffectCategory::zDKEffectUltraSuperAttack);
            return false;
        }
    );
    return zDKEffectOrder;
}

struct zDKEffectUnitProfile
{
    std::string zDKEffectName = "Unit";
    double zDKEffectBaseDefense = 0, zDKEffectLeaderSkillPercent = 0, zDKEffectTypeDefenseBoostLevel = 0,
           zDKEffectManualDefense = 0;
    bool zDKEffectUseManualDefense = false, zDKEffectRoundDownDefense = true;
    zDKCalcUnitType zDKEffectElementType = zDKCalcUnitType::zDKCalcAGL;
    zDKCalcUnitClass zDKEffectCombatClass = zDKCalcUnitClass::zDKCalcSuper;
    std::vector<zDKEffectDefinition> zDKEffectEffects;
    zDKTurnAttackPlan zDKEffectAttackPlan;
};

struct zDKEffectBossProfile
{
    std::string zDKEffectName = "Boss";
    zDKCalcEnemyStats zDKEffectStats;
    double zDKEffectBaseDefense = 0, zDKEffectHealthPercent = 0;
    std::vector<zDKEffectDefinition> zDKEffectEffects;

    zDKEffectBossProfile()
    {
        zDKEffectStats.zDKCalcNormalAttack = 0;
        zDKEffectStats.zDKCalcSuperAttackMultiplier = 1;
        zDKEffectStats.zDKCalcUseSeparateAttackStats = true;
        zDKEffectStats.zDKCalcSuperAttack = 0;
    }
};

struct zDKEffectBattleState
{
    int zDKEffectTurn = 1;
    double zDKEffectMaxHP = 0, zDKEffectCurrentHP = 0;
    bool zDKEffectAfterUnitAttack = true;
};

struct zDKEffectEvaluation
{
    zDKCalcBattleResult zDKEffectResult;
    zDKCalcUnitStats zDKEffectResolvedUnit;
    zDKCalcEnemyStats zDKEffectResolvedBoss;
    zDKCalcDefenseBreakdown zDKEffectNormalDefense, zDKEffectSuperDefense;
    double zDKEffectBossDefense = 0;
    std::vector<bool> zDKEffectUnitEffectActivation, zDKEffectBossEffectActivation;
};

inline zDKEffectDefinition zDKEffectCreate(
    zDKEffectCategory zDKEffectCategoryData, bool zDKEffectIsBoss = false
)
{
    zDKEffectDefinition zDKEffectEffectData;
    zDKEffectEffectData.zDKEffectCategory = zDKEffectCategoryData;
    zDKEffectEffectData.zDKEffectStat =
        zDKEffectIsBoss ? zDKEffectStat::zDKEffectAttack : zDKEffectStat::zDKEffectDefense;
    if (zDKEffectCategoryData == zDKEffectCategory::zDKEffectActive)
        zDKEffectEffectData.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectActive;
    if (zDKEffectCategoryData == zDKEffectCategory::zDKEffectEntrance)
        zDKEffectEffectData.zDKEffectLastTurn = 3;
    if (zDKEffectCategoryData == zDKEffectCategory::zDKEffectSuperAttackStack ||
        zDKEffectCategoryData == zDKEffectCategory::zDKEffectUltraSuperAttack)
    {
        zDKEffectEffectData.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectSuperAttack;
        zDKEffectEffectData.zDKEffectCount = 0;
    }
    if (zDKEffectCategoryData == zDKEffectCategory::zDKEffectWhenAttacking)
        zDKEffectEffectData.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectWhenAttacking;
    if (zDKEffectCategoryData == zDKEffectCategory::zDKEffectWhenReceiving)
        zDKEffectEffectData.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectWhenReceiving;
    if (zDKEffectCategoryData == zDKEffectCategory::zDKEffectGuard)
    {
        zDKEffectEffectData.zDKEffectCategory = zDKEffectCategory::zDKEffectBasic;
        zDKEffectEffectData.zDKEffectStat = zDKEffectStat::zDKEffectGuard;
        zDKEffectEffectData.zDKEffectValue = 0;
    }
    return zDKEffectEffectData;
}

inline zDKEffectUnitProfile zDKEffectCreateEmptyUnitProfile()
{
    return zDKEffectUnitProfile{};
}

inline zDKEffectUnitProfile zDKEffectCreateGuideUnitProfile()
{
    zDKEffectUnitProfile zDKEffectUnitProfileData;
    zDKEffectUnitProfileData.zDKEffectBaseDefense = 12269;
    zDKEffectUnitProfileData.zDKEffectLeaderSkillPercent = 440;
    zDKEffectUnitProfileData.zDKEffectManualDefense = 400000;
    auto zDKEffectBasic = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
    zDKEffectBasic.zDKEffectValue = 200;
    zDKEffectUnitProfileData.zDKEffectEffects.push_back(zDKEffectBasic);
    auto zDKEffectStack = zDKEffectCreate(zDKEffectCategory::zDKEffectSuperAttackStack);
    zDKEffectStack.zDKEffectValue = 50;
    zDKEffectStack.zDKEffectCount = 9;
    zDKEffectUnitProfileData.zDKEffectEffects.push_back(zDKEffectStack);
    return zDKEffectUnitProfileData;
}

inline void zDKEffectValidateEffect(const zDKEffectDefinition& zDKEffectEffectData, bool zDKEffectIsBoss)
{
    zDKCalcValidateRange(int(zDKEffectEffectData.zDKEffectCategory), 0, 8, "Invalid effect category.");
    zDKCalcValidateRange(int(zDKEffectEffectData.zDKEffectStat), 0, 12, "Invalid effect statistic.");
    zDKCalcValidateRange(int(zDKEffectEffectData.zDKEffectGroup), 0, 10, "Invalid DEF boost phase.");
    if ((zDKEffectEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectWhenAttacking &&
         zDKEffectEffectData.zDKEffectGroup != zDKEffectDefensePhase::zDKEffectWhenAttacking &&
         zDKEffectEffectData.zDKEffectGroup != zDKEffectDefensePhase::zDKEffectMidBattle) ||
        (zDKEffectEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectWhenReceiving &&
         int(zDKEffectEffectData.zDKEffectGroup) < 8) ||
        (zDKEffectEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack &&
         zDKEffectEffectData.zDKEffectGroup != zDKEffectDefensePhase::zDKEffectSuperAttack))
        throw std::invalid_argument("Effect type does not match its activation phase.");
    zDKCalcValidateRange(int(zDKEffectEffectData.zDKEffectHealthRule), 0, 2, "Invalid HP condition.");
    zDKCalcValidateRange(int(zDKEffectEffectData.zDKEffectTiming), 0, 2, "Invalid attack timing.");
    zDKCalcValidateRange(
        zDKEffectEffectData.zDKEffectCount,
        zDKEffectUsesSuperCount(zDKEffectEffectData) ||
                zDKEffectEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectSuperAttackStack ||
                zDKEffectEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack
            ? 0
            : 1,
        10000,
        "Active raise count is outside the supported range."
    );
    zDKCalcValidateRange(int(zDKEffectEffectData.zDKEffectBuildUpTrigger), 0, 3, "Invalid build-up trigger.");
    zDKCalcValidateRange(zDKEffectEffectData.zDKEffectBuildUpCount, 0, 10000, "Invalid build-up count.");
    zDKCalcValidateRange(zDKEffectEffectData.zDKEffectBuildUpLimit, 0, 10000, "Invalid build-up limit.");
    if (zDKEffectEffectData.zDKEffectBuildUpTrigger != zDKTurnBuildUpTrigger::zDKTurnNone &&
        (zDKEffectIsBoss || zDKEffectEffectData.zDKEffectStat != zDKEffectStat::zDKEffectDefense ||
         (zDKEffectEffectData.zDKEffectGroup != zDKEffectDefensePhase::zDKEffectWhenAttacking &&
          zDKEffectEffectData.zDKEffectGroup != zDKEffectDefensePhase::zDKEffectMidBattle)))
        throw std::invalid_argument("Attack build-up requires a unit DEF effect in Phase 2.");
    if (zDKEffectEffectData.zDKEffectBuildUpLimit &&
        zDKEffectEffectData.zDKEffectBuildUpCount > zDKEffectEffectData.zDKEffectBuildUpLimit)
        throw std::invalid_argument("Starting build-up exceeds its activation limit.");
    zDKCalcValidateRange(
        zDKEffectEffectData.zDKEffectFirstTurn, 1, 9999, "Start turn must be between 1 and 9,999."
    );
    zDKCalcValidateRange(
        zDKEffectEffectData.zDKEffectLastTurn, 0, 9999, "End turn must be between 0 and 9,999."
    );
    if (zDKEffectEffectData.zDKEffectLastTurn &&
        zDKEffectEffectData.zDKEffectLastTurn < zDKEffectEffectData.zDKEffectFirstTurn)
        throw std::invalid_argument("End turn must be at or after start turn.");
    zDKCalcValidateRange(
        zDKEffectEffectData.zDKEffectHealthThresholdPercent,
        0,
        100,
        "HP threshold must be between 0 and 100%."
    );
    if (zDKEffectEffectData.zDKEffectName.size() > 160)
        throw std::invalid_argument("Effect name is too long.");
    const int zDKEffectStatisticIndex = int(zDKEffectEffectData.zDKEffectStat);
    if (zDKEffectIsBoss &&
        (zDKEffectStatisticIndex < 6 || zDKEffectStatisticIndex >= 11 ||
         zDKEffectEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectGuard ||
         int(zDKEffectEffectData.zDKEffectCategory) >= 5))
        throw std::invalid_argument("Unsupported boss effect.");
    if (zDKEffectIsBoss && int(zDKEffectEffectData.zDKEffectGroup) >= 8)
        throw std::invalid_argument(
            "Receiving-attack conditions belong to the unit, not the boss boost phase."
        );
    if (!zDKEffectIsBoss && zDKEffectStatisticIndex > 5 && zDKEffectStatisticIndex != 11 &&
        !zDKEffectIsGuard(zDKEffectEffectData))
        throw std::invalid_argument("Unsupported unit effect.");
    const double zDKEffectMinimum =
        (zDKEffectIsBoss && zDKEffectEffectData.zDKEffectStat != zDKEffectStat::zDKEffectPenetration) ? -100
                                                                                                      : 0;
    const double zDKEffectMaximum =
        (zDKEffectEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDamageReduction ||
         zDKEffectEffectData.zDKEffectStat == zDKEffectStat::zDKEffectNormalDamageReduction ||
         zDKEffectEffectData.zDKEffectStat == zDKEffectStat::zDKEffectSuperDamageReduction ||
         zDKEffectEffectData.zDKEffectStat == zDKEffectStat::zDKEffectDefenseDebuff ||
         zDKEffectEffectData.zDKEffectStat == zDKEffectStat::zDKEffectPenetration)
        ? 100
        : 1e6;
    zDKCalcValidateRange(
        zDKEffectEffectData.zDKEffectValue,
        zDKEffectMinimum,
        zDKEffectMaximum,
        "Effect amount is outside the supported range."
    );
    if (zDKEffectEffectData.zDKEffectStat == zDKEffectStat::zDKEffectTypeDefenseBoost &&
        (std::floor(zDKEffectEffectData.zDKEffectValue) != zDKEffectEffectData.zDKEffectValue ||
         zDKEffectEffectData.zDKEffectValue > 10000))
        throw std::invalid_argument("Type DEF Boost must be a whole level between 0 and 10,000.");
    if (std::abs(zDKEffectEffectData.zDKEffectValue * zDKEffectEffectData.zDKEffectCount) > 1e8 ||
        std::abs(zDKEffectEffectData.zDKEffectValue * zDKEffectEffectData.zDKEffectBuildUpCount) > 1e8)
        throw std::invalid_argument("Combined stack amount is too large.");
}

inline void zDKEffectValidate(const zDKEffectUnitProfile& zDKEffectUnitProfileData)
{
    zDKTurnValidate(zDKEffectUnitProfileData.zDKEffectAttackPlan);
    zDKCalcValidateRange(
        zDKEffectUnitProfileData.zDKEffectBaseDefense, 0, 1e12, "Base DEF must be between 0 and 1 trillion."
    );
    zDKCalcValidateRange(
        zDKEffectUnitProfileData.zDKEffectManualDefense,
        0,
        1e12,
        "Manual DEF must be between 0 and 1 trillion."
    );
    zDKCalcValidateRange(
        zDKEffectUnitProfileData.zDKEffectLeaderSkillPercent,
        0,
        1e6,
        "Leader bonus is outside the supported range."
    );
    zDKCalcValidateRange(
        zDKEffectUnitProfileData.zDKEffectTypeDefenseBoostLevel,
        0,
        10000,
        "Type DEF Boost level is outside the supported range."
    );
    if (std::floor(zDKEffectUnitProfileData.zDKEffectTypeDefenseBoostLevel) !=
        zDKEffectUnitProfileData.zDKEffectTypeDefenseBoostLevel)
        throw std::invalid_argument("Type DEF Boost must be a whole level.");
    zDKCalcValidateRange(int(zDKEffectUnitProfileData.zDKEffectElementType), 0, 4, "Invalid unit type.");
    zDKCalcValidateRange(int(zDKEffectUnitProfileData.zDKEffectCombatClass), 0, 1, "Invalid unit class.");
    if (zDKEffectUnitProfileData.zDKEffectName.size() > 160 ||
        zDKEffectUnitProfileData.zDKEffectEffects.size() > 128)
        throw std::invalid_argument("Unit preset exceeds the supported size.");
    for (const auto& zDKEffectEntry : zDKEffectUnitProfileData.zDKEffectEffects)
        zDKEffectValidateEffect(zDKEffectEntry, false);
}

inline void zDKEffectValidate(const zDKEffectBossProfile& zDKEffectBossProfileData)
{
    zDKCalcUnitStats zDKEffectUnitStatsData;
    zDKCalcCalculateBattle(zDKEffectUnitStatsData, zDKEffectBossProfileData.zDKEffectStats);
    zDKCalcValidateRange(
        zDKEffectBossProfileData.zDKEffectBaseDefense, 0, 1e12, "Boss DEF is outside the supported range."
    );
    zDKCalcValidateRange(
        zDKEffectBossProfileData.zDKEffectHealthPercent, 0, 100, "Boss HP must be between 0 and 100%."
    );
    zDKCalcValidateRange(
        int(zDKEffectBossProfileData.zDKEffectStats.zDKCalcCombatClass), 0, 1, "Invalid boss class."
    );
    if (zDKEffectBossProfileData.zDKEffectName.size() > 160 ||
        zDKEffectBossProfileData.zDKEffectEffects.size() > 128)
        throw std::invalid_argument("Boss configuration exceeds the supported size.");
    for (const auto& zDKEffectEntry : zDKEffectBossProfileData.zDKEffectEffects)
        zDKEffectValidateEffect(zDKEffectEntry, true);
}

inline void zDKEffectValidate(const zDKEffectBattleState& zDKEffectBattleStateData)
{
    zDKCalcValidateRange(
        zDKEffectBattleStateData.zDKEffectTurn, 1, 9999, "Turn must be between 1 and 9,999."
    );
    zDKCalcValidateRange(
        zDKEffectBattleStateData.zDKEffectMaxHP, 0, 1e12, "Max HP must be between 0 and 1 trillion."
    );
    zDKCalcValidateRange(
        zDKEffectBattleStateData.zDKEffectCurrentHP,
        0,
        zDKEffectBattleStateData.zDKEffectMaxHP,
        "Current HP must be between 0 and maximum HP."
    );
}

inline bool zDKEffectIsActive(
    const zDKEffectDefinition& zDKEffectEffectData,
    const zDKEffectBattleState& zDKEffectBattleStateData,
    double zDKEffectHealthPercent,
    bool zDKEffectIncomingSuper = true,
    bool zDKEffectIsBoss = false
)
{
    if (!zDKEffectEffectData.zDKEffectEnabled ||
        zDKEffectBattleStateData.zDKEffectTurn < zDKEffectEffectData.zDKEffectFirstTurn ||
        (zDKEffectEffectData.zDKEffectLastTurn &&
         zDKEffectBattleStateData.zDKEffectTurn > zDKEffectEffectData.zDKEffectLastTurn))
        return false;
    if (zDKEffectEffectData.zDKEffectHealthRule == zDKEffectHealthRule::zDKEffectBelow &&
        zDKEffectHealthPercent > zDKEffectEffectData.zDKEffectHealthThresholdPercent)
        return false;
    if (zDKEffectEffectData.zDKEffectHealthRule == zDKEffectHealthRule::zDKEffectAbove &&
        zDKEffectHealthPercent < zDKEffectEffectData.zDKEffectHealthThresholdPercent)
        return false;
    if (zDKEffectEffectData.zDKEffectTiming == zDKEffectAttackTiming::zDKEffectBeforeAttack &&
        zDKEffectBattleStateData.zDKEffectAfterUnitAttack)
        return false;
    if (zDKEffectEffectData.zDKEffectTiming == zDKEffectAttackTiming::zDKEffectAfterAttack &&
        !zDKEffectBattleStateData.zDKEffectAfterUnitAttack)
        return false;
    if (!zDKEffectIsBoss &&
        zDKEffectEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectWhenAttacking &&
        zDKEffectEffectData.zDKEffectBuildUpTrigger == zDKTurnBuildUpTrigger::zDKTurnNone &&
        !zDKEffectBattleStateData.zDKEffectAfterUnitAttack)
        return false;
    if (!zDKEffectIsBoss &&
        ((zDKEffectEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectWhenReceivingSuper &&
          !zDKEffectIncomingSuper) ||
         (zDKEffectEffectData.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectWhenReceivingNormal &&
          zDKEffectIncomingSuper)))
        return false;
    return true;
}

inline double zDKEffectTeamHealthPercent(const zDKEffectBattleState& zDKEffectState)
{
    const auto zDKEffectMaximum = zDKCalcFloorStatistic(zDKEffectState.zDKEffectMaxHP);
    return zDKEffectMaximum > 0
        ? 100.0 * zDKCalcFloorStatistic(zDKEffectState.zDKEffectCurrentHP) / zDKEffectMaximum
        : 0;
}

inline zDKCalcUnitStats zDKEffectResolveUnit(
    const zDKEffectUnitProfile& zDKEffectUnitProfileData,
    const zDKEffectBattleState& zDKEffectBattleStateData,
    std::vector<bool>* zDKEffectActiveEffects = nullptr,
    bool zDKEffectIncomingSuper = true
)
{
    zDKCalcUnitStats zDKEffectUnitStatsData;
    zDKEffectUnitStatsData.zDKCalcBaseDefense = zDKEffectUnitProfileData.zDKEffectBaseDefense;
    zDKEffectUnitStatsData.zDKCalcLeaderSkillPercent = zDKEffectUnitProfileData.zDKEffectLeaderSkillPercent;
    zDKEffectUnitStatsData.zDKCalcStartOfTurnPercent = 0;
    zDKEffectUnitStatsData.zDKCalcSuperAttackStackCount = 0;
    zDKEffectUnitStatsData.zDKCalcStackDefensePercent = 0;
    zDKEffectUnitStatsData.zDKCalcSuperAttackDefensePercent = 0;
    zDKEffectUnitStatsData.zDKCalcCurrentSuperAttackCount = 0;
    zDKEffectUnitStatsData.zDKCalcElementType = zDKEffectUnitProfileData.zDKEffectElementType;
    zDKEffectUnitStatsData.zDKCalcCombatClass = zDKEffectUnitProfileData.zDKEffectCombatClass;
    zDKEffectUnitStatsData.zDKCalcUseManualDefense = zDKEffectUnitProfileData.zDKEffectUseManualDefense;
    zDKEffectUnitStatsData.zDKCalcManualDefense = zDKEffectUnitProfileData.zDKEffectManualDefense;
    zDKEffectUnitStatsData.zDKCalcRoundDownDefense = true;
    zDKEffectUnitStatsData.zDKCalcTeamHealth =
        static_cast<double>(zDKCalcFloorStatistic(zDKEffectBattleStateData.zDKEffectCurrentHP));
    zDKEffectUnitStatsData.zDKCalcTypeDefenseBoostLevel =
        zDKEffectUnitProfileData.zDKEffectTypeDefenseBoostLevel;
    zDKEffectUnitStatsData.zDKCalcAfterUnitAttack = true;
    double* zDKEffectGroups[] = {
        &zDKEffectUnitStatsData.zDKCalcStartOfTurnPercent,
        &zDKEffectUnitStatsData.zDKCalcDomainPercent,
        &zDKEffectUnitStatsData.zDKCalcItemPercent,
        &zDKEffectUnitStatsData.zDKCalcLinkSkillPercent,
        &zDKEffectUnitStatsData.zDKCalcActiveSkillPercent,
        &zDKEffectUnitStatsData.zDKCalcOnAttackPercent,
        &zDKEffectUnitStatsData.zDKCalcOtherSuperAttackPercent
    };
    double zDKEffectSums[4] = {0, 0, 0, 0}, zDKEffectFactors[4] = {1, 1, 1, 1};
    double zDKEffectDamageIncrease = 0, zDKEffectDamageFactor = 1;
    if (zDKEffectActiveEffects)
        zDKEffectActiveEffects->clear();
    for (const auto& zDKEffectEntry : zDKEffectUnitProfileData.zDKEffectEffects)
    {
        bool zDKEffectEnabled = zDKEffectIsActive(
            zDKEffectEntry,
            zDKEffectBattleStateData,
            zDKEffectTeamHealthPercent(zDKEffectBattleStateData),
            zDKEffectIncomingSuper
        );
        if (zDKEffectActiveEffects)
            zDKEffectActiveEffects->push_back(zDKEffectEnabled);
        if (!zDKEffectEnabled)
            continue;
        if (zDKEffectIsGuard(zDKEffectEntry))
        {
            zDKEffectUnitStatsData.zDKCalcGuardsAllAttacks = true;
            continue;
        }
        const double zDKEffectValue = zDKEffectEntry.zDKEffectValue *
            (zDKEffectUsesSuperCount(zDKEffectEntry) ? zDKEffectEntry.zDKEffectCount
                 : zDKEffectEntry.zDKEffectBuildUpTrigger != zDKTurnBuildUpTrigger::zDKTurnNone
                 ? zDKEffectEntry.zDKEffectBuildUpCount
                 : 1);
        if (zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectDefense)
        {
            const int zDKEffectPhaseIndex =
                int(zDKEffectEntry.zDKEffectGroup) >= 7 ? 5 : int(zDKEffectEntry.zDKEffectGroup);
            if (zDKEffectEntry.zDKEffectMultiplicative)
                zDKEffectUnitStatsData.zDKCalcIndependentDefenseFactors[zDKEffectPhaseIndex + 1].push_back(
                    zDKCalcPercentMultiplier(zDKEffectValue)
                );
            else
                *zDKEffectGroups[zDKEffectPhaseIndex] += zDKEffectValue;
        }
        else if (zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectDamageReceived)
        {
            if (zDKEffectEntry.zDKEffectMultiplicative)
            {
                zDKEffectDamageFactor *= zDKCalcPercentMultiplier(zDKEffectValue);
                zDKEffectUnitStatsData.zDKCalcDamageReceivedFactors.push_back(
                    zDKCalcPercentMultiplier(zDKEffectValue)
                );
            }
            else
                zDKEffectDamageIncrease += zDKEffectValue;
        }
        else if (zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectTypeDefenseBoost)
            zDKEffectUnitStatsData.zDKCalcTypeDefenseBoostLevel += zDKEffectValue;
        else
        {
            int zDKEffectSlot = zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectDamageReduction ? 0
                : zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectNormalDamageReduction         ? 1
                : zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectSuperDamageReduction          ? 2
                                                                                                        : 3;
            if (zDKEffectEntry.zDKEffectMultiplicative)
            {
                zDKEffectFactors[zDKEffectSlot] *= std::max(0.0, 1 - zDKEffectValue / 100);
                zDKEffectUnitStatsData.zDKCalcReductionFactors[zDKEffectSlot].push_back(
                    std::max(0.0, 1 - zDKEffectValue / 100)
                );
            }
            else
                zDKEffectSums[zDKEffectSlot] += zDKEffectValue;
        }
    }
    double* zDKEffectReductions[] = {
        &zDKEffectUnitStatsData.zDKCalcDamageReductionPercent,
        &zDKEffectUnitStatsData.zDKCalcNormalDamageReductionPercent,
        &zDKEffectUnitStatsData.zDKCalcSuperDamageReductionPercent,
        &zDKEffectUnitStatsData.zDKCalcDefenseDebuffPercent
    };
    for (int zDKEffectIndex = 0; zDKEffectIndex < 4; ++zDKEffectIndex)
    {
        *zDKEffectReductions[zDKEffectIndex] = 100 *
            (1 -
             (1 - std::min(100.0, zDKEffectSums[zDKEffectIndex]) / 100) * zDKEffectFactors[zDKEffectIndex]);
        auto& zDKEffectSources = zDKEffectUnitStatsData.zDKCalcReductionFactors[zDKEffectIndex];
        zDKEffectSources.insert(
            zDKEffectSources.begin(), 1 - std::min(100.0, zDKEffectSums[zDKEffectIndex]) / 100
        );
    }
    zDKEffectUnitStatsData.zDKCalcDamageReceivedPercent =
        100 * (zDKCalcPercentMultiplier(zDKEffectDamageIncrease) * zDKEffectDamageFactor - 1);
    zDKEffectUnitStatsData.zDKCalcDamageReceivedFactors.insert(
        zDKEffectUnitStatsData.zDKCalcDamageReceivedFactors.begin(),
        zDKCalcPercentMultiplier(zDKEffectDamageIncrease)
    );
    return zDKEffectUnitStatsData;
}

inline zDKCalcEnemyStats zDKEffectResolveBoss(
    const zDKEffectBossProfile& zDKEffectBossProfileData,
    const zDKEffectBattleState& zDKEffectBattleStateData,
    double& zDKEffectDefense,
    std::vector<bool>* zDKEffectActiveEffects = nullptr
)
{
    zDKCalcEnemyStats zDKEffectEnemy = zDKEffectBossProfileData.zDKEffectStats;
    double zDKEffectSums[4][4]{};
    std::vector<double> zDKEffectFactors[4][4];
    if (zDKEffectActiveEffects)
        zDKEffectActiveEffects->clear();
    for (const auto& zDKEffectEntry : zDKEffectBossProfileData.zDKEffectEffects)
    {
        bool zDKEffectEnabled = zDKEffectIsActive(
            zDKEffectEntry,
            zDKEffectBattleStateData,
            zDKEffectBossProfileData.zDKEffectHealthPercent,
            true,
            true
        );
        if (zDKEffectActiveEffects)
            zDKEffectActiveEffects->push_back(zDKEffectEnabled);
        if (!zDKEffectEnabled)
            continue;
        const double zDKEffectValue = zDKEffectEntry.zDKEffectValue *
            (zDKEffectUsesSuperCount(zDKEffectEntry) ? zDKEffectEntry.zDKEffectCount : 1);
        if (zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectPenetration)
        {
            zDKEffectEnemy.zDKCalcDefenseIgnoredPercent =
                std::min(100.0, zDKEffectEnemy.zDKCalcDefenseIgnoredPercent + zDKEffectValue);
            continue;
        }
        const int zDKEffectIndex = zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectAttack ? 0
            : zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectBossDefense                 ? 1
            : zDKEffectEntry.zDKEffectStat == zDKEffectStat::zDKEffectNormalAttack                ? 2
                                                                                                  : 3;
        const int zDKEffectPhaseIndex =
            zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectMidBattle ||
                zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectWhenAttacking
            ? 3
            : zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectDomain ? 1
            : zDKEffectEntry.zDKEffectGroup == zDKEffectDefensePhase::zDKEffectItem   ? 2
                                                                                      : 0;
        if (zDKEffectEntry.zDKEffectMultiplicative)
            zDKEffectFactors[zDKEffectIndex][zDKEffectPhaseIndex].push_back(
                std::max(0.0, zDKCalcPercentMultiplier(zDKEffectValue))
            );
        else
            zDKEffectSums[zDKEffectIndex][zDKEffectPhaseIndex] += zDKEffectValue;
    }
    auto zDKEffectBuildStatistic = [&](double zDKEffectBase, int zDKEffectStatIndex)
    {
        auto zDKEffectValue = zDKCalcFloorStatistic(zDKEffectBase);
        for (int zDKEffectPhaseIndex = 0; zDKEffectPhaseIndex < 4; ++zDKEffectPhaseIndex)
        {
            zDKEffectValue = zDKCalcApplyMultiplier(
                zDKEffectValue,
                std::max(
                    0.0, zDKCalcPercentMultiplier(zDKEffectSums[zDKEffectStatIndex][zDKEffectPhaseIndex])
                )
            );
            for (double zDKEffectFactor : zDKEffectFactors[zDKEffectStatIndex][zDKEffectPhaseIndex])
                zDKEffectValue = zDKCalcApplyMultiplier(zDKEffectValue, zDKEffectFactor);
        }
        return zDKEffectValue;
    };
    const double zDKEffectManualFactor =
        zDKCalcPercentMultiplier(zDKEffectEnemy.zDKCalcAttackModifierPercent);
    zDKEffectEnemy.zDKCalcNormalAttack = static_cast<double>(zDKCalcApplyMultiplier(
        zDKEffectBuildStatistic(zDKEffectEnemy.zDKCalcNormalAttack, 0), zDKEffectManualFactor
    ));
    if (zDKEffectEnemy.zDKCalcUseSeparateAttackStats)
        zDKEffectEnemy.zDKCalcSuperAttack = static_cast<double>(zDKCalcApplyMultiplier(
            zDKEffectBuildStatistic(zDKEffectEnemy.zDKCalcSuperAttack, 0), zDKEffectManualFactor
        ));
    zDKEffectEnemy.zDKCalcAttackModifierPercent = 0;
    zDKEffectDefense =
        static_cast<double>(zDKEffectBuildStatistic(zDKEffectBossProfileData.zDKEffectBaseDefense, 1));
    for (int zDKEffectPhaseIndex = 0; zDKEffectPhaseIndex < 4; ++zDKEffectPhaseIndex)
        for (int zDKEffectStatIndex = 2; zDKEffectStatIndex < 4; ++zDKEffectStatIndex)
        {
            auto& zDKEffectSources = zDKEffectStatIndex == 2 ? zDKEffectEnemy.zDKCalcNormalAttackFactors
                                                             : zDKEffectEnemy.zDKCalcSuperAttackFactors;
            zDKEffectSources.push_back(
                std::max(
                    0.0, zDKCalcPercentMultiplier(zDKEffectSums[zDKEffectStatIndex][zDKEffectPhaseIndex])
                )
            );
            const auto& zDKEffectIndependent = zDKEffectFactors[zDKEffectStatIndex][zDKEffectPhaseIndex];
            zDKEffectSources.insert(
                zDKEffectSources.end(), zDKEffectIndependent.begin(), zDKEffectIndependent.end()
            );
        }
    return zDKEffectEnemy;
}

inline zDKEffectEvaluation zDKEffectEvaluate(
    const zDKEffectUnitProfile& zDKEffectUnitProfileData,
    const zDKEffectBossProfile& zDKEffectBossProfileData,
    const zDKEffectBattleState& zDKEffectBattleStateData,
    bool zDKEffectIncomingSuper = true
)
{
    zDKEffectValidate(zDKEffectUnitProfileData);
    zDKEffectValidate(zDKEffectBossProfileData);
    zDKEffectValidate(zDKEffectBattleStateData);
    zDKEffectEvaluation zDKEffectEvaluationData;
    zDKEffectEvaluationData.zDKEffectResolvedUnit = zDKEffectResolveUnit(
        zDKEffectUnitProfileData,
        zDKEffectBattleStateData,
        &zDKEffectEvaluationData.zDKEffectUnitEffectActivation,
        zDKEffectIncomingSuper
    );
    zDKEffectEvaluationData.zDKEffectResolvedBoss = zDKEffectResolveBoss(
        zDKEffectBossProfileData,
        zDKEffectBattleStateData,
        zDKEffectEvaluationData.zDKEffectBossDefense,
        &zDKEffectEvaluationData.zDKEffectBossEffectActivation
    );
    zDKEffectEvaluationData.zDKEffectResult = zDKCalcCalculateBattle(
        zDKEffectEvaluationData.zDKEffectResolvedUnit, zDKEffectEvaluationData.zDKEffectResolvedBoss
    );
    const auto zDKEffectNormalResult = zDKCalcCalculateBattle(
        zDKEffectResolveUnit(zDKEffectUnitProfileData, zDKEffectBattleStateData, nullptr, false),
        zDKEffectEvaluationData.zDKEffectResolvedBoss
    );
    const auto zDKEffectSuperResult = zDKCalcCalculateBattle(
        zDKEffectResolveUnit(zDKEffectUnitProfileData, zDKEffectBattleStateData, nullptr, true),
        zDKEffectEvaluationData.zDKEffectResolvedBoss
    );
    zDKEffectEvaluationData.zDKEffectResult.zDKCalcNormalAttackDamage =
        zDKEffectNormalResult.zDKCalcNormalAttackDamage;
    zDKEffectEvaluationData.zDKEffectResult.zDKCalcSuperAttackDamage =
        zDKEffectSuperResult.zDKCalcSuperAttackDamage;
    zDKEffectEvaluationData.zDKEffectNormalDefense = zDKEffectNormalResult.zDKCalcDefense;
    zDKEffectEvaluationData.zDKEffectSuperDefense = zDKEffectSuperResult.zDKCalcDefense;
    zDKEffectBattleState zDKEffectBefore = zDKEffectBattleStateData,
                         zDKEffectAfter = zDKEffectBattleStateData;
    zDKEffectBefore.zDKEffectAfterUnitAttack = false;
    zDKEffectAfter.zDKEffectAfterUnitAttack = true;
    double zDKEffectIgnored;
    zDKEffectEvaluationData.zDKEffectResult.zDKCalcDefense.zDKCalcBeforeAttack =
        zDKCalcCalculateBattle(
            zDKEffectResolveUnit(zDKEffectUnitProfileData, zDKEffectBefore, nullptr, zDKEffectIncomingSuper),
            zDKEffectResolveBoss(zDKEffectBossProfileData, zDKEffectBefore, zDKEffectIgnored)
        )
            .zDKCalcDefense.zDKCalcSelected;
    zDKEffectEvaluationData.zDKEffectResult.zDKCalcDefense.zDKCalcAfterAttack =
        zDKCalcCalculateBattle(
            zDKEffectResolveUnit(zDKEffectUnitProfileData, zDKEffectAfter, nullptr, zDKEffectIncomingSuper),
            zDKEffectResolveBoss(zDKEffectBossProfileData, zDKEffectAfter, zDKEffectIgnored)
        )
            .zDKCalcDefense.zDKCalcSelected;
    return zDKEffectEvaluationData;
}

inline void zDKStoreWriteEffect(
    std::ostream& zDKEffectOutputStream, const zDKEffectDefinition& zDKEffectEffectData
)
{
    zDKEffectOutputStream << std::quoted(zDKEffectEffectData.zDKEffectName) << ' '
                          << int(zDKEffectEffectData.zDKEffectCategory) << ' '
                          << int(zDKEffectEffectData.zDKEffectStat) << ' '
                          << int(zDKEffectEffectData.zDKEffectGroup) << ' '
                          << zDKEffectEffectData.zDKEffectValue << ' ' << zDKEffectEffectData.zDKEffectCount
                          << ' ' << zDKEffectEffectData.zDKEffectEnabled << ' '
                          << zDKEffectEffectData.zDKEffectMultiplicative << ' '
                          << zDKEffectEffectData.zDKEffectFirstTurn << ' '
                          << zDKEffectEffectData.zDKEffectLastTurn << ' '
                          << int(zDKEffectEffectData.zDKEffectHealthRule) << ' '
                          << zDKEffectEffectData.zDKEffectHealthThresholdPercent << ' '
                          << int(zDKEffectEffectData.zDKEffectTiming) << '\n';
}

inline bool zDKStoreReadEffect(
    std::istream& zDKEffectInputStream, zDKEffectDefinition& zDKEffectEffectData, bool zDKEffectIsBoss
)
{
    int zDKEffectCategoryIndex, zDKEffectStatisticIndex, zDKEffectDefensePhaseIndex, zDKEffectHealthPercent,
        zDKEffectTiming;
    if (!(zDKEffectInputStream >> std::quoted(zDKEffectEffectData.zDKEffectName) >> zDKEffectCategoryIndex >>
          zDKEffectStatisticIndex >> zDKEffectDefensePhaseIndex >> zDKEffectEffectData.zDKEffectValue >>
          zDKEffectEffectData.zDKEffectCount >> zDKEffectEffectData.zDKEffectEnabled >>
          zDKEffectEffectData.zDKEffectMultiplicative >> zDKEffectEffectData.zDKEffectFirstTurn >>
          zDKEffectEffectData.zDKEffectLastTurn >> zDKEffectHealthPercent >>
          zDKEffectEffectData.zDKEffectHealthThresholdPercent >> zDKEffectTiming))
        return false;
    zDKEffectEffectData.zDKEffectCategory = zDKEffectCategory(zDKEffectCategoryIndex);
    zDKEffectEffectData.zDKEffectStat = zDKEffectStat(zDKEffectStatisticIndex);
    zDKEffectEffectData.zDKEffectGroup = zDKEffectDefensePhase(zDKEffectDefensePhaseIndex);
    zDKEffectEffectData.zDKEffectHealthRule = zDKEffectHealthRule(zDKEffectHealthPercent);
    zDKEffectEffectData.zDKEffectTiming = zDKEffectAttackTiming(zDKEffectTiming);
    if (zDKEffectEffectData.zDKEffectCategory == zDKEffectCategory::zDKEffectGuard)
    {
        zDKEffectEffectData.zDKEffectCategory = zDKEffectCategory::zDKEffectBasic;
        zDKEffectEffectData.zDKEffectStat = zDKEffectStat::zDKEffectGuard;
        zDKEffectEffectData.zDKEffectMultiplicative = false;
    }
    try
    {
        zDKEffectValidateEffect(zDKEffectEffectData, zDKEffectIsBoss);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

inline std::string zDKStoreSaveUnit(const zDKEffectUnitProfile& zDKEffectUnitProfileData)
{
    zDKEffectValidate(zDKEffectUnitProfileData);
    std::ostringstream zDKEffectOutputStream;
    zDKEffectOutputStream << std::setprecision(17) << "DOKKAN_UNIT 4\n"
                          << std::quoted(zDKEffectUnitProfileData.zDKEffectName) << '\n';
    zDKEffectOutputStream << zDKEffectUnitProfileData.zDKEffectBaseDefense << ' '
                          << zDKEffectUnitProfileData.zDKEffectLeaderSkillPercent << ' '
                          << zDKEffectUnitProfileData.zDKEffectTypeDefenseBoostLevel << ' '
                          << zDKEffectUnitProfileData.zDKEffectManualDefense << ' '
                          << zDKEffectUnitProfileData.zDKEffectUseManualDefense << ' ' << true << ' '
                          << int(zDKEffectUnitProfileData.zDKEffectElementType) << ' '
                          << int(zDKEffectUnitProfileData.zDKEffectCombatClass) << '\n'
                          << zDKEffectUnitProfileData.zDKEffectEffects.size() << '\n';
    for (const auto& zDKEffectEntry : zDKEffectUnitProfileData.zDKEffectEffects)
        zDKStoreWriteEffect(zDKEffectOutputStream, zDKEffectEntry);
    return zDKEffectOutputStream.str();
}

inline bool zDKStoreLoadUnit(std::istream& zDKEffectInputStream, zDKEffectUnitProfile& zDKEffectOutput)
{
    std::string zDKEffectMagic;
    int zDKEffectVersion, zDKEffectTypeIndex, zDKEffectClassIndex;
    size_t zDKEffectCount;
    zDKEffectUnitProfile zDKEffectUnitProfileData;
    if (!(zDKEffectInputStream >> zDKEffectMagic >> zDKEffectVersion) || zDKEffectMagic != "DOKKAN_UNIT" ||
        (zDKEffectVersion < 2 || zDKEffectVersion > 4))
        return false;
    if (!(zDKEffectInputStream >> std::quoted(zDKEffectUnitProfileData.zDKEffectName) >>
          zDKEffectUnitProfileData.zDKEffectBaseDefense >>
          zDKEffectUnitProfileData.zDKEffectLeaderSkillPercent >>
          zDKEffectUnitProfileData.zDKEffectTypeDefenseBoostLevel >>
          zDKEffectUnitProfileData.zDKEffectManualDefense >>
          zDKEffectUnitProfileData.zDKEffectUseManualDefense >>
          zDKEffectUnitProfileData.zDKEffectRoundDownDefense >> zDKEffectTypeIndex >> zDKEffectClassIndex >>
          zDKEffectCount) ||
        zDKEffectCount > 128)
        return false;
    zDKEffectUnitProfileData.zDKEffectRoundDownDefense = true;
    zDKEffectUnitProfileData.zDKEffectElementType = zDKCalcUnitType(zDKEffectTypeIndex);
    zDKEffectUnitProfileData.zDKEffectCombatClass = zDKCalcUnitClass(zDKEffectClassIndex);
    zDKEffectUnitProfileData.zDKEffectEffects.resize(zDKEffectCount);
    for (auto& zDKEffectEntry : zDKEffectUnitProfileData.zDKEffectEffects)
        if (!zDKStoreReadEffect(zDKEffectInputStream, zDKEffectEntry, false))
            return false;
    try
    {
        zDKEffectValidate(zDKEffectUnitProfileData);
    }
    catch (...)
    {
        return false;
    }
    zDKEffectOutput = std::move(zDKEffectUnitProfileData);
    return true;
}

inline std::string zDKStoreSaveSession(
    const zDKEffectUnitProfile& zDKEffectUnitProfileData,
    const zDKEffectBossProfile& zDKEffectBossProfileData,
    const zDKEffectBattleState& zDKEffectBattleStateData,
    bool zDKEffectAnimationsEnabled,
    int zDKEffectAttack
)
{
    zDKEffectEvaluate(zDKEffectUnitProfileData, zDKEffectBossProfileData, zDKEffectBattleStateData);
    std::ostringstream zDKEffectOutputStream;
    zDKEffectOutputStream << std::setprecision(17) << "DOKKAN_SESSION 5\n"
                          << zDKStoreSaveUnit(zDKEffectUnitProfileData)
                          << std::quoted(zDKEffectBossProfileData.zDKEffectName) << '\n';
    const zDKCalcEnemyStats& zDKEffectEnemyStatsData = zDKEffectBossProfileData.zDKEffectStats;
    zDKEffectOutputStream << zDKEffectEnemyStatsData.zDKCalcNormalAttack << ' '
                          << zDKEffectEnemyStatsData.zDKCalcSuperAttackMultiplier << ' '
                          << zDKEffectEnemyStatsData.zDKCalcAttackModifierPercent << ' '
                          << int(zDKEffectEnemyStatsData.zDKCalcElementType) << ' '
                          << int(zDKEffectEnemyStatsData.zDKCalcCombatClass) << ' '
                          << zDKEffectEnemyStatsData.zDKCalcIsCritical << ' '
                          << zDKEffectEnemyStatsData.zDKCalcUseCustomCriticalType << ' '
                          << zDKEffectEnemyStatsData.zDKCalcCriticalMultiplier << ' '
                          << zDKEffectEnemyStatsData.zDKCalcCriticalDefenseIgnoredPercent << ' '
                          << zDKEffectEnemyStatsData.zDKCalcCriticalTypeMultiplier << ' '
                          << zDKEffectEnemyStatsData.zDKCalcCriticalGuardMultiplier << ' '
                          << zDKEffectEnemyStatsData.zDKCalcDefenseIgnoredPercent << ' '
                          << zDKEffectBossProfileData.zDKEffectBaseDefense << ' '
                          << zDKEffectBossProfileData.zDKEffectHealthPercent << ' '
                          << zDKEffectEnemyStatsData.zDKCalcUseSeparateAttackStats << ' '
                          << zDKEffectEnemyStatsData.zDKCalcSuperAttack << ' '
                          << zDKEffectEnemyStatsData.zDKCalcPassiveAttackDownPercent << ' '
                          << zDKEffectEnemyStatsData.zDKCalcItemAttackDownPercent << ' '
                          << zDKEffectEnemyStatsData.zDKCalcSuperAttackDownPercent << ' '
                          << zDKEffectEnemyStatsData.zDKCalcEnemySuperRaisePercent << ' '
                          << zDKEffectEnemyStatsData.zDKCalcEnemyAlreadySupered << '\n'
                          << zDKEffectBossProfileData.zDKEffectEffects.size() << '\n';
    for (const auto& zDKEffectCurrentEffect : zDKEffectBossProfileData.zDKEffectEffects)
        zDKStoreWriteEffect(zDKEffectOutputStream, zDKEffectCurrentEffect);
    zDKEffectOutputStream << zDKEffectBattleStateData.zDKEffectTurn << ' '
                          << zDKEffectBattleStateData.zDKEffectMaxHP << ' '
                          << zDKEffectBattleStateData.zDKEffectCurrentHP << ' '
                          << zDKEffectBattleStateData.zDKEffectAfterUnitAttack << ' '
                          << zDKEffectAnimationsEnabled << ' ' << zDKEffectAttack << '\n';
    return zDKEffectOutputStream.str();
}

inline bool zDKStoreLoadSession(
    std::istream& zDKEffectInputStream,
    zDKEffectUnitProfile& zDKEffectUnitProfileData,
    zDKEffectBossProfile& zDKEffectBossProfileData,
    zDKEffectBattleState& zDKEffectBattleStateData,
    bool& zDKEffectAnimationsEnabled,
    int& zDKEffectAttack
)
{
    zDKEffectUnitProfile zDKEffectLoadedUnitProfile;
    zDKEffectBossProfile zDKEffectLoadedBossProfile;
    zDKEffectBattleState zDKEffectLoadedBattleState;
    bool zDKEffectLoadedAnimationEnabled;
    int zDKEffectLoadedAttackSelection, zDKEffectTypeIndex, zDKEffectClassIndex, zDKEffectVersion;
    size_t zDKEffectCount;
    std::string zDKEffectMagic;
    if (!(zDKEffectInputStream >> zDKEffectMagic >> zDKEffectVersion) || zDKEffectMagic != "DOKKAN_SESSION" ||
        (zDKEffectVersion < 2 || zDKEffectVersion > 5) ||
        !zDKStoreLoadUnit(zDKEffectInputStream, zDKEffectLoadedUnitProfile))
        return false;
    auto& zDKEffectEnemyStatsData = zDKEffectLoadedBossProfile.zDKEffectStats;
    if (!(zDKEffectInputStream >> std::quoted(zDKEffectLoadedBossProfile.zDKEffectName) >>
          zDKEffectEnemyStatsData.zDKCalcNormalAttack >>
          zDKEffectEnemyStatsData.zDKCalcSuperAttackMultiplier >>
          zDKEffectEnemyStatsData.zDKCalcAttackModifierPercent >> zDKEffectTypeIndex >> zDKEffectClassIndex >>
          zDKEffectEnemyStatsData.zDKCalcIsCritical >> zDKEffectEnemyStatsData.zDKCalcUseCustomCriticalType >>
          zDKEffectEnemyStatsData.zDKCalcCriticalMultiplier >>
          zDKEffectEnemyStatsData.zDKCalcCriticalDefenseIgnoredPercent >>
          zDKEffectEnemyStatsData.zDKCalcCriticalTypeMultiplier >>
          zDKEffectEnemyStatsData.zDKCalcCriticalGuardMultiplier >>
          zDKEffectEnemyStatsData.zDKCalcDefenseIgnoredPercent >>
          zDKEffectLoadedBossProfile.zDKEffectBaseDefense >>
          zDKEffectLoadedBossProfile.zDKEffectHealthPercent))
        return false;
    if (zDKEffectVersion >= 3)
    {
        if (!(zDKEffectInputStream >> zDKEffectEnemyStatsData.zDKCalcUseSeparateAttackStats >>
              zDKEffectEnemyStatsData.zDKCalcSuperAttack))
            return false;
    }
    else
    {
        zDKEffectEnemyStatsData.zDKCalcUseSeparateAttackStats = true;
        zDKEffectEnemyStatsData.zDKCalcSuperAttack = zDKEffectEnemyStatsData.zDKCalcNormalAttack *
            zDKEffectEnemyStatsData.zDKCalcSuperAttackMultiplier;
    }
    if (zDKEffectVersion >= 4 &&
        !(zDKEffectInputStream >> zDKEffectEnemyStatsData.zDKCalcPassiveAttackDownPercent >>
          zDKEffectEnemyStatsData.zDKCalcItemAttackDownPercent >>
          zDKEffectEnemyStatsData.zDKCalcSuperAttackDownPercent >>
          zDKEffectEnemyStatsData.zDKCalcEnemySuperRaisePercent >>
          zDKEffectEnemyStatsData.zDKCalcEnemyAlreadySupered))
        return false;
    if (!(zDKEffectInputStream >> zDKEffectCount) || zDKEffectCount > 128)
        return false;
    zDKEffectEnemyStatsData.zDKCalcElementType = zDKCalcUnitType(zDKEffectTypeIndex);
    zDKEffectEnemyStatsData.zDKCalcCombatClass = zDKCalcUnitClass(zDKEffectClassIndex);
    zDKEffectLoadedBossProfile.zDKEffectEffects.resize(zDKEffectCount);
    for (auto& zDKEffectEntry : zDKEffectLoadedBossProfile.zDKEffectEffects)
        if (!zDKStoreReadEffect(zDKEffectInputStream, zDKEffectEntry, true))
            return false;
    if (!(zDKEffectInputStream >> zDKEffectLoadedBattleState.zDKEffectTurn >>
          zDKEffectLoadedBattleState.zDKEffectMaxHP >> zDKEffectLoadedBattleState.zDKEffectCurrentHP >>
          zDKEffectLoadedBattleState.zDKEffectAfterUnitAttack >> zDKEffectLoadedAnimationEnabled >>
          zDKEffectLoadedAttackSelection) ||
        (zDKEffectLoadedAttackSelection != 0 && zDKEffectLoadedAttackSelection != 1))
        return false;
    try
    {
        zDKEffectEvaluate(zDKEffectLoadedUnitProfile, zDKEffectLoadedBossProfile, zDKEffectLoadedBattleState);
    }
    catch (...)
    {
        return false;
    }
    zDKEffectUnitProfileData = std::move(zDKEffectLoadedUnitProfile);
    zDKEffectBossProfileData = std::move(zDKEffectLoadedBossProfile);
    zDKEffectBattleStateData = zDKEffectLoadedBattleState;
    zDKEffectAnimationsEnabled = zDKEffectLoadedAnimationEnabled;
    zDKEffectAttack = zDKEffectLoadedAttackSelection;
    return true;
}
} // namespace zDK
