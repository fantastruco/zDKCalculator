#pragma once

void zDKTestIntegerRounding()
{
    // Exact ratios must not lose an integer to binary floating point (100 * 0.29).
    zDKTestExpectNear(
        static_cast<double>(zDKCalcApplyMultiplier(100, .29)),
        29,
        "Ratio multiplication avoids binary floating point off-by-one errors"
    );
    zDKTestExpectNear(
        static_cast<double>(zDKCalcApplyMultiplier(137, 1.17)),
        160,
        "A statistic is floored before the next stage"
    );
    zDKTestExpectNear(
        static_cast<double>(zDKCalcApplyMultiplier(160, 1.13)),
        180,
        "Independent factors discard the previous fractional remainder"
    );
    zDKTestExpect(
        zDKCalcApplyMultiplier(zDKCalcMaximumInteger, 1) == zDKCalcMaximumInteger,
        "Largest supported integer survives a neutral factor exactly"
    );
    zDKTestExpect(
        zDKCalcMultiplyRatio(zDKCalcMaximumInteger, 999999999, 1000000000) == 9007199245733791ULL,
        "Large rational product uses exact quotient and remainder arithmetic"
    );
    bool zDKTestRejected = false;
    try
    {
        zDKCalcApplyMultiplier(zDKCalcMaximumInteger, 1.000000001);
    }
    catch (const std::invalid_argument&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Integer overflow is rejected instead of losing precision");

    zDKCalcUnitStats zDKTestUnit;
    zDKCalcEnemyStats zDKTestEnemy;
    zDKTestUnit.zDKCalcBaseDefense = 101.9;
    zDKTestUnit.zDKCalcLeaderSkillPercent = 17;
    zDKTestUnit.zDKCalcStartOfTurnPercent = 66;
    zDKTestUnit.zDKCalcIndependentDefenseFactors[1] = {1.17};
    zDKTestUnit.zDKCalcDomainPercent = 13;
    zDKTestUnit.zDKCalcItemPercent = 19;
    zDKTestUnit.zDKCalcLinkSkillPercent = 11;
    zDKTestUnit.zDKCalcActiveSkillPercent = 23;
    zDKTestUnit.zDKCalcOnAttackPercent = 31;
    zDKTestUnit.zDKCalcSuperAttackStackCount = 3;
    zDKTestUnit.zDKCalcRoundDownDefense = false;
    auto zDKTestResult = zDKCalcCalculateBattle(zDKTestUnit, zDKTestEnemy);
    const double zDKTestExpectedSteps[] = {118, 228, 257, 305, 338, 415, 543, 1357};
    for (int zDKTestIndex = 0; zDKTestIndex < 8; ++zDKTestIndex)
        zDKTestExpectNear(
            zDKTestResult.zDKCalcDefense.zDKCalcSteps[zDKTestIndex].zDKCalcValue,
            zDKTestExpectedSteps[zDKTestIndex],
            "DEF phase has the independently worked integer value"
        );

    // Every cut is separate: floor(101*.83)=83, floor(83*.87)=72, floor(72*.81)=58.
    zDKTestUnit = zDKCalcUnitStats{};
    zDKTestUnit.zDKCalcUseManualDefense = true;
    zDKTestUnit.zDKCalcManualDefense = 101.9;
    zDKTestUnit.zDKCalcDefenseDebuffPercent = 17;
    zDKTestUnit.zDKCalcGuardsAllAttacks = true;
    zDKTestUnit.zDKCalcDamageReductionPercent = 17;
    zDKTestUnit.zDKCalcNormalDamageReductionPercent = 13;
    zDKTestUnit.zDKCalcSuperDamageReductionPercent = 19;
    zDKTestUnit.zDKCalcDamageReceivedPercent = 30;
    zDKTestEnemy.zDKCalcNormalAttack = 137.9;
    zDKTestEnemy.zDKCalcUseSeparateAttackStats = true;
    zDKTestEnemy.zDKCalcSuperAttack = 293.9;
    zDKTestEnemy.zDKCalcNormalAttackFactor = 1.17;
    zDKTestEnemy.zDKCalcSuperAttackFactor = 1.23;
    zDKTestEnemy.zDKCalcPassiveAttackDownPercent = 17;
    zDKTestEnemy.zDKCalcItemAttackDownPercent = 13;
    zDKTestEnemy.zDKCalcAttackModifierPercent = 11;
    zDKTestEnemy.zDKCalcIsCritical = true;
    zDKTestEnemy.zDKCalcDefenseIgnoredPercent = 13;
    zDKTestEnemy.zDKCalcCriticalDefenseIgnoredPercent = 19;
    zDKTestExpectNear(
        zDKCalcNormalAttackStat(zDKTestEnemy),
        114,
        "Normal ATK floors its factor, passive ATK Down and item ATK Down separately"
    );
    zDKTestExpectNear(
        zDKCalcSuperAttackStat(zDKTestEnemy),
        259,
        "Direct SA floors its factor and both ATK Down sources separately"
    );
    zDKTestResult = zDKCalcCalculateBattle(zDKTestUnit, zDKTestEnemy);
    zDKTestExpectNear(zDKTestResult.zDKCalcDefense.zDKCalcSelected, 101, "Manual DEF also floors at input");
    zDKTestExpectNear(zDKTestResult.zDKCalcDefense.zDKCalcEffective, 58, "Each DEF cut floors separately");
    zDKTestExpectNear(
        zDKTestResult.zDKCalcNormalAttackDamage.zDKCalcAdjustedAttack,
        108,
        "Normal modifier, critical, general DR, normal DR and type each floor"
    );
    zDKTestExpectNear(
        zDKTestResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        32,
        "Normal min roll floors guard before final Domain damage"
    );
    zDKTestExpectNear(
        zDKTestResult.zDKCalcNormalAttackDamage.zDKCalcAverageDamage,
        32,
        "Normal mid roll floors variance before subtracting DEF"
    );
    zDKTestExpectNear(
        zDKTestResult.zDKCalcNormalAttackDamage.zDKCalcMaximumDamage,
        33,
        "Normal max roll follows integer stages"
    );
    zDKTestExpectNear(
        zDKTestResult.zDKCalcNormalAttackDamage.zDKCalcRequiredDefense,
        111,
        "Required DEF uses the same truncated max-roll attack as forward damage"
    );
    zDKTestExpectNear(
        zDKTestResult.zDKCalcSuperAttackDamage.zDKCalcAdjustedAttack,
        230,
        "SA uses its own DR before truncating type"
    );
    zDKTestExpectNear(
        zDKTestResult.zDKCalcSuperAttackDamage.zDKCalcMinimumDamage, 111, "SA min roll follows integer stages"
    );
    zDKTestExpectNear(
        zDKTestResult.zDKCalcSuperAttackDamage.zDKCalcAverageDamage, 113, "SA mid roll follows integer stages"
    );
    zDKTestExpectNear(
        zDKTestResult.zDKCalcSuperAttackDamage.zDKCalcMaximumDamage, 115, "SA max roll follows integer stages"
    );

    zDKEffectUnitProfile zDKTestProfile;
    zDKEffectBossProfile zDKTestBoss;
    zDKEffectBattleState zDKTestState;
    zDKTestBoss.zDKEffectStats.zDKCalcNormalAttack = 101.9;
    zDKTestBoss.zDKEffectStats.zDKCalcUseSeparateAttackStats = false;
    zDKTestBoss.zDKEffectStats.zDKCalcSuperAttackMultiplier = 2.15;
    zDKTestBoss.zDKEffectStats.zDKCalcAttackModifierPercent = 11;
    zDKTestBoss.zDKEffectStats.zDKCalcPassiveAttackDownPercent = 17;
    zDKTestBoss.zDKEffectStats.zDKCalcItemAttackDownPercent = 13;
    zDKTestBoss.zDKEffectBaseDefense = 103;
    const zDKEffectDefensePhase zDKTestPhases[] = {
        zDKEffectDefensePhase::zDKEffectStartOfTurn,
        zDKEffectDefensePhase::zDKEffectDomain,
        zDKEffectDefensePhase::zDKEffectItem,
        zDKEffectDefensePhase::zDKEffectMidBattle
    };
    const double zDKTestBoosts[] = {17, 19, 23, 31};
    for (int zDKTestPhase = 0; zDKTestPhase < 4; ++zDKTestPhase)
    {
        auto zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic, true);
        zDKTestEffect.zDKEffectValue = zDKTestBoosts[zDKTestPhase];
        zDKTestEffect.zDKEffectGroup = zDKTestPhases[zDKTestPhase];
        zDKTestBoss.zDKEffectEffects.push_back(zDKTestEffect);
        zDKTestEffect.zDKEffectStat = zDKEffectStat::zDKEffectBossDefense;
        zDKTestBoss.zDKEffectEffects.push_back(zDKTestEffect);
    }
    auto zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic, true);
    zDKTestEffect.zDKEffectMultiplicative = true;
    zDKTestEffect.zDKEffectValue = 13;
    zDKTestBoss.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestEffect.zDKEffectStat = zDKEffectStat::zDKEffectBossDefense;
    zDKTestBoss.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestEffect.zDKEffectStat = zDKEffectStat::zDKEffectSuperAttack;
    zDKTestBoss.zDKEffectEffects.push_back(zDKTestEffect);
    zDKTestEffect.zDKEffectMultiplicative = false;
    zDKTestEffect.zDKEffectValue = 17;
    zDKTestBoss.zDKEffectEffects.push_back(zDKTestEffect);
    auto zDKTestEvaluation = zDKEffectEvaluate(zDKTestProfile, zDKTestBoss, zDKTestState);
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectResolvedBoss.zDKCalcNormalAttack,
        281,
        "Boss shared ATK rounds SoT, independent boost, Domain, item, Phase 2 and modifier"
    );
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectBossDefense, 256, "Boss DEF follows the same separate phase boundaries"
    );
    zDKTestExpectNear(
        zDKCalcSuperAttackStat(zDKTestEvaluation.zDKEffectResolvedBoss),
        575,
        "Boss SA-specific additive and independent effects round separately after SA multiplier"
    );

    zDKTestBoss = zDKEffectBossProfile{};
    zDKTestBoss.zDKEffectStats.zDKCalcNormalAttack = 101;
    for (double zDKTestPercent : {17.0, 13.0, 19.0})
    {
        zDKTestEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
        zDKTestEffect.zDKEffectStat = zDKEffectStat::zDKEffectDamageReduction;
        zDKTestEffect.zDKEffectValue = zDKTestPercent;
        zDKTestEffect.zDKEffectMultiplicative = zDKTestPercent != 17;
        zDKTestProfile.zDKEffectEffects.push_back(zDKTestEffect);
    }
    zDKTestEvaluation = zDKEffectEvaluate(zDKTestProfile, zDKTestBoss, zDKTestState);
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        58,
        "Independent DR sources retain integer rounding boundaries"
    );
    zDKTestProfile.zDKEffectEffects.clear();
    zDKTestBoss.zDKEffectStats.zDKCalcNormalAttack = 137;
    for (double zDKTestPercent : {17.0, 13.0})
    {
        zDKTestEffect.zDKEffectStat = zDKEffectStat::zDKEffectDamageReceived;
        zDKTestEffect.zDKEffectValue = zDKTestPercent;
        zDKTestEffect.zDKEffectMultiplicative = true;
        zDKTestProfile.zDKEffectEffects.push_back(zDKTestEffect);
    }
    zDKTestEvaluation = zDKEffectEvaluate(zDKTestProfile, zDKTestBoss, zDKTestState);
    zDKTestExpectNear(
        zDKTestEvaluation.zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMinimumDamage,
        180,
        "Independent final damage increases do not carry fractional damage"
    );
    zDKTestState.zDKEffectMaxHP = .9;
    zDKTestState.zDKEffectCurrentHP = .5;
    zDKTestExpectNear(
        zDKEffectTeamHealthPercent(zDKTestState),
        0,
        "Sub-unit legacy HP cannot divide by zero after truncation"
    );
    std::istringstream zDKTestLegacy("DOKKAN_UNIT 2\n\"Legacy\"\n101 0 0 0 0 0 0 0\n0\n");
    zDKTestExpect(
        zDKStoreLoadUnit(zDKTestLegacy, zDKTestProfile) && zDKTestProfile.zDKEffectRoundDownDefense,
        "Legacy presets cannot disable mandatory integer rounding"
    );
}
