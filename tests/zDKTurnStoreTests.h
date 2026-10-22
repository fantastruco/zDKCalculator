#pragma once

void zDKTestTurnPersistence()
{
    zDKEffectUnitProfile zDKTestUnit;
    zDKTestUnit.zDKEffectBaseDefense = 1000;
    auto zDKTestBuild = zDKEffectCreate(zDKEffectCategory::zDKEffectWhenAttacking);
    zDKTestBuild.zDKEffectValue = 50;
    zDKTestBuild.zDKEffectBuildUpTrigger = zDKTurnBuildUpTrigger::zDKTurnAnyAttack;
    zDKTestBuild.zDKEffectBuildUpCount = 2;
    zDKTestBuild.zDKEffectBuildUpLimit = 8;
    auto zDKTestSuper = zDKEffectCreate(zDKEffectCategory::zDKEffectSuperAttackStack);
    zDKTestSuper.zDKEffectValue = 30;
    zDKTestUnit.zDKEffectEffects = {zDKTestBuild, zDKTestSuper};
    zDKTestUnit.zDKEffectAttackPlan.zDKTurnAttacks = {
        zDKTurnAttackKind::zDKTurnUltra, zDKTurnAttackKind::zDKTurnNormal, zDKTurnAttackKind::zDKTurnSuper
    };
    const auto zDKTestPreset = zDKStoreSaveUnit(zDKTestUnit);
    zDKEffectUnitProfile zDKTestLoaded;
    std::istringstream zDKTestInput(zDKTestPreset);
    zDKTestExpect(
        zDKStoreLoadUnit(zDKTestInput, zDKTestLoaded) && zDKStoreSaveUnit(zDKTestLoaded) == zDKTestPreset,
        "Unit v5 round trip retains ordered attacks, zero raises and passive build-up"
    );
    zDKEffectBossProfile zDKTestBoss;
    auto zDKTestEnemyEffect = zDKEffectCreate(zDKEffectCategory::zDKEffectBasic, true);
    zDKTestEnemyEffect.zDKEffectValue = 25;
    zDKTestBoss.zDKEffectEffects.push_back(zDKTestEnemyEffect);
    zDKEffectBattleState zDKTestBattle;
    bool zDKTestAnimations = false;
    int zDKTestAttack = -1;
    const auto zDKTestSession = zDKStoreSaveSession(zDKTestUnit, zDKTestBoss, zDKTestBattle, true, 1);
    std::istringstream zDKTestSessionInput(zDKTestSession);
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestSessionInput, zDKTestLoaded, zDKTestBoss, zDKTestBattle, zDKTestAnimations, zDKTestAttack
        ) &&
            zDKStoreSaveSession(
                zDKTestLoaded, zDKTestBoss, zDKTestBattle, zDKTestAnimations, zDKTestAttack
            ) == zDKTestSession,
        "Session v6 preserves the complete plan and independent boss effects"
    );
    const std::string zDKTestLegacyUnit =
        "DOKKAN_UNIT 4\n\"Legacy\"\n1000 0 0 0 0 1 0 0\n1\n\"Super Attack\" 3 0 6 30 2 1 0 1 0 0 0 0\n";
    std::istringstream zDKTestLegacyInput(zDKTestLegacyUnit);
    zDKTestExpect(
        zDKStoreLoadUnit(zDKTestLegacyInput, zDKTestLoaded) &&
            zDKTestLoaded.zDKEffectAttackPlan.zDKTurnAttacks.empty() &&
            zDKTestLoaded.zDKEffectEffects[0].zDKEffectCount == 2 &&
            zDKTestLoaded.zDKEffectEffects[0].zDKEffectBuildUpTrigger == zDKTurnBuildUpTrigger::zDKTurnNone,
        "Legacy v4 presets retain their raises and start without an attack plan"
    );
    const std::string zDKTestLegacySession = "DOKKAN_SESSION 5\n" + zDKTestLegacyUnit +
        "\"Boss\"\n1000 1 0 2 0 0 0 1.5 0 1 1 0 0 0 1 3000 0 0 0 0 0\n1\n"
        "\"Basic\" 0 6 0 25 1 1 0 1 0 0 0 0\n1 100 100 1 1 0\n";
    std::istringstream zDKTestLegacySessionInput(zDKTestLegacySession);
    zDKTestExpect(
        zDKStoreLoadSession(
            zDKTestLegacySessionInput,
            zDKTestLoaded,
            zDKTestBoss,
            zDKTestBattle,
            zDKTestAnimations,
            zDKTestAttack
        ) && zDKTestBoss.zDKEffectEffects.size() == 1 &&
            zDKTestBoss.zDKEffectEffects[0].zDKEffectValue == 25,
        "Legacy v5 sessions parse boss effects without consuming new build-up fields"
    );
    const auto zDKTestOriginal = zDKStoreSaveUnit(zDKTestLoaded);
    const std::string zDKTestEmpty = "DOKKAN_UNIT 5\n\"Bad\"\n1000 0 0 0 0 1 0 0\n0\n";
    for (const std::string zDKTestTail : {"17\n", "1 3\n", "1\n"})
    {
        std::istringstream zDKTestBad(zDKTestEmpty + zDKTestTail);
        zDKTestExpect(
            !zDKStoreLoadUnit(zDKTestBad, zDKTestLoaded) &&
                zDKStoreSaveUnit(zDKTestLoaded) == zDKTestOriginal,
            "Invalid or truncated attack plans fail atomically without replacing the loaded unit"
        );
    }
    zDKTestBuild.zDKEffectBuildUpCount = 9;
    zDKTestUnit.zDKEffectEffects[0] = zDKTestBuild;
    bool zDKTestRejected = false;
    try
    {
        zDKStoreSaveUnit(zDKTestUnit);
    }
    catch (const std::exception&)
    {
        zDKTestRejected = true;
    }
    zDKTestExpect(zDKTestRejected, "Starting build-up above its cap cannot be persisted");
}
