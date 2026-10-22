#pragma once
#include "zDKIntegerMath.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace zDK
{
enum class zDKCalcUnitType
{
    zDKCalcAGL,
    zDKCalcTEQ,
    zDKCalcINT,
    zDKCalcSTR,
    zDKCalcPHY
};
enum class zDKCalcUnitClass
{
    zDKCalcSuper,
    zDKCalcExtreme
};
enum class zDKCalcTypeRelation
{
    zDKCalcAdvantage,
    zDKCalcNeutral,
    zDKCalcDisadvantage
};

struct zDKCalcUnitStats
{
    double zDKCalcBaseDefense = 12269, zDKCalcLeaderSkillPercent = 440, zDKCalcStartOfTurnPercent = 200;
    double zDKCalcDomainPercent = 0, zDKCalcItemPercent = 0, zDKCalcLinkSkillPercent = 0,
           zDKCalcActiveSkillPercent = 0, zDKCalcOnAttackPercent = 0;
    double zDKCalcSuperAttackStackCount = 9, zDKCalcStackDefensePercent = 50,
           zDKCalcCurrentSuperAttackCount = 0, zDKCalcSuperAttackDefensePercent = 30,
           zDKCalcOtherSuperAttackPercent = 0;
    // RoundDownDefense is retained for source/file compatibility; all arithmetic always truncates.
    bool zDKCalcUseManualDefense = false, zDKCalcAfterUnitAttack = true, zDKCalcRoundDownDefense = true,
         zDKCalcGuardsAllAttacks = false;
    double zDKCalcManualDefense = 400000, zDKCalcDamageReductionPercent = 0,
           zDKCalcNormalDamageReductionPercent = 0, zDKCalcSuperDamageReductionPercent = 0,
           zDKCalcTypeDefenseBoostLevel = 0;
    double zDKCalcDefenseDebuffPercent = 0, zDKCalcTeamHealth = 700000;
    double zDKCalcDamageReceivedPercent = 0;
    zDKCalcUnitType zDKCalcElementType = zDKCalcUnitType::zDKCalcAGL;
    zDKCalcUnitClass zDKCalcCombatClass = zDKCalcUnitClass::zDKCalcSuper;
    std::array<std::vector<double>, 8> zDKCalcIndependentDefenseFactors;
    // Resolved source factors preserve each independent rounding boundary.
    std::array<std::vector<double>, 4> zDKCalcReductionFactors;
    std::vector<double> zDKCalcDamageReceivedFactors;
};

struct zDKCalcEnemyStats
{
    double zDKCalcNormalAttack = 1000000, zDKCalcSuperAttackMultiplier = 3, zDKCalcAttackModifierPercent = 0;
    bool zDKCalcUseSeparateAttackStats = false;
    double zDKCalcSuperAttack = 0;
    zDKCalcUnitType zDKCalcElementType = zDKCalcUnitType::zDKCalcINT;
    zDKCalcUnitClass zDKCalcCombatClass = zDKCalcUnitClass::zDKCalcSuper;
    bool zDKCalcIsCritical = false, zDKCalcUseCustomCriticalType = false;
    double zDKCalcCriticalMultiplier = 1.5, zDKCalcCriticalDefenseIgnoredPercent = 0;
    double zDKCalcCriticalTypeMultiplier = 1, zDKCalcCriticalGuardMultiplier = 1;
    double zDKCalcNormalAttackFactor = 1, zDKCalcSuperAttackFactor = 1, zDKCalcDefenseIgnoredPercent = 0;
    double zDKCalcPassiveAttackDownPercent = 0, zDKCalcItemAttackDownPercent = 0;
    double zDKCalcSuperAttackDownPercent = 0, zDKCalcEnemySuperRaisePercent = 0;
    bool zDKCalcEnemyAlreadySupered = false;
    std::vector<double> zDKCalcNormalAttackFactors, zDKCalcSuperAttackFactors;
};

struct zDKCalcDefenseStep
{
    const char* zDKCalcName;
    double zDKCalcMultiplier, zDKCalcValue;
};

struct zDKCalcDefenseBreakdown
{
    double zDKCalcBeforeAttack = 0, zDKCalcAfterAttack = 0, zDKCalcSelected = 0, zDKCalcEffective = 0;
    std::array<zDKCalcDefenseStep, 8> zDKCalcSteps{};
};

struct zDKCalcDamageRange
{
    double zDKCalcMinimumDamage = 0, zDKCalcAverageDamage = 0, zDKCalcMaximumDamage = 0,
           zDKCalcAdjustedAttack = 0, zDKCalcRequiredDefense = 0;
};

struct zDKCalcBattleResult
{
    zDKCalcDefenseBreakdown zDKCalcDefense;
    zDKCalcTypeRelation zDKCalcMatchupRelation;
    double zDKCalcTypeMultiplier = 1, zDKCalcGuardMultiplier = 1;
    bool zDKCalcTypeDefenseBoostActive = false;
    zDKCalcDamageRange zDKCalcNormalAttackDamage, zDKCalcSuperAttackDamage;
};

inline double zDKCalcNormalAttackStat(const zDKCalcEnemyStats& zDKCalcEnemyStatsData)
{
    const double zDKCalcSuperEffect = zDKCalcEnemyStatsData.zDKCalcUseSeparateAttackStats
        ? 0
        : ((zDKCalcEnemyStatsData.zDKCalcEnemyAlreadySupered
                ? zDKCalcEnemyStatsData.zDKCalcEnemySuperRaisePercent
                : 0) -
           zDKCalcEnemyStatsData.zDKCalcSuperAttackDownPercent) /
            100;
    auto zDKCalcAttack = zDKCalcApplyMultiplier(
        zDKCalcFloorStatistic(zDKCalcEnemyStatsData.zDKCalcNormalAttack),
        std::max(0.0, 1 + zDKCalcSuperEffect)
    );
    zDKCalcAttack = zDKCalcApplyMultiplier(zDKCalcAttack, zDKCalcEnemyStatsData.zDKCalcNormalAttackFactor);
    for (double zDKCalcFactor : zDKCalcEnemyStatsData.zDKCalcNormalAttackFactors)
        zDKCalcAttack = zDKCalcApplyMultiplier(zDKCalcAttack, zDKCalcFactor);
    zDKCalcAttack = zDKCalcApplyMultiplier(
        zDKCalcAttack, 1 - zDKCalcEnemyStatsData.zDKCalcPassiveAttackDownPercent / 100
    );
    return static_cast<double>(
        zDKCalcApplyMultiplier(zDKCalcAttack, 1 - zDKCalcEnemyStatsData.zDKCalcItemAttackDownPercent / 100)
    );
}

inline double zDKCalcSuperAttackStat(const zDKCalcEnemyStats& zDKCalcEnemyStatsData)
{
    const double zDKCalcSuperMultiplier = std::max(
        0.0,
        zDKCalcEnemyStatsData.zDKCalcSuperAttackMultiplier +
            (zDKCalcEnemyStatsData.zDKCalcEnemySuperRaisePercent -
             zDKCalcEnemyStatsData.zDKCalcSuperAttackDownPercent) /
                100
    );
    auto zDKCalcAttack = zDKCalcEnemyStatsData.zDKCalcUseSeparateAttackStats
        ? zDKCalcFloorStatistic(zDKCalcEnemyStatsData.zDKCalcSuperAttack)
        : zDKCalcApplyMultiplier(
              zDKCalcFloorStatistic(zDKCalcEnemyStatsData.zDKCalcNormalAttack), zDKCalcSuperMultiplier
          );
    zDKCalcAttack = zDKCalcApplyMultiplier(zDKCalcAttack, zDKCalcEnemyStatsData.zDKCalcSuperAttackFactor);
    for (double zDKCalcFactor : zDKCalcEnemyStatsData.zDKCalcSuperAttackFactors)
        zDKCalcAttack = zDKCalcApplyMultiplier(zDKCalcAttack, zDKCalcFactor);
    zDKCalcAttack = zDKCalcApplyMultiplier(
        zDKCalcAttack, 1 - zDKCalcEnemyStatsData.zDKCalcPassiveAttackDownPercent / 100
    );
    return static_cast<double>(
        zDKCalcApplyMultiplier(zDKCalcAttack, 1 - zDKCalcEnemyStatsData.zDKCalcItemAttackDownPercent / 100)
    );
}

inline zDKCalcTypeRelation zDKCalcResolveTypeRelation(
    zDKCalcUnitType zDKCalcUnit, zDKCalcUnitType zDKCalcEnemy
)
{
    constexpr zDKCalcUnitType zDKCalcTypeAdvantages[] = {
        zDKCalcUnitType::zDKCalcSTR,
        zDKCalcUnitType::zDKCalcAGL,
        zDKCalcUnitType::zDKCalcTEQ,
        zDKCalcUnitType::zDKCalcPHY,
        zDKCalcUnitType::zDKCalcINT
    };
    if (zDKCalcTypeAdvantages[static_cast<int>(zDKCalcUnit)] == zDKCalcEnemy)
        return zDKCalcTypeRelation::zDKCalcAdvantage;
    if (zDKCalcTypeAdvantages[static_cast<int>(zDKCalcEnemy)] == zDKCalcUnit)
        return zDKCalcTypeRelation::zDKCalcDisadvantage;
    return zDKCalcTypeRelation::zDKCalcNeutral;
}

inline double zDKCalcPercentMultiplier(double zDKCalcNumber)
{
    return 1 + zDKCalcNumber / 100;
}

inline void zDKCalcValidateRange(
    double zDKCalcNumber, double zDKCalcMinimum, double zDKCalcMaximum, const char* zDKCalcName
)
{
    if (!std::isfinite(zDKCalcNumber) || zDKCalcNumber < zDKCalcMinimum || zDKCalcNumber > zDKCalcMaximum)
        throw std::invalid_argument(zDKCalcName);
}

inline zDKCalcBattleResult zDKCalcCalculateBattle(
    const zDKCalcUnitStats& zDKCalcUnitStatsData, const zDKCalcEnemyStats& zDKCalcEnemyStatsData
)
{
    for (double zDKCalcNumber :
         {zDKCalcUnitStatsData.zDKCalcBaseDefense,
          zDKCalcUnitStatsData.zDKCalcManualDefense,
          zDKCalcEnemyStatsData.zDKCalcNormalAttack,
          zDKCalcUnitStatsData.zDKCalcTeamHealth})
        zDKCalcValidateRange(zDKCalcNumber, 0, 1e12, "Base statistics must be between 0 and 1 trillion.");
    for (double zDKCalcNumber :
         {zDKCalcUnitStatsData.zDKCalcLeaderSkillPercent,
          zDKCalcUnitStatsData.zDKCalcStartOfTurnPercent,
          zDKCalcUnitStatsData.zDKCalcDomainPercent,
          zDKCalcUnitStatsData.zDKCalcItemPercent,
          zDKCalcUnitStatsData.zDKCalcLinkSkillPercent,
          zDKCalcUnitStatsData.zDKCalcActiveSkillPercent,
          zDKCalcUnitStatsData.zDKCalcOnAttackPercent,
          zDKCalcUnitStatsData.zDKCalcStackDefensePercent,
          zDKCalcUnitStatsData.zDKCalcSuperAttackDefensePercent,
          zDKCalcUnitStatsData.zDKCalcOtherSuperAttackPercent})
        zDKCalcValidateRange(zDKCalcNumber, 0, 1e6, "DEF bonuses must be between 0 and 1,000,000%.");
    for (double zDKCalcNumber :
         {zDKCalcUnitStatsData.zDKCalcDamageReductionPercent,
          zDKCalcUnitStatsData.zDKCalcNormalDamageReductionPercent,
          zDKCalcUnitStatsData.zDKCalcSuperDamageReductionPercent,
          zDKCalcUnitStatsData.zDKCalcDefenseDebuffPercent,
          zDKCalcEnemyStatsData.zDKCalcCriticalDefenseIgnoredPercent,
          zDKCalcEnemyStatsData.zDKCalcDefenseIgnoredPercent})
        zDKCalcValidateRange(zDKCalcNumber, 0, 100, "Percentages must be between 0 and 100%.");
    for (double zDKCalcNumber :
         {zDKCalcUnitStatsData.zDKCalcSuperAttackStackCount,
          zDKCalcUnitStatsData.zDKCalcCurrentSuperAttackCount,
          zDKCalcUnitStatsData.zDKCalcTypeDefenseBoostLevel})
    {
        zDKCalcValidateRange(zDKCalcNumber, 0, 10000, "Counts must be between 0 and 10,000.");
        if (std::floor(zDKCalcNumber) != zDKCalcNumber)
            throw std::invalid_argument("Counts must be whole numbers.");
    }
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcSuperAttackMultiplier,
        0,
        1000,
        "Super Attack multiplier must be between 0 and 1,000."
    );
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcSuperAttack,
        0,
        1e15,
        "Super Attack ATK must be between 0 and 1 quadrillion."
    );
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcAttackModifierPercent,
        -100,
        1e6,
        "ATK modifier is outside the supported range."
    );
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcCriticalMultiplier,
        0,
        1000,
        "Critical multiplier is outside the supported range."
    );
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcCriticalTypeMultiplier,
        0,
        10,
        "Critical type multiplier must be between 0 and 10."
    );
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcCriticalGuardMultiplier,
        0,
        1,
        "Critical guard multiplier must be between 0 and 1."
    );
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcNormalAttackFactor,
        0,
        1e12,
        "Normal attack factor is outside the supported range."
    );
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcSuperAttackFactor,
        0,
        1e12,
        "Super attack factor is outside the supported range."
    );
    for (const auto& zDKCalcDefensePhaseIndex : zDKCalcUnitStatsData.zDKCalcIndependentDefenseFactors)
        for (double zDKCalcFactor : zDKCalcDefensePhaseIndex)
            zDKCalcValidateRange(
                zDKCalcFactor, 0, 1e6, "Independent DEF factor is outside the supported range."
            );
    for (double zDKCalcPercent :
         {zDKCalcEnemyStatsData.zDKCalcPassiveAttackDownPercent,
          zDKCalcEnemyStatsData.zDKCalcItemAttackDownPercent})
        zDKCalcValidateRange(zDKCalcPercent, 0, 100, "ATK Down must be between 0 and 100%.");
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcSuperAttackDownPercent,
        0,
        10000,
        "SA-effect ATK Down must be between 0 and 10,000%."
    );
    zDKCalcValidateRange(
        zDKCalcEnemyStatsData.zDKCalcEnemySuperRaisePercent,
        0,
        10000,
        "Enemy SA ATK raise must be between 0 and 10,000%."
    );
    zDKCalcValidateRange(
        zDKCalcUnitStatsData.zDKCalcDamageReceivedPercent,
        0,
        1e6,
        "Damage received increase must be between 0 and 1,000,000%."
    );
    if (zDKCalcEnemyStatsData.zDKCalcUseSeparateAttackStats &&
        (zDKCalcEnemyStatsData.zDKCalcSuperAttackDownPercent != 0 ||
         zDKCalcEnemyStatsData.zDKCalcEnemySuperRaisePercent != 0))
        throw std::invalid_argument(
            "SA effects require Base ATK + SA multiplier mode; direct ATK must already include them."
        );
    if (static_cast<int>(zDKCalcUnitStatsData.zDKCalcElementType) < 0 ||
        static_cast<int>(zDKCalcUnitStatsData.zDKCalcElementType) > 4 ||
        static_cast<int>(zDKCalcEnemyStatsData.zDKCalcElementType) < 0 ||
        static_cast<int>(zDKCalcEnemyStatsData.zDKCalcElementType) > 4)
        throw std::invalid_argument("Invalid type.");
    zDKCalcBattleResult zDKCalcBattleResultData{};
    auto zDKCalcBuildDefense = [&](bool zDKCalcAfter, bool zDKCalcCapture)
    {
        auto zDKCalcValue = zDKCalcFloorStatistic(zDKCalcUnitStatsData.zDKCalcBaseDefense);
        const double zDKCalcBonuses[] = {
            zDKCalcUnitStatsData.zDKCalcLeaderSkillPercent,
            zDKCalcUnitStatsData.zDKCalcStartOfTurnPercent,
            zDKCalcUnitStatsData.zDKCalcDomainPercent,
            zDKCalcUnitStatsData.zDKCalcItemPercent,
            zDKCalcUnitStatsData.zDKCalcLinkSkillPercent,
            zDKCalcUnitStatsData.zDKCalcActiveSkillPercent,
            zDKCalcAfter ? zDKCalcUnitStatsData.zDKCalcOnAttackPercent : 0,
            zDKCalcUnitStatsData.zDKCalcSuperAttackStackCount *
                    zDKCalcUnitStatsData.zDKCalcStackDefensePercent +
                zDKCalcUnitStatsData.zDKCalcOtherSuperAttackPercent +
                (zDKCalcAfter ? zDKCalcUnitStatsData.zDKCalcCurrentSuperAttackCount *
                         zDKCalcUnitStatsData.zDKCalcSuperAttackDefensePercent
                              : 0)
        };
        const char* zDKCalcNames[] = {
            "Leader Skill",
            "Start-of-turn passive & support",
            "Domain",
            "Support Items",
            "Link Skills",
            "Active Skill",
            "Mid-battle passive (Phase 2)",
            "Super Attack Effect"
        };
        for (int zDKCalcIndex = 0; zDKCalcIndex < 8; ++zDKCalcIndex)
        {
            zDKCalcValue =
                zDKCalcApplyMultiplier(zDKCalcValue, zDKCalcPercentMultiplier(zDKCalcBonuses[zDKCalcIndex]));
            double zDKCalcMultiplier = zDKCalcPercentMultiplier(zDKCalcBonuses[zDKCalcIndex]);
            if (zDKCalcIndex != 6 || zDKCalcAfter)
                for (double zDKCalcFactor :
                     zDKCalcUnitStatsData.zDKCalcIndependentDefenseFactors[zDKCalcIndex])
                {
                    zDKCalcValue = zDKCalcApplyMultiplier(zDKCalcValue, zDKCalcFactor);
                    zDKCalcMultiplier *= zDKCalcFactor;
                }
            if (zDKCalcCapture)
                zDKCalcBattleResultData.zDKCalcDefense.zDKCalcSteps[zDKCalcIndex] = {
                    zDKCalcNames[zDKCalcIndex], zDKCalcMultiplier, static_cast<double>(zDKCalcValue)
                };
        }
        return static_cast<double>(zDKCalcValue);
    };
    zDKCalcBattleResultData.zDKCalcDefense.zDKCalcBeforeAttack = zDKCalcUnitStatsData.zDKCalcUseManualDefense
        ? static_cast<double>(zDKCalcFloorStatistic(zDKCalcUnitStatsData.zDKCalcManualDefense))
        : zDKCalcBuildDefense(false, !zDKCalcUnitStatsData.zDKCalcAfterUnitAttack);
    zDKCalcBattleResultData.zDKCalcDefense.zDKCalcAfterAttack = zDKCalcUnitStatsData.zDKCalcUseManualDefense
        ? static_cast<double>(zDKCalcFloorStatistic(zDKCalcUnitStatsData.zDKCalcManualDefense))
        : zDKCalcBuildDefense(true, zDKCalcUnitStatsData.zDKCalcAfterUnitAttack);
    zDKCalcBattleResultData.zDKCalcDefense.zDKCalcSelected = zDKCalcUnitStatsData.zDKCalcAfterUnitAttack
        ? zDKCalcBattleResultData.zDKCalcDefense.zDKCalcAfterAttack
        : zDKCalcBattleResultData.zDKCalcDefense.zDKCalcBeforeAttack;
    auto zDKCalcApplyReduction = [&](zDKCalcInteger zDKCalcValue, int zDKCalcSlot, double zDKCalcPercent)
    {
        const auto& zDKCalcFactors = zDKCalcUnitStatsData.zDKCalcReductionFactors[zDKCalcSlot];
        if (zDKCalcFactors.empty())
            return zDKCalcApplyMultiplier(zDKCalcValue, 1 - zDKCalcPercent / 100);
        for (double zDKCalcFactor : zDKCalcFactors)
            zDKCalcValue = zDKCalcApplyMultiplier(zDKCalcValue, zDKCalcFactor);
        return zDKCalcValue;
    };
    auto zDKCalcEffectiveDefense = zDKCalcApplyReduction(
        zDKCalcFloorStatistic(zDKCalcBattleResultData.zDKCalcDefense.zDKCalcSelected),
        3,
        zDKCalcUnitStatsData.zDKCalcDefenseDebuffPercent
    );
    zDKCalcEffectiveDefense = zDKCalcApplyMultiplier(
        zDKCalcEffectiveDefense, 1 - zDKCalcEnemyStatsData.zDKCalcDefenseIgnoredPercent / 100
    );
    if (zDKCalcEnemyStatsData.zDKCalcIsCritical)
        zDKCalcEffectiveDefense = zDKCalcApplyMultiplier(
            zDKCalcEffectiveDefense, 1 - zDKCalcEnemyStatsData.zDKCalcCriticalDefenseIgnoredPercent / 100
        );
    zDKCalcBattleResultData.zDKCalcDefense.zDKCalcEffective = static_cast<double>(zDKCalcEffectiveDefense);
    zDKCalcBattleResultData.zDKCalcMatchupRelation = zDKCalcResolveTypeRelation(
        zDKCalcUnitStatsData.zDKCalcElementType, zDKCalcEnemyStatsData.zDKCalcElementType
    );
    const bool zDKCalcSame =
        zDKCalcUnitStatsData.zDKCalcCombatClass == zDKCalcEnemyStatsData.zDKCalcCombatClass;
    if (zDKCalcUnitStatsData.zDKCalcGuardsAllAttacks)
    {
        zDKCalcBattleResultData.zDKCalcTypeMultiplier = .8;
        zDKCalcBattleResultData.zDKCalcGuardMultiplier = .5;
    }
    else if (zDKCalcBattleResultData.zDKCalcMatchupRelation == zDKCalcTypeRelation::zDKCalcAdvantage)
    {
        zDKCalcBattleResultData.zDKCalcTypeMultiplier = zDKCalcSame ? .9 : 1;
        zDKCalcBattleResultData.zDKCalcGuardMultiplier = .5;
    }
    else if (zDKCalcBattleResultData.zDKCalcMatchupRelation == zDKCalcTypeRelation::zDKCalcDisadvantage)
        zDKCalcBattleResultData.zDKCalcTypeMultiplier = zDKCalcSame ? 1.25 : 1.5;
    else
        zDKCalcBattleResultData.zDKCalcTypeMultiplier = zDKCalcSame ? 1 : 1.15;
    zDKCalcBattleResultData.zDKCalcTypeDefenseBoostActive =
        zDKCalcBattleResultData.zDKCalcMatchupRelation == zDKCalcTypeRelation::zDKCalcAdvantage;
    if (zDKCalcBattleResultData.zDKCalcTypeDefenseBoostActive)
        zDKCalcBattleResultData.zDKCalcTypeMultiplier = std::max(
            0.0,
            zDKCalcBattleResultData.zDKCalcTypeMultiplier -
                zDKCalcUnitStatsData.zDKCalcTypeDefenseBoostLevel * .01
        );
    if (zDKCalcEnemyStatsData.zDKCalcIsCritical && zDKCalcEnemyStatsData.zDKCalcUseCustomCriticalType)
    {
        zDKCalcBattleResultData.zDKCalcTypeMultiplier = zDKCalcEnemyStatsData.zDKCalcCriticalTypeMultiplier;
        zDKCalcBattleResultData.zDKCalcGuardMultiplier = zDKCalcEnemyStatsData.zDKCalcCriticalGuardMultiplier;
        zDKCalcBattleResultData.zDKCalcTypeDefenseBoostActive = false;
    }
    auto zDKCalcComputeDamage = [&](double zDKCalcAttack, double zDKCalcSpecific, int zDKCalcSpecificSlot)
    {
        zDKCalcDamageRange zDKCalcDamageRangeData;
        auto zDKCalcAdjustedAttack = zDKCalcApplyMultiplier(
            zDKCalcFloorStatistic(zDKCalcAttack),
            zDKCalcPercentMultiplier(zDKCalcEnemyStatsData.zDKCalcAttackModifierPercent)
        );
        if (zDKCalcEnemyStatsData.zDKCalcIsCritical)
            zDKCalcAdjustedAttack = zDKCalcApplyMultiplier(
                zDKCalcAdjustedAttack, zDKCalcEnemyStatsData.zDKCalcCriticalMultiplier
            );
        zDKCalcAdjustedAttack = zDKCalcApplyReduction(
            zDKCalcAdjustedAttack, 0, zDKCalcUnitStatsData.zDKCalcDamageReductionPercent
        );
        zDKCalcAdjustedAttack =
            zDKCalcApplyReduction(zDKCalcAdjustedAttack, zDKCalcSpecificSlot, zDKCalcSpecific);
        zDKCalcAdjustedAttack =
            zDKCalcApplyMultiplier(zDKCalcAdjustedAttack, zDKCalcBattleResultData.zDKCalcTypeMultiplier);
        zDKCalcDamageRangeData.zDKCalcAdjustedAttack = static_cast<double>(zDKCalcAdjustedAttack);
        auto zDKCalcDamageAtVariance = [&](double zDKCalcVariance)
        {
            auto zDKCalcDamage = zDKCalcApplyMultiplier(zDKCalcAdjustedAttack, zDKCalcVariance);
            zDKCalcDamage =
                zDKCalcDamage > zDKCalcEffectiveDefense ? zDKCalcDamage - zDKCalcEffectiveDefense : 0;
            zDKCalcDamage =
                zDKCalcApplyMultiplier(zDKCalcDamage, zDKCalcBattleResultData.zDKCalcGuardMultiplier);
            if (zDKCalcUnitStatsData.zDKCalcDamageReceivedFactors.empty())
                zDKCalcDamage = zDKCalcApplyMultiplier(
                    zDKCalcDamage, zDKCalcPercentMultiplier(zDKCalcUnitStatsData.zDKCalcDamageReceivedPercent)
                );
            else
                for (double zDKCalcFactor : zDKCalcUnitStatsData.zDKCalcDamageReceivedFactors)
                    zDKCalcDamage = zDKCalcApplyMultiplier(zDKCalcDamage, zDKCalcFactor);
            return static_cast<double>(zDKCalcDamage);
        };
        zDKCalcDamageRangeData.zDKCalcMinimumDamage = zDKCalcDamageAtVariance(1);
        zDKCalcDamageRangeData.zDKCalcAverageDamage = zDKCalcDamageAtVariance(1.015);
        zDKCalcDamageRangeData.zDKCalcMaximumDamage = zDKCalcDamageAtVariance(1.03);
        zDKCalcDamageRangeData.zDKCalcRequiredDefense = zDKCalcBattleResultData.zDKCalcGuardMultiplier == 0
            ? 0
            : static_cast<double>(zDKCalcApplyMultiplier(zDKCalcAdjustedAttack, 1.03));
        return zDKCalcDamageRangeData;
    };
    zDKCalcBattleResultData.zDKCalcNormalAttackDamage = zDKCalcComputeDamage(
        zDKCalcNormalAttackStat(zDKCalcEnemyStatsData),
        zDKCalcUnitStatsData.zDKCalcNormalDamageReductionPercent,
        1
    );
    zDKCalcBattleResultData.zDKCalcSuperAttackDamage = zDKCalcComputeDamage(
        zDKCalcSuperAttackStat(zDKCalcEnemyStatsData),
        zDKCalcUnitStatsData.zDKCalcSuperDamageReductionPercent,
        2
    );
    return zDKCalcBattleResultData;
}
} // namespace zDK
