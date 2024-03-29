// GENERATED FROM REDMOD SOURCES.
// DO NOT MODIFY BY HAND.

#include "Red/TweakDB/Reflection.hpp"

Red::TweakDBReflection::ExtraFlatMap Red::TweakDBReflection::s_extraFlats = {
    {"gamedataWeakspot_Record", {
        {"Bool", "None", ".saveStatPools"},
    }},
    {"gamedataGameplayRestrictionStatusEffect_Record", {
        {"CName", "None", ".restrictionName"},
    }},
    {"gamedataAttack_Projectile_Record", {
        {"Float", "None", ".puppetBroadphaseHitRadiusSquared"},
        {"Float", "None", ".detonationTimer"},
        {"Float", "None", ".p1PositionZOffset"},
        {"Float", "None", ".vehicleBroadphaseHitRadiusSquared"},
        {"Float", "None", ".armingDistance"},
        {"Bool", "None", ".p2HardCurve"},
        {"Float", "None", ".p1PositionLateralOffset"},
        {"Float", "None", ".p2BendFactor"},
        {"Float", "None", ".p1StartVelocity"},
        {"Bool", "None", ".followTargetInPhase2"},
        {"Float", "None", ".p1AngleInHitPlaneMax"},
        {"Float", "None", ".p2BendRation"},
        {"Float", "None", ".p2AngleInVerticalPlane"},
        {"Float", "None", ".playerPositionGrabTime"},
        {"Float", "None", ".p1DurationMin"},
        {"Float", "None", ".explosionRange"},
        {"Float", "None", ".p2AngleInHitPlaneMax"},
        {"Float", "None", ".p1DurationMax"},
        {"Float", "None", ".finalTargetPositionCalculationDelay"},
        {"Float", "None", ".p2StartVelocity"},
        {"Float", "None", ".hitCooldown"},
        {"Bool", "None", ".startVelocityDirectionCheck"},
        {"Float", "None", ".p1BendFactor"},
        {"Float", "None", ".p1PositionForwardOffsetMin"},
        {"Float", "None", ".p1AngleInVerticalPlane"},
        {"Bool", "None", ".p2ShouldRotate"},
        {"Float", "None", ".armingTime"},
        {"Float", "None", ".explosionRadius"},
        {"Float", "None", ".p1BendTimeRatio"},
        {"Float", "None", ".projectileAccelOverride"},
        {"Float", "None", ".p1SnapRadius"},
        {"Bool", "None", ".usetargetOffsetWhenUntargeted"},
        {"Float", "None", ".p1PositionForwardOffsetMax"},
        {"Float", "None", ".p2AngleInHitPlaneMin"},
        {"Float", "None", ".p1AngleInHitPlaneMin"},
        {"Float", "None", ".gravitySimulation"},
        {"Float", "None", ".targetPositionXYAdditive"},
        {"Float", "None", ".targetPositionOffset"},
        {"Float", "None", ".p2HardCurveInterpolationTimeRatio"},
        {"Bool", "None", ".inheritVehicleSpeed"},
    }},
    {"gamedataItem_Record", {
        {"Float", "None", ".rampDownFactor"},
        {"Bool", "None", ".canTargetDevices"},
        {"CName", "None", ".quickActionlaunchMode"},
        {"Float", "None", ".lifetime"},
        {"Float", "None", ".quickActionChargeCost"},
        {"Float", "None", ".amountOfCapacityGivenForRipperdocUI"},
        {"CName", "None", ".collisionActionCharged"},
        {"Float", "None", ".detonationDelay"},
        {"Float", "None", ".angleInterpolationDuration"},
        {"Float", "None", ".onCollisionStimBroadcastRadius"},
        {"Float", "None", ".rampUpDistanceEnd"},
        {"array:TweakDBID", "StatusEffectType", ".statusEffectsTypeCostReductors"},
        {"String", "None", ".overrideAttack"},
        {"Float", "None", ".waterSurfaceImpactImpulseStrength"},
        {"Float", "None", ".endLeanAngle"},
        {"CName", "None", ".chargeActionlaunchMode"},
        {"Float", "None", ".linearTimeRatio"},
        {"Bool", "None", ".canTargetVehicles"},
        {"Float", "None", ".rampDownDistanceEnd"},
        {"CName", "None", ".shardType"},
        {"CName", "None", ".collisionAction"},
        {"Float", "None", ".rampDownDistanceStart"},
        {"Float", "None", ".interpolationTimeRatio"},
        {"Float", "None", ".angle"},
        {"Float", "None", ".returnTimeMargin"},
        {"Float", "None", ".onCollisionStimBroadcastLifetime"},
        {"Float", "None", ".finisherThresholdModifier"},
        {"String", "None", ".relicAttack"},
        {"String", "None", ".secondaryAttack"},
        {"Bool", "None", ".randomizeDirection"},
        {"Float", "None", ".bonusThermalDamageFactor"},
        {"Float", "None", ".halfLeanAngle"},
        {"Float", "None", ".startVelocityCharged"},
        {"CName", "None", ".cyberwareType"},
        {"Bool", "None", ".hideCooldownUI"},
        {"Float", "None", ".revealPositionValue"},
        {"Float", "None", ".exitCombatDelay"},
        {"Float", "None", ".maxCostReduction"},
        {"CName", "None", ".projectileTemplateName"},
        {"Float", "None", ".startVelocity"},
        {"Float", "None", ".costReductionPerReductorStack"},
        {"Float", "None", ".sinkingDetonationDelay"},
        {"Bool", "None", ".hideDurationUI"},
        {"Float", "None", ".rampUpDistanceStart"},
        {"String", "None", ".attack"},
        {"Float", "None", ".waterSurfaceImpactImpulseRadius"},
        {"CName", "None", ".onCollisionStimType"},
        {"Int32", "None", ".maxBounceCount"},
        {"TweakDBID", "gamedataConstantStatModifier_Record", ".price"},
        {"Float", "None", ".chargeActionChargeCost"},
        {"Bool", "None", ".applyAdditiveProjectileSpiraling"},
        {"Float", "None", ".waterDetonationImpulseStrength"},
        {"CName", "None", ".launchTrajectory"},
        {"Bool", "None", ".randomizePhase"},
        {"Vector3", "None", ".gravitySimulation"},
        {"Float", "None", ".deepWaterDepth"},
        {"Bool", "None", ".isIKEnabled"},
        {"Float", "None", ".waterDetonationImpulseRadius"},
        {"CName", "None", ".modType"},
        {"Float", "None", ".bendFactor"},
        {"Float", "None", ".bendTimeRatio"},
        {"Float", "None", ".energyLossFactor"},
    }},
    {"gamedatadevice_scanning_data_Record", {
        {"Int32", "None", ".revealOrder"},
    }},
    {"gamedataConsumableItem_Record", {
        {"array:Float", "None", ".healthIncreasePerUse"},
        {"Float", "None", ".amountOfCapacityGivenForRipperdocUI"},
        {"array:Float", "None", ".staminaNotificationValues"},
        {"array:Float", "None", ".memoryNotificationValues"},
        {"array:Float", "None", ".staminaRegenRateIncreasePerUse"},
        {"array:Float", "None", ".memoryRegenRateIncreasePerUse"},
    }},
    {"gamedataVehicle_Record", {
        {"String", "None", ".hijackDifficulty"},
        {"String", "None", ".fullDisplayName"},
        {"String", "None", ".crackLockDifficulty"},
    }},
    {"gamedataInteractionBase_Record", {
        {"CName", "None", ".tag"},
    }},
    {"gamedataModifyStatPoolModifierEffector_Record", {
        {"TweakDBID", "gamedataStatModifierGroup_Record", ".statGroup"},
    }},
    {"gamedatadevice_role_action_desctiption_Record", {
        {"Int32", "None", ".revealOrder"},
        {"Bool", "None", ".isQuickHack"},
        {"String", "None", ".skillcheck"},
    }},
    {"gamedataGrenade_Record", {
        {"Float", "None", ".stopAttackDelay"},
        {"Bool", "None", ".explodeOnImpact"},
        {"Float", "None", ".delayToDetonate"},
        {"Int32", "None", ".numberOfHitsForAdditionalAttack"},
        {"Float", "None", ".addAxisRotationSpeedMax"},
        {"Float", "None", ".freezingDuration"},
        {"Float", "None", ".addAxisRotationSpeedMin"},
        {"Float", "None", ".freezeDelayAfterBounce"},
        {"Bool", "None", ".additionalAttackOnDetonate"},
        {"Float", "None", ".minimumDistanceFromFloor"},
        {"Float", "None", ".smokeEffectRadius"},
        {"Float", "None", ".effectCooldown"},
        {"Float", "None", ".addAxisRotationDelay"},
        {"Float", "None", ".smokeEffectDuration"},
        {"Bool", "None", ".isContinuousEffect"},
    }},
    {"gamedataNewPerk_Record", {
        {"Float", "None", ".memoryCostReduction"},
        {"Float", "None", ".offsetMult"},
        {"Float", "None", ".timeDilationDuration"},
        {"Float", "None", ".poiseStrongMeleeMultiplier"},
        {"Int32", "None", ".focusedStaminaCost"},
        {"Float", "None", ".baseOffset"},
        {"Float", "None", ".distanceMaxStacks"},
        {"Int32", "None", ".maxTargets"},
        {"Float", "None", ".crouchSprintStaminaCostMultiplier"},
        {"Bool", "None", ".blockSameQhTypeQueuingOnVehicles"},
        {"Float", "None", ".amountOfCapacityGivenForRipperdocUI"},
        {"Float", "None", ".poiseQuickMeleeMultiplier"},
        {"Float", "None", ".poiseBossLowHealthMultiplier"},
        {"Float", "None", ".timeDilationStrength"},
        {"Int32", "None", ".staminaCostReduction"},
        {"Float", "None", ".distanceMult"},
        {"Bool", "None", ".disallowDeviceQhQueue"},
        {"array:CName", "None", ".preventInQueueAgain"},
        {"Float", "None", ".memoryReturnFactor"},
        {"Float", "None", ".durationIncease"},
        {"Float", "None", ".overshieldPercentage"},
        {"Float", "None", ".focusedStaminaThreshold"},
        {"Float", "None", ".distanceBase"},
        {"Float", "None", ".durationMultiplier"},
        {"Float", "None", ".graceChance"},
        {"Float", "None", ".graceChanceThrowable"},
        {"Float", "None", ".poiseDebufMultiplier"},
    }},
    {"gamedataApplyStatusEffectEffector_Record", {
        {"Bool", "None", ".isRandom"},
    }},
    {"gamedataScannableData_Record", {
        {"Int32", "None", ".revealOrder"},
    }},
    {"gamedataLootTable_Record", {
        {"TweakDBID", "gamedataItem_Record", ".itemID"},
    }},
    {"gamedataApplyStatGroupEffector_Record", {
        {"Bool", "None", ".reapplyOnWeaponChange"},
    }},
    {"gamedataCrackAction_Record", {
        {"TweakDBID", "gamedataEffector_Record", ".effectorToApply"},
        {"CName", "None", ".factToCheck"},
    }},
    {"gamedataAttachmentSlot_Record", {
        {"String", "None", ".unlockedBy"},
    }},
    {"gamedataAttack_GameEffect_Record", {
        {"Float", "None", ".chance"},
        {"Float", "None", ".attackEffectDuration"},
        {"Float", "None", ".damageBasedOnMissingMemoryBonusMax"},
        {"Float", "None", ".attackEffectDelay"},
        {"Float", "None", ".normalNPCMaxHPDamage"},
        {"Float", "None", ".overridePlayerDamageFixedPercentage"},
        {"Float", "None", ".nonEliteDamageBonusMultiplier"},
        {"Float", "None", ".damageBasedOnMissingMemoryBonusMultiplier"},
        {"Float", "None", ".trashNPCMaxHPDamage"},
        {"Float", "None", ".cyberwareMalfunctionDamageBonus"},
        {"Float", "None", ".eliteNPCMaxHPDamage"},
        {"Float", "None", ".weakNPCMaxHPDamage"},
        {"TweakDBID", "gamedataAttackDirection_Record", ".attackDirection"},
        {"Float", "None", ".bossNPCMaxHPDamage"},
        {"Float", "None", ".hitCooldown"},
        {"Float", "None", ".rareNPCMaxHPDamage"},
    }},
    {"gamedataAimAssistMagnetism_Record", {
        {"Bool", "None", ".finishingEnabled"},
    }},
    {"gamedataProgressionBuild_Record", {
        {"array:TweakDBID", "StatModifier", ".statModifiers"},
    }},
    {"gamedataInventoryItem_Record", {
        {"array:TweakDBID", "Item", ".recipes"},
        {"array:TweakDBID", "Item", ".programs"},
    }},
    {"gamedataHitPrereqCondition_Record", {
        {"String", "None", ".ammoState"},
        {"CName", "None", ".comparisonSource"},
        {"String", "None", ".weaponEvolution"},
        {"TweakDBID", "gamedataHackCategory_Record", ".hackCategory"},
        {"CName", "None", ".bodyPart"},
        {"String", "None", ".damageType"},
        {"String", "None", ".rarity"},
        {"String", "None", ".comparisonType"},
        {"CName", "None", ".comparisonTarget"},
        {"Bool", "None", ".isTheNextQhInQueue"},
        {"String", "None", ".hitFlag"},
        {"String", "None", ".attackType"},
        {"Bool", "None", ".isMoving"},
        {"String", "None", ".npcType"},
        {"String", "None", ".statToCompare"},
        {"Int32", "None", ".consecutiveHitsRequired"},
        {"Float", "None", ".distanceRequired"},
        {"CName", "None", ".attackTag"},
        {"TweakDBID", "gamedataCheckType_Record", ".checkType"},
        {"CName", "None", ".targetType"},
        {"Float", "None", ".valueToCheck"},
        {"String", "None", ".reactionPreset"},
        {"String", "None", ".dotType"},
        {"CName", "None", ".weaponType"},
        {"CName", "None", ".statusEffect"},
        {"Float", "None", ".timeOut"},
        {"Float", "None", ".ratioToCompare"},
        {"CName", "None", ".instigatorType"},
        {"CName", "None", ".tagToCheck"},
        {"TweakDBID", "gamedataTriggerMode_Record", ".triggerMode"},
        {"CName", "None", ".effectName"},
        {"CName", "None", ".objectToCheck"},
        {"String", "None", ".attackSubtype"},
        {"Bool", "None", ".uniqueTarget"},
        {"CName", "None", ".source"},
        {"Bool", "None", ".invertHitFlag"},
        {"CName", "None", ".object"},
        {"String", "None", ".statPoolToCompare"},
        {"String", "None", ".weaponItemType"},
    }},
    {"gamedataWeaponItem_Record", {
        {"Float", "None", ".weaponNearPlane"},
        {"Float", "None", ".weaponVignetteIntensity"},
        {"Bool", "None", ".scaleToPlayer"},
        {"Bool", "None", ".hide_nametag"},
        {"Float", "None", ".weaponBlurIntensity_aim"},
        {"Float", "None", ".slowDuration"},
        {"Float", "None", ".maxRandomDelay"},
        {"Float", "None", ".upwardsVectorStrength"},
        {"Float", "None", ".weaponFarPlane_aim"},
        {"Float", "None", ".weaponFarPlane"},
        {"Float", "None", ".weaponNearPlane_aim"},
        {"Bool", "None", ".scaleToPlayerInStash"},
        {"CName", "None", ".cyberwareType"},
        {"Vector3", "None", ".position"},
        {"Float", "None", ".specialAttackCost"},
        {"Float", "None", ".impulseOfOnePunch"},
        {"Float", "None", ".weaponVignetteRadius_aim"},
        {"Float", "None", ".explosionRangeBonus"},
        {"Float", "None", ".weaponVignetteRadius"},
        {"Float", "None", ".projectileSmartTargetingAngle"},
        {"Float", "None", ".minRandomDelay"},
        {"Float", "None", ".projectileSmartTargetingDistance"},
        {"Float", "None", ".colliderBoxHeight"},
        {"Float", "None", ".weaponVignetteCircular"},
        {"CName", "None", ".projectileTemplateName"},
        {"Float", "None", ".weaponEdgesSharpness_aim"},
        {"Int32", "None", ".specialAttackNumber"},
        {"Float", "None", ".offset"},
        {"Bool", "None", ".divideAttacksByPelletsOnUI"},
        {"Float", "None", ".ironsightAngleWithScope"},
        {"Float", "None", ".weaponSwapOnAttackDelay"},
        {"Float", "None", ".weaponBlurIntensity"},
        {"Int32", "None", ".numberOfHandAnims"},
        {"Float", "None", ".colliderBoxWidth"},
        {"Float", "None", ".minimumReloadTime"},
        {"TweakDBID", "gamedataConstantStatModifier_Record", ".price"},
        {"Float", "None", ".onePunchAttackDelay"},
        {"Float", "None", ".weaponVignetteCircular_aim"},
        {"Float", "None", ".slowMoDelay"},
        {"Bool", "None", ".useProjectileAppearance"},
        {"Float", "None", ".weaponVignetteIntensity_aim"},
        {"Bool", "None", ".saveStatPools"},
        {"CName", "None", ".specific_player_appearance"},
        {"CName", "None", ".audioEventName"},
        {"Float", "None", ".weaponEdgesSharpness"},
        {"Float", "None", ".grandFinaleAttackDelay"},
        {"Float", "None", ".vfxDelay"},
        {"Float", "None", ".targetMaxDistanceCheck"},
        {"Float", "None", ".slowMoAmount"},
        {"Float", "None", ".safeActionDuration"},
    }},
    {"gamedataModifyStatPoolValueEffector_Record", {
        {"Float", "None", ".healingItemChargeRestorePercentage"},
        {"CName", "None", ".applicationTarget"},
        {"Float", "None", ".modificationValue"},
    }},
    {"gamedataStatusEffectPrereq_Record", {
        {"Bool", "None", ".fireAndForget"},
    }},
    {"gamedataVehicleTPPCameraPresetParams_Record", {
        {"Float", "None", ".boomLength"},
    }},
    {"gamedataSubCharacter_Record", {
        {"Bool", "None", ".humanoid"},
        {"Float", "None", ".tiltAngleOnSpeed"},
        {"String", "None", ".relaxedSensesPreset"},
        {"CName", "None", ".weaponSlot"},
        {"Float", "None", ".sizeRight"},
        {"Float", "None", ".turnInertiaDamping"},
        {"String", "None", ".statusEffectParamsPackageName"},
        {"Float", "None", ".sizeFront"},
        {"Float", "None", ".sizeLeft"},
        {"Float", "None", ".startingRecoveryBalance"},
        {"Float", "None", ".combatDefaultZOffset"},
        {"Float", "None", ".walkTiltCoefficient"},
        {"Float", "None", ".mass"},
        {"Float", "None", ".pseudoAcceleration"},
        {"Float", "None", ".massNormalizedCoefficient"},
        {"Float", "None", ".sizeBack"},
        {"String", "None", ".alertedSensesPreset"},
        {"String", "None", ".combatSensesPreset"},
        {"Float", "None", ".speedIdleThreshold"},
    }},
    {"gamedataSpreadEffector_Record", {
        {"Bool", "None", ".applyOverclock"},
    }},
    {"gamedataNewSkillsProficiency_Record", {
        {"Float", "None", ".amountOfCapacityGivenForRipperdocUI"},
    }},
    {"gamedataEffector_Record", {
        {"Bool", "None", ".stopActiveSfxOnDeactivate"},
        {"Float", "None", ".percentToRefund"},
        {"Bool", "None", ".isBlockingCompletely"},
        {"Bool", "None", ".improveWithDistance"},
        {"String", "None", ".actionID"},
        {"Float", "None", ".damageConversion"},
        {"Float", "None", ".visionBlockerDetectionModifier"},
        {"CName", "None", ".visionBlockerTypeName"},
        {"Float", "None", ".timeWindow"},
        {"CName", "None", ".effectTag"},
        {"array:String", "None", ".effectTypes"},
        {"Bool", "None", ".listenConstantly"},
        {"Bool", "None", ".onlyHostileDetection"},
        {"Float", "None", ".visionBlockerTBHModifier"},
        {"Float", "None", ".minTimeBetweenBlocks"},
        {"Float", "None", ".height"},
        {"Float", "None", ".dotDistributionTime"},
        {"Float", "None", ".probability"},
        {"Int32", "None", ".nanoPlatesStacks"},
        {"Int32", "None", ".valueOff"},
        {"Float", "None", ".criticalHealthThreshold"},
        {"Bool", "None", ".blocksParent"},
        {"Int32", "None", ".minStacks"},
        {"array:CName", "None", ".gameplayTags"},
        {"Float", "None", ".revealDuration"},
        {"Float", "None", ".fromHeight"},
        {"Float", "None", ".chanceIncrement"},
        {"String", "None", ".poolStatus"},
        {"Float", "None", ".maxDistance"},
        {"String", "None", ".attributeType"},
        {"Float", "None", ".delay"},
        {"CName", "None", ".reason"},
        {"Float", "None", ".maxPercentMult"},
        {"Float", "None", ".uploadTime"},
        {"Bool", "None", ".startOnUninitialize"},
        {"Int32", "None", ".maxStacks"},
        {"Float", "None", ".distance"},
        {"Int32", "None", ".valueOn"},
        {"Float", "None", ".baseChance"},
        {"Float", "None", ".chanceStep"},
        {"Int32", "None", ".targetEquipSlotIndex"},
        {"Float", "None", ".minDistance"},
        {"TweakDBID", "gamedataAttack_Record", ".attackRecord"},
        {"Float", "None", ".minPlayerHealthPercentage"},
        {"String", "None", ".statPoolType"},
        {"Int32", "None", ".targetHowManyEnemies"},
        {"TweakDBID", "gamedataObjectAction_Record", ".objectAction"},
        {"CName", "None", ".sfxToStop"},
        {"Float", "None", ".startingThreshold"},
        {"TweakDBID", "gamedataStatPool_Record", ".statPool"},
        {"Float", "None", ".detectionStep"},
        {"String", "None", ".networkAction"},
        {"CName", "None", ".vfxToStop"},
        {"CName", "None", ".vfxToStart"},
        {"TweakDBID", "gamedataStatusEffect_Record", ".statusEffectForSelf"},
        {"Bool", "None", ".stepUsesPercent"},
        {"Float", "None", ".playerIncomingDamageMultiplier"},
        {"CName", "None", ".applicationTarget"},
        {"Bool", "None", ".dontRemoveStacks"},
        {"TweakDBID", "gamedataStatusEffect_Record", ".statusEffectForTarget"},
        {"Float", "None", ".healAmount"},
        {"CName", "None", ".effectName"},
        {"CName", "None", ".sfxToStart"},
        {"Float", "None", ".damageReduction"},
        {"Float", "None", ".unitThreshold"},
        {"Vector3", "None", ".visionBlockerOffset"},
        {"CName", "None", ".sfxName"},
        {"String", "None", ".pointsType"},
        {"Bool", "None", ".countOnlyUnique"},
        {"CName", "None", ".vfxName"},
        {"Float", "None", ".change"},
        {"CName", "None", ".appearanceName"},
        {"array:CName", "None", ".tags"},
        {"Bool", "None", ".unique"},
        {"Float", "None", ".critChanceBonus"},
        {"Float", "None", ".duration"},
        {"Float", "None", ".damageBuffAmount"},
        {"Float", "None", ".targetMaxDistance"},
        {"String", "None", ".operationType"},
        {"Float", "None", ".visionBlockerRadius"},
        {"CName", "None", ".deactivationSFXName"},
        {"array:Float", "None", ".vectorRotations"},
        {"Bool", "None", ".roundUpwards"},
        {"Float", "None", ".chanceToTrigger"},
        {"CName", "None", ".playVFXOnHitTargets"},
        {"array:CName", "None", ".effectTags"},
        {"String", "None", ".refObj"},
        {"CName", "None", ".animFeatureName"},
        {"Float", "None", ".level"},
        {"String", "None", ".secondaryText"},
        {"Float", "None", ".blendInDuration"},
        {"Bool", "None", ".applyToWeapon"},
        {"array:CName", "None", ".factSuffixes"},
        {"Float", "None", ".playerMaxIncomingDamage"},
        {"String", "None", ".statusEffect"},
        {"Float", "None", ".amplitudeWeight"},
        {"CName", "None", ".activationSFXName"},
        {"Float", "None", ".range"},
        {"Bool", "None", ".resetAppearance"},
        {"String", "None", ".primaryText"},
        {"array:String", "None", ".statusEffects"},
        {"Float", "None", ".maxDmg"},
        {"Bool", "None", ".affectsPlayer"},
        {"Float", "None", ".width"},
        {"CName", "None", ".easeOutCurve"},
        {"Float", "None", ".statPoolValue"},
        {"String", "None", ".effectPath"},
        {"Bool", "None", ".fireAndForget"},
        {"Bool", "None", ".isPercentage"},
        {"CName", "None", ".overrideMaterialName"},
        {"TweakDBID", "gamedataStat_Record", ".statForValue"},
        {"Float", "None", ".percentMult"},
        {"String", "None", ".statusEffectToRemove"},
        {"String", "None", ".hitFlag"},
        {"Float", "None", ".blendOutDuration"},
        {"Float", "None", ".dilation"},
        {"Float", "None", ".uploadChance"},
        {"Float", "None", ".recoverMemoryAmount"},
        {"Bool", "None", ".thresholdUsesPercent"},
        {"Int32", "None", ".weakSpotIndex"},
        {"Bool", "None", ".usePercent"},
        {"Float", "None", ".damageBuffRarity"},
        {"Float", "None", ".maxRangeAddition"},
        {"Float", "None", ".statPoolStep"},
        {"Bool", "None", ".applyToOwner"},
        {"Float", "None", ".value"},
        {"Bool", "None", ".skipLastCombatHack"},
        {"Float", "None", ".armorMultiplier"},
        {"String", "None", ".targetEquipArea"},
        {"String", "None", ".targetOfStatPoolCheck"},
        {"Float", "None", ".targetMinDistance"},
        {"CName", "None", ".easeInCurve"},
        {"CName", "None", ".overrideMaterialTag"},
        {"Bool", "None", ".inverted"},
        {"Float", "None", ".visionBlockerHeight"},
        {"TweakDBID", "gamedataStatusEffect_Record", ".statusEffectRecord"},
        {"CName", "None", ".fact"},
    }},
    {"gamedataStatusEffect_Record", {
        {"Float", "None", ".costReductionPerStack"},
        {"Float", "None", ".expModifier"},
        {"CName", "None", ".vfxToStart"},
        {"Bool", "None", ".canReapply"},
        {"String", "None", ".displayName"},
        {"String", "None", ".iconPath"},
        {"TweakDBID", "gamedataStatModifier_Record", ".maxDuration"},
        {"Float", "None", ".armorMeltMaxPercent"},
        {"array:TweakDBID", "Effector", ".effectors"},
        {"array:CName", "None", ".vfx"},
    }},
    {"gamedataClothing_Record", {
        {"Bool", "None", ".scaleToPlayer"},
    }},
    {"gamedataVehicleDataPackage_Record", {
        {"Float", "None", ".exiting"},
    }},
    {"gamedataTriggerHackingMinigameEffector_Record", {
        {"CName", "None", ".factName"},
        {"Int32", "None", ".factValue"},
        {"Bool", "None", ".showPopup"},
    }},
    {"gamedataCharacter_Record", {
        {"CName", "None", ".BodyDisposalFact"},
        {"Bool", "None", ".hasToBeKilledInWounded"},
        {"Float", "None", ".turnInertiaDamping"},
        {"Bool", "None", ".hide_nametag"},
        {"String", "None", ".relaxedSensesPreset"},
        {"CName", "None", ".weaponSlot"},
        {"CName", "None", ".preferedWeapon"},
        {"Bool", "None", ".hide_nametag_displayname"},
        {"CName", "None", ".globalSquads"},
        {"array:CName", "None", ".communitySquads"},
        {"CName", "None", ".factCounterName"},
        {"String", "None", ".statusEffectParamsPackageName"},
        {"Float", "None", ".sizeFront"},
        {"Float", "None", ".pseudoAcceleration"},
        {"Float", "None", ".shieldControllerDestroyed_staggerThreshold"},
        {"Int32", "None", ".amountOfQuests"},
        {"CName", "None", ".additionalDescription"},
        {"Float", "None", ".airDeathRagdollDelay"},
        {"Bool", "None", ".temp_doNotNotifySS"},
        {"Float", "None", ".speedIdleThreshold"},
        {"Int32", "None", ".level"},
        {"Float", "None", ".tiltAngleOnSpeed"},
        {"Float", "None", ".sizeRight"},
        {"TweakDBID", "gamedataUIIcon_Record", ".tooltipAvatar"},
        {"Float", "None", ".combatDefaultZOffset"},
        {"Float", "None", ".sizeLeft"},
        {"Float", "None", ".startingRecoveryBalance"},
        {"Bool", "None", ".keepColliderOnDeath"},
        {"Float", "None", ".walkTiltCoefficient"},
        {"Float", "None", ".mass"},
        {"array:Int32", "None", ".thresholds"},
        {"Bool", "None", ".nameplate"},
        {"Bool", "None", ".saveStatPools"},
        {"Float", "None", ".massNormalizedCoefficient"},
        {"Float", "None", ".sizeBack"},
        {"String", "None", ".alertedSensesPreset"},
        {"String", "None", ".combatSensesPreset"},
    }},
    {"gamedataDevice_Record", {
        {"String", "None", ".deviceType"},
        {"Float", "None", ".UI_PlayerScanningTime"},
    }},
    {"gamedataStat_Record", {
        {"Float", "None", ".value"},
        {"Float", "None", ".impactValue"},
        {"Float", "None", ".staggerValue"},
        {"Bool", "None", ".isPercentage"},
        {"Bool", "None", ".shouldFlipNegativeValue"},
    }},
    {"gamedataParentAttachmentType_Record", {
        {"String", "None", ".comment"},
    }},
    {"gamedataGadget_Record", {
        {"TweakDBID", "gamedataConstantStatModifier_Record", ".price"},
    }},
    {"gamedatadevice_gameplay_role_Record", {
        {"Int32", "None", ".revealOrder"},
    }},
    {"gamedataMinigameAction_Record", {
        {"CName", "None", ".targetClass"},
    }},
    {"gamedataTriggerAttackEffector_Record", {
        {"CName", "None", ".woundType"},
        {"Float", "None", ".chance"},
        {"Bool", "None", ".isRandom"},
        {"Float", "None", ".applicationChance"},
        {"CName", "None", ".bodyPart"},
        {"Vector3", "None", ".hitPosition"},
        {"CName", "None", ".attackPositionSlotName"},
        {"Bool", "None", ".isCritical"},
        {"Bool", "None", ".playerAsInstigator"},
        {"TweakDBID", "gamedataStat_Record", ".statForChance"},
        {"Bool", "None", ".shouldDelay"},
        {"Bool", "None", ".useHitPosition"},
        {"Bool", "None", ".triggerHitReaction"},
    }},
    {"gamedataFocusClue_Record", {
        {"Int32", "None", ".revealOrder"},
    }},
    {"gamedataHudEnhancer_Record", {
        {"Float", "None", ".coneAngle"},
    }},
    {"gamedataItemAction_Record", {
        {"String", "None", ".journalEntry"},
    }},
    {"gamedataMinigame_Def_Record", {
        {"array:TweakDBID", "Program", ".forbiddenPrograms"},
    }},
    {"gamedataRangedAttack_Record", {
        {"CName", "None", ".explosionAttack"},
        {"Float", "None", ".hitCooldown"},
    }},
    {"gamedataContentAssignment_Record", {
        {"Int32", "None", ".overrideValue"},
        {"Bool", "None", ".upToCheck"},
    }},
    {"gamedataAttack_Landing_Record", {
        {"Float", "None", ".maxRange"},
    }},
    {"gamedataContinuousEffector_Record", {
        {"Float", "None", ".maxEnemyDistance"},
        {"Float", "None", ".kInit"},
        {"CName", "None", ".targetingSet"},
        {"Bool", "None", ".onlyClosestToCrosshair"},
        {"Float", "None", ".b"},
        {"TweakDBID", "gamedataStatPool_Record", ".statPoolType"},
        {"Float", "None", ".maxDistance"},
        {"Float", "None", ".maxDecay"},
        {"TweakDBID", "gamedataStatusEffect_Record", ".statusEffect"},
        {"Float", "None", ".k"},
        {"Bool", "None", ".highlightVisible"},
        {"CName", "None", ".searchFilter"},
        {"Bool", "None", ".onlyClosestByDistance"},
        {"Bool", "None", ".onlyWhileAiming"},
    }},
    {"gamedataStatPoolPrereq_Record", {
        {"Bool", "None", ".invert"},
        {"Float", "None", ".perfectDischargeWindow"},
    }},
    {"gamedataObjectAction_Record", {
        {"Bool", "None", ".isQuickHack"},
        {"Bool", "None", ".isFlatheadAction"},
        {"TweakDBID", "gamedataStatModifierGroup_Record", ".awarenessCost"},
        {"Float", "None", ".memoryCostReductionInOverclock"},
    }},
    {"gamedataHitPrereq_Record", {
        {"String", "None", ".hitFlag"},
        {"Bool", "None", ".processMiss"},
    }},
    {"gamedataSensePreset_Record", {
        {"TweakDBID", "gamedataSenseObjectType_Record", ".objectType"},
    }},
    {"gamedataIPrereq_Record", {
        {"Bool", "None", ".waitForVisuals"},
        {"Bool", "None", ".hasTag"},
        {"Float", "None", ".timeFrame"},
        {"Bool", "None", ".checkGain"},
        {"Float", "None", ".horizontalTolerance"},
        {"Float", "None", ".minTime"},
        {"Float", "None", ".timeout"},
        {"Bool", "None", ".isBreached"},
        {"Int32", "None", ".tier"},
        {"String", "None", ".howManyCharges"},
        {"Float", "None", ".duration"},
        {"String", "None", ".pipelineStage"},
        {"String", "None", ".recipe"},
        {"Float", "None", ".minStateTime"},
        {"String", "None", ".typeOfItem"},
        {"Bool", "None", ".isInState"},
        {"TweakDBID", "gamedataAttachmentSlot_Record", ".attachmentSlot"},
        {"String", "None", ".callbackType"},
        {"CName", "None", ".visionModeType"},
        {"Bool", "None", ".repeated"},
        {"Int32", "None", ".level"},
        {"TweakDBID", "gamedataCheckType_Record", ".checkType"},
        {"String", "None", ".reactionPreset"},
        {"Float", "None", ".valueToCheck"},
        {"Int32", "None", ".howManyShots"},
        {"Bool", "None", ".isSynchronous"},
        {"TweakDBID", "gamedataStatusEffect_Record", ".statusEffect"},
        {"CName", "None", ".tagToCheck"},
        {"CName", "None", ".consumableItemTag"},
        {"Bool", "None", ".canCheckProxy"},
        {"Float", "None", ".range"},
        {"String", "None", ".perkType"},
        {"Float", "None", ".elevationThreshold"},
        {"Float", "None", ".delay"},
        {"Bool", "None", ".isPlayerNoticed"},
        {"TweakDBID", "gamedataItemCategory_Record", ".itemCategory"},
        {"String", "None", ".ammoState"},
        {"TweakDBID", "gamedataWeaponEvolution_Record", ".weaponEvolution"},
        {"Bool", "None", ".checkForRMA"},
        {"Bool", "None", ".comparePercentage"},
        {"String", "None", ".quickhack"},
        {"Float", "None", ".randRange"},
        {"Bool", "None", ".ignoreSelfInflictedPressureWave"},
        {"String", "None", ".rarity"},
        {"CName", "None", ".slotname"},
        {"CName", "None", ".comparisonType"},
        {"Bool", "None", ".previousState"},
        {"Float", "None", ".maxTime"},
        {"Float", "None", ".verticalTolerance"},
        {"Float", "None", ".setChance"},
        {"String", "None", ".hitFlag"},
        {"String", "None", ".stateName"},
        {"Float", "None", ".percentage"},
        {"Bool", "None", ".isMoving"},
        {"String", "None", ".characterRecord"},
        {"String", "None", ".attackType"},
        {"CName", "None", ".statPoolType"},
        {"Int32", "None", ".minimumQuality"},
        {"String", "None", ".visualTag"},
        {"CName", "None", ".qualityLessThan"},
        {"Bool", "None", ".invert"},
        {"String", "None", ".failureExplanation"},
        {"String", "None", ".hitReactionType"},
        {"Bool", "None", ".isCheckInverted"},
        {"Bool", "None", ".intert"},
        {"TweakDBID", "gamedataAIActionTarget_Record", ".target"},
        {"Int32", "None", ".value"},
        {"Float", "None", ".ratioToCompare"},
        {"String", "None", ".attitude"},
        {"Float", "None", ".detectionThreshold"},
        {"Int32", "None", ".count"},
        {"String", "None", ".attackSubtype"},
        {"CName", "None", ".itemTag"},
        {"String", "None", ".hitSource"},
        {"Bool", "None", ".inverted"},
        {"TweakDBID", "gamedataItemType_Record", ".itemType"},
        {"array:CName", "None", ".equipAreas"},
        {"String", "None", ".pipelineType"},
        {"String", "None", ".locomotionType"},
        {"Int32", "None", ".minAmountOfAmmoReloaded"},
        {"String", "None", ".object"},
        {"CName", "None", ".fact"},
        {"Bool", "None", ".isChoiceHubActive"},
    }},
    {"gamedataAttack_Melee_Record", {
        {"Float", "None", ".adjustmentDistanceRadiusOffset"},
        {"Float", "None", ".addedImpulseDelay"},
        {"Bool", "None", ".enableMoveAssistOnLightAimAssist"},
        {"Float", "None", ".cooldown"},
        {"Float", "None", ".attackRange"},
        {"Float", "None", ".adjustmentRange"},
        {"Bool", "None", ".useAttackSlot"},
        {"Float", "None", ".attackGameEffectDelay"},
        {"Float", "None", ".adjustmentRadius"},
        {"CName", "None", ".adjustmentCurve"},
        {"Float", "None", ".adjustmentDuration"},
        {"Float", "None", ".addedImpulse"},
        {"Float", "None", ".attackGameEffectDuration"},
        {"Bool", "None", ".forcePlayerToStand"},
        {"Bool", "None", ".shouldAdjust"},
        {"Float", "None", ".hitCooldown"},
        {"Bool", "None", ".disableAdjustingPlayerPositionToTarget"},
        {"Float", "None", ".minimumDistanceToTargetToAddImpulse"},
    }},
    {"gamedataGameplayLogicPackage_Record", {
        {"Float", "None", ".memoryCostReductionPerStack"},
        {"TweakDBID", "gamedataStatModifierGroup_Record", ".duration"},
    }},
};
