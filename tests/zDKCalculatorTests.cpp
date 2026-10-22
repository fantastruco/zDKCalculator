#include "zDKEffects.h"
#include <iostream>
#include <limits>
#include <cstdlib>
using namespace zDK;
int zDKTestChecks = 0;

void zDKTestExpect(bool zDKTestCondition, const char* zDKTestLabel)
{
    ++zDKTestChecks;
    if (!zDKTestCondition)
    {
        std::cerr << "FAIL: " << zDKTestLabel << '\n';
        std::exit(1);
    }
}

void zDKTestExpectNear(double zDKTestFirstValue, double zDKTestSecondValue, const char* zDKTestLabel)
{
    zDKTestExpect(std::abs(zDKTestFirstValue - zDKTestSecondValue) < .001, zDKTestLabel);
}

zDKEffectBossProfile zDKTestExampleBoss()
{
    zDKEffectBossProfile zDKTestBossProfileData;
    zDKTestBossProfileData.zDKEffectStats = zDKCalcEnemyStats{};
    zDKTestBossProfileData.zDKEffectHealthPercent = 100;
    return zDKTestBossProfileData;
}

#include "zDKResearchTests.h"
#include "zDKNumberFormatTests.h"
#include "zDKRoundingTests.h"
#include "zDKSupportTests.h"
#include "zDKUpdateTests.h"

int zDKTestRunCalculatorChecks()
{
    zDKCalcUnitStats zDKTestUnitStatsData;
    zDKCalcEnemyStats zDKTestEnemyStatsData;
    zDKTestUnitStatsData.zDKCalcRoundDownDefense = false;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData).zDKCalcDefense.zDKCalcSelected,
        1093158,
        "Nine additive stacks use integer phases even with the legacy flag disabled"
    );
    zDKTestUnitStatsData.zDKCalcUseManualDefense = true;
    zDKTestUnitStatsData.zDKCalcManualDefense = 400000;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData)
            .zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        600000,
        "Neutral type, same class"
    );
    zDKTestUnitStatsData.zDKCalcGuardsAllAttacks = true;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData)
            .zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        200000,
        "Guard applies after subtracting DEF"
    );
    zDKTestUnitStatsData.zDKCalcGuardsAllAttacks = false;
    zDKTestUnitStatsData.zDKCalcDamageReductionPercent = 50;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData)
            .zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        100000,
        "DR applies before DEF"
    );
    zDKTestUnitStatsData.zDKCalcDamageReductionPercent = 30;
    zDKTestUnitStatsData.zDKCalcGuardsAllAttacks = true;
    zDKTestUnitStatsData.zDKCalcTypeDefenseBoostLevel = 10;
    zDKTestEnemyStatsData.zDKCalcElementType = zDKCalcUnitType::zDKCalcSTR;
    auto zDKTestBattleResultData = zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData);
    zDKTestExpectNear(
        zDKTestBattleResultData.zDKCalcNormalAttackDamage.zDKCalcAverageDamage,
        48675,
        "Complete guide example"
    );
    zDKTestExpectNear(
        zDKTestBattleResultData.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage, 45000, "Minimum variance"
    );
    zDKTestExpectNear(
        zDKTestBattleResultData.zDKCalcNormalAttackDamage.zDKCalcMaximumDamage, 52350, "Maximum variance"
    );
    zDKTestEnemyStatsData.zDKCalcElementType = zDKCalcUnitType::zDKCalcINT;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData).zDKCalcTypeMultiplier,
        .8,
        "Type DEF Boost inactive at neutral type with guard"
    );
    zDKTestEnemyStatsData.zDKCalcElementType = zDKCalcUnitType::zDKCalcTEQ;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData).zDKCalcTypeMultiplier,
        .8,
        "Type DEF Boost inactive at disadvantage with guard"
    );
    zDKTestUnitStatsData.zDKCalcGuardsAllAttacks = false;
    zDKTestUnitStatsData.zDKCalcTypeDefenseBoostLevel = 0;
    const zDKCalcUnitType zDKTestTypeAdvantages[] = {
        zDKCalcUnitType::zDKCalcSTR,
        zDKCalcUnitType::zDKCalcAGL,
        zDKCalcUnitType::zDKCalcTEQ,
        zDKCalcUnitType::zDKCalcPHY,
        zDKCalcUnitType::zDKCalcINT
    };
    for (int zDKTestIndex = 0; zDKTestIndex < 5; ++zDKTestIndex)
    {
        zDKTestUnitStatsData.zDKCalcElementType = static_cast<zDKCalcUnitType>(zDKTestIndex);
        zDKTestEnemyStatsData.zDKCalcElementType = zDKTestTypeAdvantages[zDKTestIndex];
        zDKTestEnemyStatsData.zDKCalcCombatClass = zDKTestUnitStatsData.zDKCalcCombatClass;
        zDKTestBattleResultData = zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData);
        zDKTestExpect(
            zDKTestBattleResultData.zDKCalcMatchupRelation == zDKCalcTypeRelation::zDKCalcAdvantage,
            "Type wheel advantage"
        );
        zDKTestExpectNear(zDKTestBattleResultData.zDKCalcTypeMultiplier, .9, "Advantage with the same class");
        zDKTestExpectNear(zDKTestBattleResultData.zDKCalcGuardMultiplier, .5, "Natural guard");
        zDKTestEnemyStatsData.zDKCalcCombatClass = zDKCalcUnitClass::zDKCalcExtreme;
        zDKTestExpectNear(
            zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData).zDKCalcTypeMultiplier,
            1,
            "Advantage with the opposite class"
        );
        zDKTestEnemyStatsData.zDKCalcCombatClass = zDKTestUnitStatsData.zDKCalcCombatClass;
        std::swap(zDKTestUnitStatsData.zDKCalcElementType, zDKTestEnemyStatsData.zDKCalcElementType);
        zDKTestExpectNear(
            zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData).zDKCalcTypeMultiplier,
            1.25,
            "Disadvantage with the same class"
        );
        zDKTestEnemyStatsData.zDKCalcCombatClass = zDKCalcUnitClass::zDKCalcExtreme;
        zDKTestExpectNear(
            zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData).zDKCalcTypeMultiplier,
            1.5,
            "Disadvantage with the opposite class"
        );
    }
    zDKTestUnitStatsData = zDKCalcUnitStats{};
    zDKTestEnemyStatsData = zDKCalcEnemyStats{};
    zDKTestUnitStatsData.zDKCalcUseManualDefense = true;
    zDKTestUnitStatsData.zDKCalcManualDefense = 400000;
    zDKTestUnitStatsData.zDKCalcDamageReductionPercent = 30;
    zDKTestUnitStatsData.zDKCalcNormalDamageReductionPercent = 20;
    zDKTestBattleResultData = zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData);
    zDKTestExpectNear(
        zDKTestBattleResultData.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        160000,
        "Normal-specific DR uses a separate factor"
    );
    zDKTestExpectNear(
        zDKTestBattleResultData.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        1700000,
        "Normal DR does not affect Super Attacks"
    );
    zDKTestUnitStatsData.zDKCalcSuperDamageReductionPercent = 50;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData)
            .zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        650000,
        "Super Attack-specific DR"
    );
    zDKTestUnitStatsData.zDKCalcDamageReductionPercent = 100;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData)
            .zDKCalcSuperAttackDamage.zDKCalcMaximumDamage,
        0,
        "Full DR and zero damage floor"
    );
    zDKTestUnitStatsData = zDKCalcUnitStats{};
    zDKTestUnitStatsData.zDKCalcSuperAttackStackCount = 0;
    zDKTestUnitStatsData.zDKCalcOnAttackPercent = 100;
    zDKTestUnitStatsData.zDKCalcCurrentSuperAttackCount = 2;
    zDKTestUnitStatsData.zDKCalcSuperAttackDefensePercent = 50;
    zDKTestUnitStatsData.zDKCalcRoundDownDefense = false;
    zDKTestBattleResultData = zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData);
    zDKTestExpectNear(
        zDKTestBattleResultData.zDKCalcDefense.zDKCalcAfterAttack,
        zDKTestBattleResultData.zDKCalcDefense.zDKCalcBeforeAttack * 4,
        "Phase 2 and new SA raises after attacking"
    );
    zDKTestUnitStatsData.zDKCalcAfterUnitAttack = false;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData).zDKCalcDefense.zDKCalcSelected,
        zDKTestBattleResultData.zDKCalcDefense.zDKCalcBeforeAttack,
        "Impact timing"
    );
    zDKTestUnitStatsData = zDKCalcUnitStats{};
    zDKTestUnitStatsData.zDKCalcUseManualDefense = true;
    zDKTestEnemyStatsData = zDKCalcEnemyStats{};
    zDKTestEnemyStatsData.zDKCalcIsCritical = true;
    zDKTestEnemyStatsData.zDKCalcCriticalDefenseIgnoredPercent = 50;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData)
            .zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        1300000,
        "Configurable critical hit and DEF ignored"
    );
    zDKTestEnemyStatsData.zDKCalcUseCustomCriticalType = true;
    zDKTestEnemyStatsData.zDKCalcCriticalTypeMultiplier = .8;
    zDKTestEnemyStatsData.zDKCalcCriticalGuardMultiplier = .5;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData)
            .zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        500000,
        "Manual critical type and guard"
    );
    zDKTestUnitStatsData.zDKCalcDefenseDebuffPercent = 50;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData).zDKCalcDefense.zDKCalcEffective,
        100000,
        "DEF Down and DEF ignored"
    );
    zDKTestUnitStatsData = zDKCalcUnitStats{};
    zDKTestEnemyStatsData = zDKCalcEnemyStats{};
    zDKTestUnitStatsData.zDKCalcRoundDownDefense = true;
    zDKTestBattleResultData = zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData);
    zDKTestExpect(
        zDKTestBattleResultData.zDKCalcDefense.zDKCalcSelected ==
            std::floor(zDKTestBattleResultData.zDKCalcDefense.zDKCalcSelected),
        "Truncation after each phase"
    );
    zDKTestUnitStatsData.zDKCalcBaseDefense = std::numeric_limits<double>::quiet_NaN();
    bool zDKTestRejected = false;
    try
    {
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData);
    }
    catch (const std::invalid_argument&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Rejects NaN");
    zDKTestUnitStatsData = zDKCalcUnitStats{};
    zDKTestUnitStatsData.zDKCalcDamageReductionPercent = 101;
    zDKTestRejected = false;
    try
    {
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData);
    }
    catch (const std::invalid_argument&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Rejects invalid DR");
    zDKTestUnitStatsData = zDKCalcUnitStats{};
    zDKTestUnitStatsData.zDKCalcSuperAttackStackCount = 1.5;
    zDKTestRejected = false;
    try
    {
        zDKCalcCalculateBattle(zDKTestUnitStatsData, zDKTestEnemyStatsData);
    }
    catch (const std::invalid_argument&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Rejects fractional stacks");
    zDKEffectUnitProfile zDKTestUnitProfileData;
    zDKEffectBossProfile zDKTestBossProfileData = zDKTestExampleBoss();
    zDKEffectBattleState zDKTestState;
    zDKTestState.zDKEffectCurrentHP = zDKTestState.zDKEffectMaxHP = 700000;
    zDKTestUnitProfileData.zDKEffectBaseDefense = 1000;
    zDKTestUnitProfileData.zDKEffectLeaderSkillPercent = 0;
    zDKTestUnitProfileData.zDKEffectRoundDownDefense = false;
    auto zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
    zDKTestEffect.zDKEffectValue = 200;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestEffect.zDKEffectValue = 100;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcDefense.zDKCalcSelected,
        4000,
        "Effects add within a group"
    );
    zDKTestUnitProfileData.zDKEffectEffects[1].zDKEffectMultiplicative = true;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcDefense.zDKCalcSelected,
        6000,
        "Independent multiplicative effect"
    );
    zDKTestUnitProfileData.zDKEffectEffects[1].zDKEffectEnabled = false;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcDefense.zDKCalcSelected,
        3000,
        "Disabled effects are excluded"
    );
    zDKTestUnitProfileData.zDKEffectEffects.clear();
    zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectSuperAttackStack);
    zDKTestEffect.zDKEffectCount = 9;
    zDKTestEffect.zDKEffectValue = 50;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcDefense.zDKCalcSelected,
        5500,
        "SA stacks are additive by default"
    );
    zDKTestUnitProfileData.zDKEffectEffects[0].zDKEffectTiming = zDKEffectAttackTiming::zDKEffectAfterAttack;
    zDKTestState.zDKEffectAfterUnitAttack = false;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcDefense.zDKCalcSelected,
        1000,
        "Impact before conditional stack"
    );
    zDKTestState.zDKEffectAfterUnitAttack = true;
    auto zDKTestEvaluated = zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState);
    zDKTestExpectNear(
        zDKTestEvaluated.zDKEffectResult.zDKCalcDefense.zDKCalcBeforeAttack, 1000, "Before attack snapshot"
    );
    zDKTestExpectNear(
        zDKTestEvaluated.zDKEffectResult.zDKCalcDefense.zDKCalcAfterAttack, 5500, "After attack snapshot"
    );
    zDKTestUnitProfileData.zDKEffectEffects.clear();
    zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectEntrance);
    zDKTestEffect.zDKEffectValue = 100;
    zDKTestEffect.zDKEffectFirstTurn = 2;
    zDKTestEffect.zDKEffectLastTurn = 4;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestState.zDKEffectTurn = 1;
    zDKTestExpect(
        !zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
             .zDKEffectUnitEffectActivation[0],
        "Entrance not started"
    );
    zDKTestState.zDKEffectTurn = 2;
    zDKTestExpect(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectUnitEffectActivation[0],
        "Entrance start inclusive"
    );
    zDKTestState.zDKEffectTurn = 4;
    zDKTestExpect(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectUnitEffectActivation[0],
        "Entrance end inclusive"
    );
    zDKTestState.zDKEffectTurn = 5;
    zDKTestExpect(
        !zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
             .zDKEffectUnitEffectActivation[0],
        "Entrance expires"
    );
    zDKTestUnitProfileData.zDKEffectEffects.clear();
    auto zDKTestBossEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic, true);
    zDKTestBossEffect.zDKEffectValue = 100;
    zDKTestBossEffect.zDKEffectFirstTurn = 3;
    zDKTestBossEffect.zDKEffectHealthRule = zDKEffectHealthRule::zDKEffectBelow;
    zDKTestBossEffect.zDKEffectHealthThresholdPercent = 50;
    zDKTestBossProfileData.zDKEffectEffects.push_back(zDKTestBossEffect);
    zDKTestBossProfileData.zDKEffectHealthPercent = 50;
    zDKTestState.zDKEffectTurn = 2;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResolvedBoss.zDKCalcNormalAttack,
        1000000,
        "Boss turn condition excludes early turns"
    );
    zDKTestState.zDKEffectTurn = 3;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResolvedBoss.zDKCalcNormalAttack,
        2000000,
        "Boss turn and HP condition boundary"
    );
    zDKTestBossProfileData.zDKEffectHealthPercent = 50.1;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResolvedBoss.zDKCalcNormalAttack,
        1000000,
        "Boss HP condition inactive"
    );
    zDKTestBossProfileData.zDKEffectEffects[0].zDKEffectHealthRule = zDKEffectHealthRule::zDKEffectAbove;
    zDKTestBossProfileData.zDKEffectHealthPercent = 50;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResolvedBoss.zDKCalcNormalAttack,
        2000000,
        "Boss HP above boundary inclusive"
    );
    zDKTestBossProfileData.zDKEffectHealthPercent = 49.9;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResolvedBoss.zDKCalcNormalAttack,
        1000000,
        "Boss above HP excludes lower HP"
    );
    zDKTestBossProfileData = zDKTestExampleBoss();
    zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectGuard);
    zDKTestEffect.zDKEffectHealthRule = zDKEffectHealthRule::zDKEffectBelow;
    zDKTestEffect.zDKEffectHealthThresholdPercent = 50;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestState.zDKEffectCurrentHP = 350000;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcGuardMultiplier,
        .5,
        "Conditional unit guard at HP boundary"
    );
    zDKTestState.zDKEffectCurrentHP = 350001;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcGuardMultiplier,
        1,
        "Conditional guard inactive"
    );
    zDKTestUnitProfileData.zDKEffectEffects.clear();
    zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
    zDKTestEffect.zDKEffectStat = zDKEffectStat::zDKEffectDamageReduction;
    zDKTestEffect.zDKEffectValue = 30;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestEffect.zDKEffectValue = 20;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResolvedUnit.zDKCalcDamageReductionPercent,
        50,
        "Additive general DR"
    );
    zDKTestUnitProfileData.zDKEffectEffects[1].zDKEffectMultiplicative = true;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResolvedUnit.zDKCalcDamageReductionPercent,
        44,
        "Independent DR factors"
    );
    zDKTestUnitProfileData.zDKEffectEffects[1].zDKEffectMultiplicative = false;
    zDKTestUnitProfileData.zDKEffectEffects[0].zDKEffectValue = 80;
    zDKTestUnitProfileData.zDKEffectEffects[1].zDKEffectValue = 60;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResolvedUnit.zDKCalcDamageReductionPercent,
        100,
        "Summed DR caps at 100 percent"
    );
    zDKTestUnitProfileData.zDKEffectEffects.clear();
    zDKTestBossProfileData.zDKEffectEffects.clear();
    zDKTestBossEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic, true);
    zDKTestBossEffect.zDKEffectStat = zDKEffectStat::zDKEffectNormalAttack;
    zDKTestBossEffect.zDKEffectValue = 100;
    zDKTestBossProfileData.zDKEffectEffects.push_back(zDKTestBossEffect);
    zDKTestEvaluated = zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState);
    zDKTestExpectNear(
        zDKTestEvaluated.zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        1999000,
        "Normal ATK effect"
    );
    zDKTestExpectNear(
        zDKTestEvaluated.zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        2999000,
        "Normal effect does not modify SA"
    );
    zDKTestBossProfileData.zDKEffectEffects[0].zDKEffectStat = zDKEffectStat::zDKEffectSuperAttack;
    zDKTestEvaluated = zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState);
    zDKTestExpectNear(
        zDKTestEvaluated.zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        999000,
        "SA effect does not modify normal"
    );
    zDKTestExpectNear(
        zDKTestEvaluated.zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        5999000,
        "SA ATK effect"
    );
    zDKTestBossProfileData.zDKEffectEffects[0].zDKEffectStat = zDKEffectStat::zDKEffectBossDefense;
    zDKTestBossProfileData.zDKEffectBaseDefense = 10000;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState).zDKEffectBossDefense,
        20000,
        "Boss DEF effects tracked separately"
    );
    zDKTestBossProfileData.zDKEffectEffects[0].zDKEffectStat = zDKEffectStat::zDKEffectAttack;
    zDKTestBossProfileData.zDKEffectStats.zDKCalcAttackModifierPercent = 50;
    zDKTestEvaluated = zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState);
    zDKTestExpectNear(
        zDKTestEvaluated.zDKEffectResolvedBoss.zDKCalcNormalAttack,
        3000000,
        "Boss ATK resolution includes global modifier"
    );
    zDKTestExpectNear(
        zDKTestEvaluated.zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        2999000,
        "Global boss modifier is applied exactly once"
    );
    zDKTestBossProfileData.zDKEffectStats.zDKCalcAttackModifierPercent = 0;
    zDKTestBossProfileData.zDKEffectEffects.clear();
    zDKTestBossEffect.zDKEffectStat = zDKEffectStat::zDKEffectPenetration;
    zDKTestBossEffect.zDKEffectValue = 50;
    zDKTestBossProfileData.zDKEffectEffects.push_back(zDKTestBossEffect);
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcDefense.zDKCalcEffective,
        500,
        "Noncritical DEF penetration"
    );
    zDKTestBossProfileData = zDKTestExampleBoss();
    zDKTestUnitProfileData = zDKEffectUnitProfile{};
    zDKTestUnitProfileData.zDKEffectUseManualDefense = true;
    zDKTestUnitProfileData.zDKEffectManualDefense = 400000;
    zDKTestUnitProfileData.zDKEffectTypeDefenseBoostLevel = 10;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKEffectCreate(zDKEffectCategory::zDKEffectGuard));
    zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
    zDKTestEffect.zDKEffectStat = zDKEffectStat::zDKEffectDamageReduction;
    zDKTestEffect.zDKEffectValue = 30;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestBossProfileData.zDKEffectStats.zDKCalcElementType = zDKCalcUnitType::zDKCalcSTR;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcAverageDamage,
        48675,
        "Effect system reproduces complete guide example"
    );
    zDKTestUnitProfileData.zDKEffectName = "Test unit \"A\"";
    zDKTestUnitProfileData.zDKEffectEffects[0].zDKEffectName = "Conditional guard";
    zDKTestUnitProfileData.zDKEffectEffects[0].zDKEffectLastTurn = 7;
    std::istringstream zDKTestUnitFile(zDKStoreSaveUnit(zDKTestUnitProfileData));
    zDKEffectUnitProfile zDKTestLoaded;
    double zDKTestBossAttackBefore = zDKTestBossProfileData.zDKEffectStats.zDKCalcNormalAttack;
    zDKTestExpect(zDKStoreLoadUnit(zDKTestUnitFile, zDKTestLoaded), "Unit preset parses");
    zDKTestExpect(
        zDKStoreSaveUnit(zDKTestLoaded) == zDKStoreSaveUnit(zDKTestUnitProfileData),
        "Unit preset preserves every field"
    );
    zDKTestExpectNear(
        zDKTestBossProfileData.zDKEffectStats.zDKCalcNormalAttack,
        zDKTestBossAttackBefore,
        "Unit preset leaves boss unchanged"
    );
    auto zDKTestSession =
        zDKStoreSaveSession(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState, false, 1);
    std::istringstream zDKTestSessionFile(zDKTestSession);
    zDKEffectUnitProfile zDKTestLoadedUnit;
    zDKEffectBossProfile zDKTestLoadedBoss;
    zDKEffectBattleState zDKTestLoadedState;
    bool zDKTestLoadedAnimation = true;
    int zDKTestLoadedAttack = 0;
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestSessionFile,
            zDKTestLoadedUnit,
            zDKTestLoadedBoss,
            zDKTestLoadedState,
            zDKTestLoadedAnimation,
            zDKTestLoadedAttack
        ),
        "Session parses"
    );
    zDKTestExpect(
        zDKStoreSaveSession(
            zDKTestLoadedUnit,
            zDKTestLoadedBoss,
            zDKTestLoadedState,
            zDKTestLoadedAnimation,
            zDKTestLoadedAttack
        ) == zDKTestSession,
        "Session round trip"
    );
    std::istringstream zDKTestBrokenUnit("DOKKAN_UNIT 2\n\"broken\"\n100 100");
    std::string zDKTestOriginal = zDKStoreSaveUnit(zDKTestLoaded);
    zDKTestExpect(!zDKStoreLoadUnit(zDKTestBrokenUnit, zDKTestLoaded), "Truncated preset rejected");
    zDKTestExpect(zDKStoreSaveUnit(zDKTestLoaded) == zDKTestOriginal, "Failed preset load is atomic");
    zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
    zDKTestEffect.zDKEffectFirstTurn = 5;
    zDKTestEffect.zDKEffectLastTurn = 2;
    zDKTestUnitProfileData.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestRejected = false;
    try
    {
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState);
    }
    catch (const std::invalid_argument&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Invalid condition interval rejected");
    zDKTestUnitProfileData = zDKEffectCreateEmptyUnitProfile();
    zDKTestBossProfileData = zDKEffectBossProfile{};
    zDKTestState = zDKEffectBattleState{};
    zDKTestExpect(
        zDKTestUnitProfileData.zDKEffectBaseDefense == 0 &&
            zDKTestUnitProfileData.zDKEffectLeaderSkillPercent == 0 &&
            zDKTestUnitProfileData.zDKEffectManualDefense == 0 &&
            zDKTestUnitProfileData.zDKEffectTypeDefenseBoostLevel == 0 &&
            zDKTestUnitProfileData.zDKEffectEffects.empty(),
        "Unit starts with zero stats and no sample effects"
    );
    zDKTestExpect(
        zDKTestBossProfileData.zDKEffectStats.zDKCalcNormalAttack == 0 &&
            zDKTestBossProfileData.zDKEffectStats.zDKCalcSuperAttack == 0 &&
            zDKTestBossProfileData.zDKEffectBaseDefense == 0 &&
            zDKTestBossProfileData.zDKEffectHealthPercent == 0,
        "Enemy starts with zero stats"
    );
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMaximumDamage,
        0,
        "Empty HP and zero stats evaluate without division by zero"
    );
    zDKTestExpect(
        zDKEffectCreate(zDKEffectCategory::zDKEffectSuperAttackStack).zDKEffectValue == 0 &&
            zDKEffectCreate(zDKEffectCategory::zDKEffectBasic).zDKEffectValue == 0,
        "New effect amounts start at zero"
    );
    zDKTestUnitProfileData.zDKEffectUseManualDefense = true;
    zDKTestUnitProfileData.zDKEffectElementType = zDKCalcUnitType::zDKCalcINT;
    zDKTestBossProfileData.zDKEffectStats.zDKCalcElementType = zDKCalcUnitType::zDKCalcINT;
    zDKTestBossProfileData.zDKEffectStats.zDKCalcNormalAttack = 1000000;
    zDKTestBossProfileData.zDKEffectStats.zDKCalcSuperAttack = 2700000;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        1000000,
        "Direct Normal ATK input"
    );
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        2700000,
        "Direct Super Attack ATK input"
    );
    zDKTestBossProfileData.zDKEffectStats.zDKCalcNormalAttack = 0;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        2700000,
        "SA ATK is independent of zero Normal ATK"
    );
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMaximumDamage,
        0,
        "Zero Normal ATK stays zero"
    );
    zDKTestBossEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic, true);
    zDKTestBossEffect.zDKEffectValue = 100;
    zDKTestBossProfileData.zDKEffectEffects = {zDKTestBossEffect};
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        5400000,
        "Shared enemy ATK boost applies to direct SA ATK"
    );
    zDKTestBossProfileData.zDKEffectStats.zDKCalcNormalAttack = 1000000;
    zDKTestBossProfileData.zDKEffectEffects[0].zDKEffectStat = zDKEffectStat::zDKEffectNormalAttack;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        2000000,
        "Normal-only effect with direct inputs"
    );
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        2700000,
        "Normal-only effect preserves direct SA ATK"
    );
    zDKTestBossProfileData.zDKEffectEffects[0].zDKEffectStat = zDKEffectStat::zDKEffectSuperAttack;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        1000000,
        "SA-only effect preserves direct Normal ATK"
    );
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        5400000,
        "SA-only effect with direct inputs"
    );
    zDKTestBossProfileData.zDKEffectEffects.clear();
    zDKTestBossProfileData.zDKEffectStats.zDKCalcAttackModifierPercent = -20;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState)
            .zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        2160000,
        "Enemy modifier is applied once to direct SA ATK"
    );
    zDKTestSession =
        zDKStoreSaveSession(zDKTestUnitProfileData, zDKTestBossProfileData, zDKTestState, true, 0);
    std::istringstream zDKTestDirectFile(zDKTestSession);
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestDirectFile,
            zDKTestLoadedUnit,
            zDKTestLoadedBoss,
            zDKTestLoadedState,
            zDKTestLoadedAnimation,
            zDKTestLoadedAttack
        ),
        "Direct attack stats session parses"
    );
    zDKTestExpect(
        zDKStoreSaveSession(
            zDKTestLoadedUnit,
            zDKTestLoadedBoss,
            zDKTestLoadedState,
            zDKTestLoadedAnimation,
            zDKTestLoadedAttack
        ) == zDKTestSession,
        "Direct attack stats and zero HP survive a session round trip"
    );
    std::ostringstream zDKTestLegacy;
    zDKTestLegacy << "DOKKAN_SESSION 2\n"
                  << zDKStoreSaveUnit(zDKTestUnitProfileData)
                  << "\"Old boss\"\n1000000 3 0 2 0 0 0 1.5 0 1 1 0 0 100\n0\n1 700000 700000 1 1 0\n";
    std::istringstream zDKTestLegacyFile(zDKTestLegacy.str());
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestLegacyFile,
            zDKTestLoadedUnit,
            zDKTestLoadedBoss,
            zDKTestLoadedState,
            zDKTestLoadedAnimation,
            zDKTestLoadedAttack
        ),
        "Legacy v2 session migrates"
    );
    zDKTestExpectNear(
        zDKTestLoadedBoss.zDKEffectStats.zDKCalcSuperAttack,
        3000000,
        "Legacy SA multiplier is converted to a direct attack stat"
    );
    zDKTestResearchCases();
    zDKTestNumberFormatting();
    zDKTestIntegerRounding();
    zDKTestSupportAndSections();
    zDKTestUpdates();
    std::cout << zDKTestChecks << " checks passed.\n";
    return 0;
}

// The C++ runtime requires this exact test executable entry point.
int main()
{
    return zDKTestRunCalculatorChecks();
}
