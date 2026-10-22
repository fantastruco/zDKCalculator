#pragma once
#include "zDKTurnTimeline.h"

void zDKTestTurnTimeline()
{
    zDKEffectUnitProfile zDKTestUnit;
    zDKTestUnit.zDKEffectBaseDefense = 1000;
    zDKEffectBossProfile zDKTestBoss;
    zDKTestBoss.zDKEffectStats.zDKCalcNormalAttack = 10000;
    zDKTestBoss.zDKEffectStats.zDKCalcSuperAttack = 20000;
    zDKEffectBattleState zDKTestBattle;
    auto zDKTestSuper = zDKEffectCreate(zDKEffectCategory::zDKEffectSuperAttackStack);
    zDKTestSuper.zDKEffectValue = 30;
    auto zDKTestUltra = zDKEffectCreate(zDKEffectCategory::zDKEffectUltraSuperAttack);
    zDKTestUltra.zDKEffectValue = 50;
    zDKTestUnit.zDKEffectEffects = {zDKTestSuper, zDKTestUltra};
    zDKTestUnit.zDKEffectAttackPlan.zDKTurnAttacks = {
        zDKTurnAttackKind::zDKTurnUltra,
        zDKTurnAttackKind::zDKTurnNormal,
        zDKTurnAttackKind::zDKTurnSuper,
        zDKTurnAttackKind::zDKTurnSuper
    };
    const auto zDKTestMixed = zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    const double zDKTestExpected[] = {1000, 1500, 1500, 1800, 2100};
    zDKTestExpect(zDKTestMixed.size() == 5, "Turn timeline includes the starting point and each attack");
    zDKTestExpectNear(
        zDKTurnEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectNormalDefense.zDKCalcSelected,
        2100,
        "After-attack damage analysis uses the final planned attack"
    );
    auto zDKTestBefore = zDKTestBattle;
    zDKTestBefore.zDKEffectAfterUnitAttack = false;
    const auto zDKTestBeforeResult = zDKTurnEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBefore);
    zDKTestExpectNear(
        zDKTestBeforeResult.zDKEffectNormalDefense.zDKCalcSelected,
        1000,
        "Before-attack analysis excludes raises from future attacks"
    );
    zDKTestExpectNear(
        zDKTestBeforeResult.zDKEffectResult.zDKCalcDefense.zDKCalcAfterAttack,
        2100,
        "The DEF breakdown still previews the real end-of-turn result before attacking"
    );
    for (int zDKTestIndex = 0; zDKTestIndex < 5; ++zDKTestIndex)
        zDKTestExpectNear(
            zDKTestMixed[zDKTestIndex].zDKTurnNormalDefense,
            zDKTestExpected[zDKTestIndex],
            "Ultra and additional SA raises add in their shared bracket; normals add no SA raise"
        );
    zDKTestExpect(
        zDKTestMixed.back().zDKTurnSupersPerformed == 2 && zDKTestMixed.back().zDKTurnUltrasPerformed == 1 &&
            zDKTestMixed.back().zDKTurnNormalsPerformed == 1,
        "Mixed turn counts distinguish normals, SA and Ultra SA"
    );
    zDKTestExpect(
        zDKTestUnit.zDKEffectEffects[0].zDKEffectCount == 0 && zDKTestBattle.zDKEffectAfterUnitAttack,
        "Turn preview preserves the original profile and battle timing"
    );
    zDKTestUnit.zDKEffectEffects[0].zDKEffectCount = 2;
    zDKTestExpectNear(
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle)[0].zDKTurnNormalDefense,
        1600,
        "Previously active SA raises remain in the starting DEF"
    );
    zDKTestUnit.zDKEffectEffects[0].zDKEffectCount = 0;
    auto zDKTestSupport = zDKEffectCreate(zDKEffectCategory::zDKEffectSupport);
    zDKTestSupport.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectSuperAttack;
    zDKTestSupport.zDKEffectValue = 20;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestSupport);
    zDKTestExpectNear(
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle).back().zDKTurnNormalDefense,
        2300,
        "Ally SA support is retained without repeating on the unit's own attacks"
    );
    zDKTestUnit.zDKEffectEffects.pop_back();
    auto zDKTestBuildUp = zDKEffectCreate(zDKEffectCategory::zDKEffectWhenAttacking);
    zDKTestBuildUp.zDKEffectValue = 25;
    zDKTestBuildUp.zDKEffectBuildUpTrigger = zDKTurnBuildUpTrigger::zDKTurnAnyAttack;
    zDKTestBuildUp.zDKEffectBuildUpLimit = 2;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestBuildUp);
    const auto zDKTestBuild = zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    zDKTestExpectNear(
        zDKTestBuild[1].zDKTurnNormalDefense,
        1875,
        "First attack activates Phase 2 build-up before SA effects"
    );
    zDKTestExpectNear(
        zDKTestBuild[2].zDKTurnNormalDefense, 2250, "Normal additional builds the passive without an SA raise"
    );
    zDKTestExpectNear(
        zDKTestBuild[4].zDKTurnNormalDefense, 3150, "Build-up stops at its configured activation limit"
    );
    zDKTestUnit.zDKEffectEffects.back().zDKEffectBuildUpTrigger = zDKTurnBuildUpTrigger::zDKTurnSuperAttack;
    zDKTestExpectNear(
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle)[2].zDKTurnNormalDefense,
        1875,
        "Super-only passive build-up counts Ultra SA but excludes normal attacks"
    );
    zDKTestUnit.zDKEffectEffects.back().zDKEffectBuildUpCount = 1;
    zDKTestExpectNear(
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle)[0].zDKTurnNormalDefense,
        1250,
        "Retained per-attack build-up is already active before this turn's first attack"
    );
    zDKTestUnit.zDKEffectUseManualDefense = true;
    zDKTestUnit.zDKEffectManualDefense = 7777;
    zDKTestExpectNear(
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle).back().zDKTurnNormalDefense,
        7777,
        "Manual DEF remains fixed through the attack plan"
    );

    // Published Wrathful Broly example: three attacks, two SA, independent integer phases.
    zDKTestUnit = {};
    zDKTestUnit.zDKEffectBaseDefense = 13169;
    zDKTestUnit.zDKEffectLeaderSkillPercent = 440;
    for (const auto& zDKTestPhase : std::vector<std::pair<zDKEffectDefensePhase, double>>{
             {zDKEffectDefensePhase::zDKEffectStartOfTurn, 300},
             {zDKEffectDefensePhase::zDKEffectDomain, 15},
             {zDKEffectDefensePhase::zDKEffectItem, 20},
             {zDKEffectDefensePhase::zDKEffectLinks, 10}
         })
    {
        auto zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
        zDKTestEffect.zDKEffectGroup = zDKTestPhase.first;
        zDKTestEffect.zDKEffectValue = zDKTestPhase.second;
        zDKTestUnit.zDKEffectEffects.push_back(zDKTestEffect);
    }
    zDKTestSuper.zDKEffectValue = 20;
    zDKTestBuildUp.zDKEffectValue = 50;
    zDKTestBuildUp.zDKEffectBuildUpLimit = 8;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestSuper);
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestBuildUp);
    zDKTestUnit.zDKEffectAttackPlan.zDKTurnAttacks = {
        zDKTurnAttackKind::zDKTurnSuper, zDKTurnAttackKind::zDKTurnNormal, zDKTurnAttackKind::zDKTurnSuper
    };
    const auto zDKTestBroly = zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    zDKTestExpectNear(
        zDKTestBroly[0].zDKTurnNormalDefense, 431791, "Published Broly pre-attack DEF truncates every stage"
    );
    zDKTestExpectNear(
        zDKTestBroly[1].zDKTurnNormalDefense, 777223, "First SA recalculates both passive and SA brackets"
    );
    zDKTestExpectNear(
        zDKTestBroly[2].zDKTurnNormalDefense, 1036298, "Normal additional updates only the passive bracket"
    );
    zDKTestExpectNear(
        zDKTestBroly[3].zDKTurnNormalDefense,
        1511267,
        "Three attacks and two SA reproduce the published final DEF"
    );
    auto zDKTestReceiving = zDKEffectCreate(zDKEffectCategory::zDKEffectWhenReceiving);
    zDKTestReceiving.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectWhenReceivingSuper;
    zDKTestReceiving.zDKEffectValue = 100;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestReceiving);
    const auto zDKTestSeparate = zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    zDKTestExpect(
        zDKTestSeparate.back().zDKTurnSuperDefense > zDKTestSeparate.back().zDKTurnNormalDefense,
        "Each turn point preserves distinct normal and Super Attack receiving DEF"
    );
    zDKTestUnit.zDKEffectEffects[4].zDKEffectEnabled = false;
    zDKTestExpectNear(
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle).back().zDKTurnNormalDefense,
        1079477,
        "Disabled SA effects do not gain raises in the turn projection"
    );
    zDKTestUnit.zDKEffectAttackPlan.zDKTurnAttacks.assign(17, zDKTurnAttackKind::zDKTurnNormal);
    bool zDKTestRejected = false;
    try
    {
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    }
    catch (const std::exception&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Oversized attack plans are rejected");
    zDKTestUnit.zDKEffectAttackPlan.zDKTurnAttacks = {static_cast<zDKTurnAttackKind>(3)};
    zDKTestRejected = false;
    try
    {
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    }
    catch (const std::exception&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Invalid attack kinds are rejected");
    zDKTestUnit = {};
    zDKTestUnit.zDKEffectBaseDefense = 1000;
    auto zDKTestGuard = zDKEffectCreate(zDKEffectCategory::zDKEffectUltraSuperAttack);
    zDKTestGuard.zDKEffectStat = zDKEffectStat::zDKEffectGuard;
    zDKTestUnit.zDKEffectEffects = {zDKTestGuard};
    zDKTestUnit.zDKEffectAttackPlan.zDKTurnAttacks = {
        zDKTurnAttackKind::zDKTurnNormal, zDKTurnAttackKind::zDKTurnUltra
    };
    const auto zDKTestGuardPoints = zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    zDKTestExpect(
        zDKTestGuardPoints[0].zDKTurnNormalMaxDamage == zDKTestGuardPoints[1].zDKTurnNormalMaxDamage &&
            zDKTestGuardPoints[2].zDKTurnNormalMaxDamage < zDKTestGuardPoints[1].zDKTurnNormalMaxDamage,
        "Ultra SA guard activates at its matching attack rather than on a normal additional"
    );
    zDKTestSuper.zDKEffectHealthRule = zDKEffectHealthRule::zDKEffectAbove;
    zDKTestSuper.zDKEffectHealthThresholdPercent = 50;
    zDKTestUnit.zDKEffectEffects = {zDKTestSuper};
    zDKTestUnit.zDKEffectAttackPlan.zDKTurnAttacks = {zDKTurnAttackKind::zDKTurnSuper};
    zDKTestExpectNear(
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle).back().zDKTurnNormalDefense,
        1000,
        "Inactive HP-conditioned raises do not enter the attack plan"
    );
    zDKTestUnit.zDKEffectAttackPlan.zDKTurnAttacks.clear();
    zDKTestExpect(
        zDKTurnCalculateTimeline(zDKTestUnit, zDKTestBoss, zDKTestBattle).size() == 1,
        "An empty turn plan has only the starting DEF"
    );
}
