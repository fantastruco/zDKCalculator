#pragma once

void zDKTestSupportAndSections()
{
    zDKEffectUnitProfile zDKTestUnit;
    zDKEffectBossProfile zDKTestBoss;
    zDKEffectBattleState zDKTestState;
    for (auto zDKTestCategory :
         {zDKEffectCategory::zDKEffectBasic,
          zDKEffectCategory::zDKEffectEntrance,
          zDKEffectCategory::zDKEffectWhenAttacking,
          zDKEffectCategory::zDKEffectWhenReceiving,
          zDKEffectCategory::zDKEffectSuperAttackStack,
          zDKEffectCategory::zDKEffectUltraSuperAttack,
          zDKEffectCategory::zDKEffectSupport,
          zDKEffectCategory::zDKEffectActive})
    {
        auto zDKTestConditionalGuard = zDKEffectCreate(zDKTestCategory);
        zDKTestConditionalGuard.zDKEffectStat = zDKEffectStat::zDKEffectGuard;
        zDKTestUnit.zDKEffectEffects = {zDKTestConditionalGuard};
        zDKTestExpect(
            zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestState)
                .zDKEffectResolvedUnit.zDKCalcGuardsAllAttacks,
            "Guard is available in every unit effect type"
        );
        zDKTestExpect(
            !zDKEffectUsesSuperCount(zDKTestConditionalGuard),
            "Guard has no SA raise count even inside Super and Ultra Super Attack"
        );
    }
    auto zDKTestConditionalGuard = zDKEffectCreate(zDKEffectCategory::zDKEffectWhenAttacking);
    zDKTestConditionalGuard.zDKEffectStat = zDKEffectStat::zDKEffectGuard;
    zDKTestUnit.zDKEffectEffects = {zDKTestConditionalGuard};
    zDKTestState.zDKEffectAfterUnitAttack = false;
    zDKTestExpect(
        !zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestState)
             .zDKEffectResolvedUnit.zDKCalcGuardsAllAttacks,
        "When-attacking guard is inactive before attacking"
    );
    zDKTestState.zDKEffectAfterUnitAttack = true;
    zDKTestConditionalGuard = zDKEffectCreate(zDKEffectCategory::zDKEffectWhenReceiving);
    zDKTestConditionalGuard.zDKEffectStat = zDKEffectStat::zDKEffectGuard;
    zDKTestConditionalGuard.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectWhenReceivingSuper;
    zDKTestUnit.zDKEffectEffects = {zDKTestConditionalGuard};
    zDKTestExpect(
        !zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestState, false)
             .zDKEffectResolvedUnit.zDKCalcGuardsAllAttacks,
        "Super-only receiving guard excludes normals"
    );
    zDKTestExpect(
        zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestState, true)
            .zDKEffectResolvedUnit.zDKCalcGuardsAllAttacks,
        "Super-only receiving guard activates against supers"
    );
    zDKTestUnit.zDKEffectEffects.clear();
    zDKTestUnit.zDKEffectBaseDefense = 1000;
    auto zDKTestBasic = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic);
    zDKTestBasic.zDKEffectValue = 200;
    auto zDKTestSupport = zDKEffectCreate(zDKEffectCategory::zDKEffectSupport);
    zDKTestSupport.zDKEffectName = "Supporting ally";
    zDKTestSupport.zDKEffectValue = 40;
    zDKTestUnit.zDKEffectEffects = {zDKTestBasic, zDKTestSupport};
    auto zDKTestEvaluateDefense = [&]
    {
        return zDKEffectEvaluate(zDKTestUnit, zDKTestBoss, zDKTestState)
            .zDKEffectResult.zDKCalcDefense.zDKCalcSelected;
    };
    zDKTestExpectNear(
        zDKTestEvaluateDefense(),
        3400,
        "Ally DEF support adds to the unit's SoT passive rather than multiplying it"
    );
    zDKTestSupport.zDKEffectValue = 30;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestSupport);
    zDKTestExpectNear(zDKTestEvaluateDefense(), 3700, "Multiple support allies add within SoT");
    auto zDKTestAttacking = zDKEffectCreate(zDKEffectCategory::zDKEffectWhenAttacking);
    zDKTestAttacking.zDKEffectValue = 100;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestAttacking);
    zDKTestExpectNear(
        zDKTestEvaluateDefense(), 7400, "When attacking multiplies the combined unit passive and ally support"
    );
    zDKTestState.zDKEffectAfterUnitAttack = false;
    zDKTestExpectNear(zDKTestEvaluateDefense(), 3700, "When attacking is inactive before the unit attacks");
    zDKTestState.zDKEffectAfterUnitAttack = true;
    auto zDKTestSuper = zDKEffectCreate(zDKEffectCategory::zDKEffectSuperAttackStack);
    zDKTestSuper.zDKEffectValue = 50;
    zDKTestSuper.zDKEffectCount = 2;
    auto zDKTestUltra = zDKEffectCreate(zDKEffectCategory::zDKEffectUltraSuperAttack);
    zDKTestUltra.zDKEffectValue = 30;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestUltra);
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestSuper);
    zDKTestExpectNear(
        zDKTestEvaluateDefense(),
        17020,
        "Super and Ultra Super raises add to one SA DEF bracket with separate counts"
    );
    zDKTestUnit.zDKEffectEffects.back().zDKEffectEnabled = false;
    zDKTestExpectNear(zDKTestEvaluateDefense(), 9620, "Disabling SA leaves the Ultra SA raise active");
    zDKTestUnit.zDKEffectEffects = {zDKTestBasic};
    zDKTestSupport.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectSuperAttack;
    zDKTestSupport.zDKEffectValue = 30;
    zDKTestSupport.zDKEffectCount = 2;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestSupport);
    zDKTestExpectNear(zDKTestEvaluateDefense(), 4800, "Ally SA support uses SA raises instead of SoT");
    zDKTestUnit.zDKEffectEffects[1].zDKEffectGroup = zDKEffectDefensePhase::zDKEffectActive;
    zDKTestUnit.zDKEffectEffects[1].zDKEffectValue = 33;
    auto zDKTestActive = zDKEffectCreate(zDKEffectCategory::zDKEffectActive);
    zDKTestActive.zDKEffectValue = 67;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestActive);
    zDKTestExpectNear(
        zDKTestEvaluateDefense(),
        6000,
        "Active Skill support adds to own Active Skill and ignores an old SA count"
    );
    zDKTestUnit.zDKEffectEffects = {zDKTestBasic, zDKTestSupport};
    zDKTestUnit.zDKEffectEffects[1].zDKEffectGroup = zDKEffectDefensePhase::zDKEffectStartOfTurn;
    zDKTestUnit.zDKEffectEffects[1].zDKEffectHealthRule = zDKEffectHealthRule::zDKEffectBelow;
    zDKTestUnit.zDKEffectEffects[1].zDKEffectHealthThresholdPercent = 50;
    zDKTestState.zDKEffectMaxHP = 100;
    zDKTestState.zDKEffectCurrentHP = 51;
    zDKTestExpectNear(zDKTestEvaluateDefense(), 3000, "Conditional ally support respects HP");
    zDKTestState.zDKEffectCurrentHP = 50;
    zDKTestExpectNear(zDKTestEvaluateDefense(), 3300, "Conditional ally support includes the HP boundary");
    zDKTestUnit.zDKEffectEffects[1].zDKEffectEnabled = false;
    zDKTestExpectNear(zDKTestEvaluateDefense(), 3000, "Disabled ally support contributes no boost");

    auto zDKTestEntrance = zDKEffectCreate(zDKEffectCategory::zDKEffectEntrance);
    auto zDKTestReceiving = zDKEffectCreate(zDKEffectCategory::zDKEffectWhenReceiving);
    auto zDKTestGuard = zDKEffectCreate(zDKEffectCategory::zDKEffectGuard);
    zDKTestExpect(
        zDKTestGuard.zDKEffectCategory == zDKEffectCategory::zDKEffectBasic &&
            zDKTestGuard.zDKEffectStat == zDKEffectStat::zDKEffectGuard,
        "The guard factory produces a Basic Effect with guard as its statistic"
    );
    zDKTestExpect(
        zDKEffectGetSection(zDKTestGuard) == zDKEffectSection::zDKEffectBasic,
        "Guard is displayed under Basic Effect"
    );
    std::vector<zDKEffectDefinition> zDKTestEntries = {
        zDKTestUltra,
        zDKTestReceiving,
        zDKTestBasic,
        zDKTestAttacking,
        zDKTestEntrance,
        zDKTestSuper,
        zDKTestGuard,
        zDKTestActive
    };
    const auto zDKTestOrder = zDKEffectDisplayOrder(zDKTestEntries);
    zDKTestExpect(
        zDKTestOrder == std::vector<int>({4, 2, 6, 3, 1, 5, 0, 7}),
        "Effect display follows game order and places SA before Ultra SA regardless of insertion"
    );
    zDKTestExpect(
        zDKTestEntries[0].zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack &&
            zDKTestEntries[4].zDKEffectCategory == zDKEffectCategory::zDKEffectEntrance,
        "Display grouping preserves original effect identities and arithmetic source order"
    );
    zDKTestSupport.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectStartOfTurn;
    zDKTestExpect(
        zDKEffectGetSection(zDKTestSupport) == zDKEffectSection::zDKEffectBasic,
        "Ordinary support is grouped alongside basic passive effects"
    );
    zDKTestSupport.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectSuperAttack;
    zDKTestExpect(
        zDKEffectGetSection(zDKTestSupport) == zDKEffectSection::zDKEffectSuperAttack,
        "SA support is grouped with Super Attack effects"
    );
    zDKTestSupport.zDKEffectGroup = zDKEffectDefensePhase::zDKEffectActive;
    zDKTestExpect(
        zDKEffectGetSection(zDKTestSupport) == zDKEffectSection::zDKEffectActive,
        "Active Skill support remains distinct from start-of-turn support"
    );
    zDKTestUnit.zDKEffectEffects = zDKTestEntries;
    zDKTestUnit.zDKEffectEffects.push_back(zDKTestSupport);
    const std::string zDKTestSaved = zDKStoreSaveUnit(zDKTestUnit);
    zDKEffectUnitProfile zDKTestLoaded;
    std::istringstream zDKTestInput(zDKTestSaved);
    zDKTestExpect(
        zDKStoreLoadUnit(zDKTestInput, zDKTestLoaded) && zDKStoreSaveUnit(zDKTestLoaded) == zDKTestSaved,
        "Preset v4 preserves categories, SA/Ultra counts, support source and guard"
    );
    std::istringstream zDKTestOldGuard(
        "DOKKAN_UNIT 3\n\"Old guard\"\n1000 0 0 0 0 1 0 0\n1\n"
        "\"Guard\" 4 0 0 0 1 1 0 1 0 0 0 0\n"
    );
    zDKTestExpect(zDKStoreLoadUnit(zDKTestOldGuard, zDKTestLoaded), "Legacy guard presets still load");
    zDKTestExpect(
        zDKTestLoaded.zDKEffectEffects[0].zDKEffectCategory == zDKEffectCategory::zDKEffectBasic &&
            zDKEffectIsGuard(zDKTestLoaded.zDKEffectEffects[0]),
        "Legacy guard migrates into Basic Effect"
    );
    zDKTestExpect(
        zDKEffectEvaluate(zDKTestLoaded, zDKTestBoss, zDKTestState)
            .zDKEffectResolvedUnit.zDKCalcGuardsAllAttacks,
        "Migrated guard still activates in damage calculations"
    );
    std::istringstream zDKTestOldSession(
        "DOKKAN_SESSION 4\nDOKKAN_UNIT 3\n\"Old unit\"\n1000 0 0 0 0 1 0 0\n1\n"
        "\"Guard\" 4 0 0 0 1 1 0 1 0 0 0 0\n\"Old boss\"\n"
        "1000000 3 0 2 0 0 0 1.5 0 1 1 0 0 100 1 3000000 0 0 0 0 0\n0\n1 100 50 1 1 0\n"
    );
    bool zDKTestOldAnimations = false;
    int zDKTestOldAttack = -1;
    zDKEffectBossProfile zDKTestOldBoss;
    zDKEffectBattleState zDKTestOldState;
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestOldSession,
            zDKTestLoaded,
            zDKTestOldBoss,
            zDKTestOldState,
            zDKTestOldAnimations,
            zDKTestOldAttack
        ),
        "Session v4 remains readable after the effect reorganization"
    );
    zDKTestExpect(
        zDKTestLoaded.zDKEffectEffects[0].zDKEffectCategory == zDKEffectCategory::zDKEffectBasic &&
            zDKEffectIsGuard(zDKTestLoaded.zDKEffectEffects[0]) &&
            zDKTestOldBoss.zDKEffectStats.zDKCalcSuperAttack == 3000000,
        "Session v4 migration preserves guard and independent boss Super ATK"
    );
    const auto zDKTestSession = zDKStoreSaveSession(zDKTestUnit, zDKTestBoss, zDKTestState, true, 0);
    std::istringstream zDKTestSessionInput(zDKTestSession);
    bool zDKTestAnimations = false;
    int zDKTestAttack = -1;
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestSessionInput, zDKTestLoaded, zDKTestBoss, zDKTestState, zDKTestAnimations, zDKTestAttack
        ),
        "Session v5 reads all organized effect types"
    );
    zDKTestExpect(
        zDKStoreSaveSession(zDKTestLoaded, zDKTestBoss, zDKTestState, zDKTestAnimations, zDKTestAttack) ==
            zDKTestSession,
        "Session v5 round trip preserves support and attack types"
    );
}
