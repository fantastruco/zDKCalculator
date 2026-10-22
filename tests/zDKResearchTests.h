#pragma once

// Regression cases for docs/zDKDefenseResearch.md. Expected values are worked
// independently from the published examples, rather than copied from the engine.
void zDKTestResearchCases()
{
    using namespace zDK;
    zDKEffectUnitProfile zDKTestUnit;
    zDKEffectBossProfile zDKTestBoss;
    zDKEffectBattleState zDKTestBattle;
    zDKTestBattle.zDKEffectAfterUnitAttack = false;
    zDKTestBoss.zDKEffectStats.zDKCalcNormalAttack = 2000000;
    zDKTestBoss.zDKEffectStats.zDKCalcSuperAttack = 4000000;
    // Razzer's player Broly example checks the complete ordered, truncated DEF pipeline.
    zDKCalcUnitStats zDKTestBroly;
    zDKTestBroly.zDKCalcBaseDefense = 13169;
    zDKTestBroly.zDKCalcLeaderSkillPercent = 440;
    zDKTestBroly.zDKCalcStartOfTurnPercent = 300;
    zDKTestBroly.zDKCalcDomainPercent = 15;
    zDKTestBroly.zDKCalcItemPercent = 20;
    zDKTestBroly.zDKCalcLinkSkillPercent = 10;
    zDKTestBroly.zDKCalcOnAttackPercent = 150;
    zDKTestBroly.zDKCalcSuperAttackStackCount = 2;
    zDKTestBroly.zDKCalcStackDefensePercent = 20;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestBroly, zDKTestBoss.zDKEffectStats).zDKCalcDefense.zDKCalcSelected,
        1511267,
        "Razzer player Broly: ordered rounding yields 1,511,267 DEF"
    );
    zDKTestBroly.zDKCalcActiveSkillPercent = 60;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestBroly, zDKTestBoss.zDKEffectStats).zDKCalcDefense.zDKCalcSelected,
        2418026,
        "Razzer player Broly: active skill yields 2,418,026 DEF"
    );
    zDKTestBroly.zDKCalcOnAttackPercent = 300;
    zDKTestBroly.zDKCalcSuperAttackStackCount = 5;
    zDKTestExpectNear(
        zDKCalcCalculateBattle(zDKTestBroly, zDKTestBoss.zDKEffectStats).zDKCalcDefense.zDKCalcSelected,
        5526920,
        "Razzer player Broly: updated buildup and SA stacks yield 5,526,920 DEF"
    );
    // Helioseckta's published Gogeta example: 446,528 -> 1,339,584 -> 2,232,640.
    zDKTestUnit.zDKEffectBaseDefense = 446528;
    auto zDKTestReceive = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
    zDKTestReceive.zDKEffectValue = 200;
    zDKTestReceive.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectWhenReceiving;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestReceive);
    zDKTestReceive.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectWhenReceivingSuper;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestReceive);
    auto zDKTestEvaluation = zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectNormalDefense.zDKCalcSelected,
        1339584,
        "Gogeta receiving normals: published DEF"
    );
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectSuperDefense.zDKCalcSelected,
        2232640,
        "Gogeta receiving SA: Phase 2 adds 200% + 200%"
    );
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        660416,
        "Normal damage uses normal-only defense"
    );
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage,
        1767360,
        "SA damage uses SA-specific defense"
    );
    zDKTestExpect(
        zDKTestEvaluation.zDKEffectUnitEffectActivation[1], "Receiving-SA effect activates before own attack"
    );
    zDKTestEvaluation = zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle, false);
    zDKTestExpect(
        !zDKTestEvaluation.zDKEffectUnitEffectActivation[1], "Receiving-SA effect inactive against normals"
    );
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectResult.zDKCalcDefense.zDKCalcSelected,
        1339584,
        "Selected DEF follows incoming attack kind"
    );
    zDKTestBattle.zDKEffectAfterUnitAttack = true;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectSuperDefense.zDKCalcSelected,
        2232640,
        "Receiving DEF also applies after own attack"
    );
    zDKTestUnit.zDKEffectBaseDefense = 554393;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectSuperDefense.zDKCalcSelected,
        2771965,
        "Rainbow Gogeta: second published DEF scenario"
    );
    zDKTestReceive.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectWhenAttacking;
    zDKTestReceive.zDKEffectValue = 100;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestReceive);
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectSuperDefense.zDKCalcSelected,
        3326358,
        "Attacking and receiving boosts share Phase 2"
    );
    zDKTestBattle.zDKEffectAfterUnitAttack = false;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectSuperDefense.zDKCalcSelected,
        2771965,
        "Before own attack excludes only attacking boost"
    );
    zDKTestUnit.zDKEffectEffects.back().zDKEffectGroup = zDKEffectDefensePhase::zDKEffectWhenReceivingNormal;
    zDKTestEvaluation = zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectNormalDefense.zDKCalcSelected,
        2217572,
        "Normal-only receiving DEF adds to generic receiving DEF"
    );
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectSuperDefense.zDKCalcSelected,
        2771965,
        "Normal-only receiving DEF excluded from SA"
    );

    // Domain damage received is applied after subtracting DEF and applying guard.
    zDKTestUnit = zDKEffectUnitProfile{};
    zDKTestUnit.zDKEffectBaseDefense = 400000;
    auto zDKTestDomain = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
    zDKTestDomain.zDKEffectStat = zDKEffectStat::zDKEffectDamageReceived;
    zDKTestDomain.zDKEffectValue = 30;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestDomain);
    zDKTestBoss.zDKEffectStats.zDKCalcNormalAttack = 1000000;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle)
            .zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        780000,
        "Domain: (1m - 400k) x 1.3, not 1.3m - 400k"
    );
    zDKTestUnit.zDKEffectEffects.push_back(zDKEffectCreate(zDKEffectCategory::zDKEffectGuard));
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle)
            .zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        260000,
        "Domain also multiplies guarded damage after DEF"
    );
    zDKTestUnit.zDKEffectBaseDefense = 2000000;
    zDKTestExpectNear(
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle)
            .zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMaximumDamage,
        0,
        "Domain damage increase preserves mathematical damage floor"
    );

    // Razzer's enemy Broly: Phase 1 +30%, Phase 2 +100%, SA 300% +50%.
    zDKTestBoss = zDKEffectBossProfile{};
    zDKTestBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats = false;
    zDKTestBoss.zDKEffectStats.zDKCalcNormalAttack = 1200000;
    zDKTestBoss.zDKEffectStats.zDKCalcSuperAttackMultiplier = 3;
    zDKTestBoss.zDKEffectStats.zDKCalcEnemySuperRaisePercent = 50;
    auto zDKTestBossBoost = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic, true);
    zDKTestBossBoost.zDKEffectValue = 30;
    zDKTestBoss.zDKEffectEffects.push_back(zDKTestBossBoost);
    zDKTestEvaluation = zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    zDKTestExpectNear(
        zDKCalcNormalAttackStat(zDKTestEvaluation.zDKEffectResolvedBoss), 1560000, "Broly turn 1 before SA"
    );
    zDKTestExpectNear(
        zDKCalcSuperAttackStat(zDKTestEvaluation.zDKEffectResolvedBoss),
        5460000,
        "Broly SA adds 50 percentage points to 300%"
    );
    zDKTestBoss.zDKEffectStats.zDKCalcEnemyAlreadySupered = true;
    zDKTestExpectNear(
        zDKCalcNormalAttackStat(
            zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectResolvedBoss
        ),
        2340000,
        "Broly post-SA normals"
    );
    zDKTestBoss.zDKEffectEffects[0].zDKEffectValue = 150;
    zDKTestBossBoost.zDKEffectValue = 100;
    zDKTestBossBoost.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectMidBattle;
    zDKTestBoss.zDKEffectEffects.push_back(zDKTestBossBoost);
    zDKTestEvaluation = zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    zDKTestExpectNear(
        zDKCalcSuperAttackStat(zDKTestEvaluation.zDKEffectResolvedBoss),
        21000000,
        "Broly full buildup: 1.2m x 2.5 x 2 x 3.5"
    );
    zDKTestExpectNear(
        zDKCalcNormalAttackStat(zDKTestEvaluation.zDKEffectResolvedBoss),
        9000000,
        "Broly full buildup post-SA normals"
    );
    zDKTestBoss.zDKEffectStats.zDKCalcSuperAttackDownPercent = 20;
    zDKTestExpectNear(
        zDKCalcSuperAttackStat(
            zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectResolvedBoss
        ),
        19800000,
        "SA ATK Down subtracts 20 points from 350%"
    );
    zDKTestExpectNear(
        zDKCalcNormalAttackStat(
            zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectResolvedBoss
        ),
        7800000,
        "SA ATK Down subtracts 20 points from post-SA normals"
    );
    zDKTestBoss.zDKEffectStats.zDKCalcPassiveAttackDownPercent = 30;
    zDKTestBoss.zDKEffectStats.zDKCalcItemAttackDownPercent = 20;
    zDKTestExpectNear(
        zDKCalcSuperAttackStat(
            zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectResolvedBoss
        ),
        11088000,
        "Passive and item ATK Down multiply separately after SA effects"
    );
    zDKTestBoss.zDKEffectStats.zDKCalcSuperAttackDownPercent = 1000;
    zDKTestExpectNear(
        zDKCalcSuperAttackStat(
            zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle).zDKEffectResolvedBoss
        ),
        0,
        "Large active SA debuff clamps attack multiplier to zero"
    );
    zDKTestBoss.zDKEffectStats.zDKCalcSuperAttackDownPercent = 20;
    zDKTestBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats = true;
    bool zDKTestRejected = false;
    try
    {
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestBattle);
    }
    catch (const std::invalid_argument&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Direct ATK rejects ambiguous unbaked SA effects");
    zDKTestBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats = false;

    std::string zDKTestSerialized = zDKStoreSaveSession(zDKTestUnit, zDKTestBoss, zDKTestBattle, true, 1);
    std::istringstream zDKTestSessionInput(zDKTestSerialized);
    zDKEffectUnitProfile zDKTestLoadedUnit;
    zDKEffectBossProfile zDKTestLoadedBoss;
    zDKEffectBattleState zDKTestLoadedBattle;
    bool zDKTestAnimations;
    int zDKTestAttack;
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestSessionInput,
            zDKTestLoadedUnit,
            zDKTestLoadedBoss,
            zDKTestLoadedBattle,
            zDKTestAnimations,
            zDKTestAttack
        ),
        "Research features load from session v4"
    );
    zDKTestExpect(
        zDKStoreSaveSession(
            zDKTestLoadedUnit, zDKTestLoadedBoss, zDKTestLoadedBattle, zDKTestAnimations, zDKTestAttack
        ) == zDKTestSerialized,
        "Research session roundtrip preserves all new fields and effects"
    );
    std::istringstream zDKTestLegacyUnit("DOKKAN_UNIT 2\n\"Legacy unit\"\n10000 400 6 0 0 1 0 0\n0\n");
    zDKTestExpect(zDKStoreLoadUnit(zDKTestLegacyUnit, zDKTestLoadedUnit), "Unit preset v2 remains readable");
    std::istringstream zDKTestLegacySession(
        "DOKKAN_SESSION 3\nDOKKAN_UNIT 2\n\"Legacy unit\"\n10000 400 6 0 0 1 0 0\n0\n\"Legacy "
        "boss\"\n1000000 3 0 2 0 0 0 1.5 0 1 1 0 0 100 1 3000000\n0\n1 700000 700000 1 1 0\n"
    );
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestLegacySession,
            zDKTestLoadedUnit,
            zDKTestLoadedBoss,
            zDKTestLoadedBattle,
            zDKTestAnimations,
            zDKTestAttack
        ),
        "Session v3 remains readable"
    );
    zDKTestExpectNear(
        zDKTestLoadedBoss.zDKEffectStats.zDKCalcPassiveAttackDownPercent,
        0,
        "Legacy session defaults new modifiers to zero"
    );
    zDKTestExpectNear(
        zDKTestLoadedBoss.zDKEffectStats.zDKCalcSuperAttack, 3000000, "Legacy direct SA ATK remains unchanged"
    );
}
