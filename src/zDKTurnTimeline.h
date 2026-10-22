#pragma once
#include "zDKEffects.h"

namespace zDK
{
struct zDKTurnDefensePoint
{
    int zDKTurnAttackNumber = 0;
    zDKTurnAttackKind zDKTurnKind = zDKTurnAttackKind::zDKTurnNormal;
    int zDKTurnNormalsPerformed = 0, zDKTurnSupersPerformed = 0, zDKTurnUltrasPerformed = 0;
    double zDKTurnNormalDefense = 0, zDKTurnSuperDefense = 0;
    double zDKTurnNormalEffectiveDefense = 0, zDKTurnSuperEffectiveDefense = 0;
    double zDKTurnNormalMaxDamage = 0, zDKTurnSuperMaxDamage = 0;
};

inline std::vector<zDKTurnDefensePoint> zDKTurnCalculateTimeline(
    const zDKEffectUnitProfile& zDKTurnUnit,
    const zDKEffectBossProfile& zDKTurnBoss,
    const zDKEffectBattleState& zDKTurnBattle
)
{
    zDKEffectValidate(zDKTurnUnit);
    zDKEffectValidate(zDKTurnBoss);
    zDKEffectValidate(zDKTurnBattle);
    auto zDKTurnProfile = zDKTurnUnit;
    auto zDKTurnState = zDKTurnBattle;
    zDKTurnState.zDKEffectAfterUnitAttack = false;
    std::vector<zDKTurnDefensePoint> zDKTurnPoints;
    zDKTurnDefensePoint zDKTurnPoint;
    auto zDKTurnAppend = [&]
    {
        auto zDKTurnSnapshot = zDKTurnProfile;
        for (auto& zDKTurnEffect : zDKTurnSnapshot.zDKEffectEffects)
            if (zDKEffectIsGuard(zDKTurnEffect) && zDKTurnEffect.zDKEffectCount == 0 &&
                (zDKTurnEffect.zDKEffectCategory == zDKEffectCategory::zDKEffectSuperAttackStack ||
                 zDKTurnEffect.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack))
                zDKTurnEffect.zDKEffectEnabled = false;
        const auto zDKTurnEvaluation = zDKEffectEvaluate(zDKTurnSnapshot, zDKTurnBoss, zDKTurnState);
        zDKTurnPoint.zDKTurnNormalDefense = zDKTurnEvaluation.zDKEffectNormalDefense.zDKCalcSelected;
        zDKTurnPoint.zDKTurnSuperDefense = zDKTurnEvaluation.zDKEffectSuperDefense.zDKCalcSelected;
        zDKTurnPoint.zDKTurnNormalEffectiveDefense =
            zDKTurnEvaluation.zDKEffectNormalDefense.zDKCalcEffective;
        zDKTurnPoint.zDKTurnSuperEffectiveDefense = zDKTurnEvaluation.zDKEffectSuperDefense.zDKCalcEffective;
        zDKTurnPoint.zDKTurnNormalMaxDamage =
            zDKTurnEvaluation.zDKEffectResult.zDKCalcNormalAttackDamage.zDKCalcMaximumDamage;
        zDKTurnPoint.zDKTurnSuperMaxDamage =
            zDKTurnEvaluation.zDKEffectResult.zDKCalcSuperAttackDamage.zDKCalcMaximumDamage;
        zDKTurnPoints.push_back(zDKTurnPoint);
    };
    zDKTurnAppend();
    for (auto zDKTurnKind : zDKTurnUnit.zDKEffectAttackPlan.zDKTurnAttacks)
    {
        zDKTurnState.zDKEffectAfterUnitAttack = true;
        ++zDKTurnPoint.zDKTurnAttackNumber;
        zDKTurnPoint.zDKTurnKind = zDKTurnKind;
        if (zDKTurnKind == zDKTurnAttackKind::zDKTurnNormal)
            ++zDKTurnPoint.zDKTurnNormalsPerformed;
        if (zDKTurnKind == zDKTurnAttackKind::zDKTurnSuper)
            ++zDKTurnPoint.zDKTurnSupersPerformed;
        if (zDKTurnKind == zDKTurnAttackKind::zDKTurnUltra)
            ++zDKTurnPoint.zDKTurnUltrasPerformed;
        for (auto& zDKTurnEffect : zDKTurnProfile.zDKEffectEffects)
        {
            if (!zDKEffectIsActive(zDKTurnEffect, zDKTurnState, zDKEffectTeamHealthPercent(zDKTurnState)))
                continue;
            const bool zDKTurnMatchingRaise =
                (zDKTurnKind == zDKTurnAttackKind::zDKTurnSuper &&
                 zDKTurnEffect.zDKEffectCategory == zDKEffectCategory::zDKEffectSuperAttackStack) ||
                (zDKTurnKind == zDKTurnAttackKind::zDKTurnUltra &&
                 zDKTurnEffect.zDKEffectCategory == zDKEffectCategory::zDKEffectUltraSuperAttack);
            if (zDKTurnMatchingRaise)
                ++zDKTurnEffect.zDKEffectCount;
            if (zDKTurnMatches(zDKTurnEffect.zDKEffectBuildUpTrigger, zDKTurnKind) &&
                (!zDKTurnEffect.zDKEffectBuildUpLimit ||
                 zDKTurnEffect.zDKEffectBuildUpCount < zDKTurnEffect.zDKEffectBuildUpLimit))
                ++zDKTurnEffect.zDKEffectBuildUpCount;
        }
        zDKTurnAppend();
    }
    return zDKTurnPoints;
}
} // namespace zDK
