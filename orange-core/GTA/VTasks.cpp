#include "stdafx.h"

VTasks *VTasks::singleInstance = nullptr;

std::string VTasks::GetTaskName(int64_t id)
{
	if (id < 0 || id >= 500)
		return std::string("Oversize");
	if (TasksArray[id] == nullptr)
	{
		std::stringstream ss;
		ss << "Unknown[" << id << "]";
		return ss.str();
	}
	else
	{
		return (*TasksArray[id]).substr(5);
	}
}

VTasks::VTasks()
{
	//CTaskReactAimWeapon
	TasksArray[424] = new std::string("CTaskReactAimWeapon");
	TasksArray[414] = new std::string("CTaskNMSit");
	TasksArray[170] = new std::string("CTaskMotionInAutomobile");
	TasksArray[173] = new std::string("CTaskMotionInVehicle");
	TasksArray[165] = new std::string("CTaskInVehicleSeatShuffle");
	TasksArray[279] = new std::string("CTaskQuadLocomotion");
	TasksArray[270] = new std::string("CTaskMotionStrafing");
	TasksArray[285] = new std::string("CTaskMotionAimingTransition");
	TasksArray[394] = new std::string("CTaskNMShot");
	TasksArray[288] = new std::string("CTaskMotionInCover");
	TasksArray[351] = new std::string("CTaskThreatResponse");
	TasksArray[238] = new std::string("CTaskMoveFollowNavMesh");
	TasksArray[407] = new std::string("CTaskNMControl");
	TasksArray[399] = new std::string("CTaskNMExplosion");
	TasksArray[395] = new std::string("CTaskNMHighFall");
	TasksArray[417] = new std::string("CTaskRageRagdoll");
	TasksArray[407] = new std::string("CTaskNMControl");
	TasksArray[272] = new std::string("CTaskMotionAiming");
	TasksArray[9] = new std::string("CTaskPlayerWeapon");
	TasksArray[56] = new std::string("CTaskSwapWeapon");
	TasksArray[130] = new std::string("CTaskMeleeActionResult");
	TasksArray[2] = new std::string("CTaskExitVehicle");
	TasksArray[200] = new std::string("CTaskVehicleGun");
	TasksArray[150] = new std::string("CTaskInVehicleBasic");
	TasksArray[10] = new std::string("CTaskPlayerIdles");
	TasksArray[421] = new std::string("CTaskJump");
	TasksArray[420] = new std::string("CTaskJumpVault");
	TasksArray[281] = new std::string("CTaskMotionSwimming");
	TasksArray[76] = new std::string("CTaskWitness");
	TasksArray[527] = new std::string("CTaskVehicleTransformToSubmarine");
	TasksArray[50] = new std::string("CTaskVault");
	TasksArray[8] = new std::string("CTaskWeapon");
	TasksArray[218] = new std::string("CTasksArraymartFlee");
	TasksArray[198] = new std::string("CTasksArrayetPedOutOfVehicle");
	TasksArray[197] = new std::string("CTasksArrayetPedInVehicle");
	TasksArray[298] = new std::string("CTaskReloadGun");
	TasksArray[6] = new std::string("CTaskPlayerOnFoot");
	TasksArray[159] = new std::string("CTaskPlayerDrive");
	TasksArray[334] = new std::string("CTaskParachute");
	TasksArray[162] = new std::string("CTaskOpenVehicleDoorFromOutside");
	TasksArray[54] = new std::string("CTaskOpenDoor");
	TasksArray[367] = new std::string("CTaskMoveWithinDefensiveArea");
	TasksArray[366] = new std::string("CTaskMoveWithinAttackWindow");
	TasksArray[34] = new std::string("CTaskMoveStandStill");
	TasksArray[252] = new std::string("CTaskMoveSlideToCoord");
	TasksArray[5] = new std::string("CTaskMovePlayer");
	TasksArray[129] = new std::string("CTaskMoveMeleeMovement");
	TasksArray[39] = new std::string("CTaskMoveInAir");
	TasksArray[196] = new std::string("CTaskMoveGoToVehicleDoor");
	TasksArray[205] = new std::string("CTaskMoveGoToPoint");
	TasksArray[206] = new std::string("CTaskMoveAchieveHeading");
	TasksArray[128] = new std::string("CTaskMelee");
	TasksArray[152] = new std::string("CTaskLeaveAnyCar");
	TasksArray[309] = new std::string("CTaskInCover");
	TasksArray[268] = new std::string("CTaskHumanLocomotion");
	TasksArray[0] = new std::string("CTaskHandsUp");
	TasksArray[290] = new std::string("CTaskGun");
	TasksArray[224] = new std::string("CTaskGoToPointAnyMeans");
	TasksArray[230] = new std::string("CTaskGoToPointAiming");
	TasksArray[195] = new std::string("CTaskGoToCarDoorAndStandStill");
	TasksArray[47] = new std::string("CTaskGoToAndClimbLadder");
	TasksArray[17] = new std::string("CTaskGetUpAndStandStill");
	TasksArray[16] = new std::string("CTaskGetUp");
	TasksArray[256] = new std::string("CTaskGetOutOfWater");
	TasksArray[178] = new std::string("CTaskGetOnTrain");
	TasksArray[179] = new std::string("CTaskGetOffTrain");
	TasksArray[57] = new std::string("CTaskGeneralSweep");
	TasksArray[45] = new std::string("CTaskForceMotionState");
	TasksArray[261] = new std::string("CTaskFollowWaypointRecording");
	TasksArray[223] = new std::string("CTaskFollowLeaderInFormation");
	TasksArray[226] = new std::string("CTaskFollowLeaderAnyMeans");
	TasksArray[229] = new std::string("CTaskFlyingWander");
	TasksArray[228] = new std::string("CTaskFlyToPoint");
	TasksArray[219] = new std::string("CTaskFlyAway");
	TasksArray[274] = new std::string("CTaskFlightlessBirdLocomotion");
	TasksArray[278] = new std::string("CTaskFishLocomotion");
	TasksArray[65] = new std::string("CTaskFirePatrol");
	TasksArray[18] = new std::string("CTaskFallOver");
	TasksArray[19] = new std::string("CTaskFallAndGetUp");
	TasksArray[422] = new std::string("CTaskFall");
	TasksArray[12] = new std::string("CTaskFSMClone");
	TasksArray[167] = new std::string("CTaskExitVehicleSeat");
	TasksArray[44] = new std::string("CTaskExitVehicle");
	TasksArray[301] = new std::string("CTaskExitCover");
	TasksArray[215] = new std::string("CTaskExhaustedFlee");
	TasksArray[240] = new std::string("CTaskEscapeBlast");
	TasksArray[163] = new std::string("CTaskEnterVehicleSeat");
	TasksArray[161] = new std::string("CTaskEnterVehicleAlign");
	TasksArray[160] = new std::string("CTaskEnterVehicle");
	TasksArray[300] = new std::string("CTaskEnterCover");
	TasksArray[97] = new std::string("CTaskDyingDead");
	TasksArray[146] = new std::string("CTaskDuckAndCover");
	TasksArray[51] = new std::string("CTaskDropDown");
	TasksArray[363] = new std::string("CTaskDraggingToSafety");
	TasksArray[364] = new std::string("CTaskDraggedToSafety");
	TasksArray[15] = new std::string("CTaskDoNothing");
	TasksArray[443] = new std::string("CTaskDiveToGround");
	TasksArray[432] = new std::string("CTaskDetonator");
	TasksArray[107] = new std::string("CTaskDeadBodyScenario");
	TasksArray[26] = new std::string("CTaskDamageElectric");
	TasksArray[440] = new std::string("CTaskCutScene");
	TasksArray[327] = new std::string("CTaskCrouchToggle");
	TasksArray[127] = new std::string("CTaskCrouch");
	TasksArray[20] = new std::string("CTaskCrawl");
	TasksArray[106] = new std::string("CTaskCowerScenario");
	TasksArray[126] = new std::string("CTaskCower");
	TasksArray[287] = new std::string("CTaskCover");
	TasksArray[117] = new std::string("CTaskCoupleScenario");
	TasksArray[169] = new std::string("CTaskControlVehicle");
	TasksArray[322] = new std::string("CTaskConfront");
	TasksArray[140] = new std::string("CTaskComplexStuckInAir");
	TasksArray[25] = new std::string("CTaskComplexOnFire");
	TasksArray[208] = new std::string("CTaskComplexGoToPointAndStandStillTimed");
	TasksArray[153] = new std::string("CTaskComplexGetOffBoat");
	TasksArray[137] = new std::string("CTaskComplexEvasiveStep");
	TasksArray[35] = new std::string("CTaskComplexControlMovement");
	TasksArray[12] = new std::string("CTaskComplex");
	TasksArray[339] = new std::string("CTaskCombatSeekCover");
	TasksArray[3] = new std::string("CTaskCombatRoll");
	TasksArray[343] = new std::string("CTaskCombatMounted");
	TasksArray[341] = new std::string("CTaskCombatFlank");
	TasksArray[307] = new std::string("CTaskCombatClosestTargetInArea");
	TasksArray[308] = new std::string("CTaskCombatAdditionalTask");
	TasksArray[342] = new std::string("CTaskCombat");
	TasksArray[168] = new std::string("CTaskCloseVehicleDoorFromOutside");
	TasksArray[164] = new std::string("CTaskCloseVehicleDoorFromInside");
	TasksArray[48] = new std::string("CTaskClimbLadderFully");
	TasksArray[1] = new std::string("CTaskClimbLadder");
	TasksArray[29] = new std::string("CTaskClearLookAt");
	TasksArray[104] = new std::string("CTaskChatScenario");
	TasksArray[425] = new std::string("CTaskChat");
	TasksArray[384] = new std::string("CTaskCharge");
	TasksArray[155] = new std::string("CTaskCarSetTempAction");
	TasksArray[95] = new std::string("CTaskCarReactToVehicleCollisionGetOut");
	TasksArray[93] = new std::string("CTaskCarReactToVehicleCollision");
	TasksArray[151] = new std::string("CTaskCarDriveWander");
	TasksArray[157] = new std::string("CTaskCarDrive");
	TasksArray[446] = new std::string("CTaskCallPolice");
	TasksArray[64] = new std::string("CTaskBusted");
	TasksArray[156] = new std::string("CTaskBringVehicleToHalt");
	TasksArray[431] = new std::string("CTaskBomb");
	TasksArray[372] = new std::string("CTaskBoatStrafe");
	TasksArray[371] = new std::string("CTaskBoatCombat");
	TasksArray[370] = new std::string("CTaskBoatChase");
	TasksArray[406] = new std::string("CTaskBlendFromNM");
	TasksArray[273] = new std::string("CTaskBirdLocomotion");
	TasksArray[62] = new std::string("CTaskArrestPed");
	TasksArray[63] = new std::string("CTaskArrestPed2");
	TasksArray[78] = new std::string("CTaskArmy");
	TasksArray[387] = new std::string("CTaskAnimatedHitByExplosion");
	TasksArray[528] = new std::string("CTaskAnimatedFallback");
	TasksArray[434] = new std::string("CTaskAnimatedAttach");
	TasksArray[68] = new std::string("CTaskAmbulancePatrol");
	TasksArray[53] = new std::string("CTaskAmbientLookAtEvent");
	TasksArray[38] = new std::string("CTaskAmbientClips");
	TasksArray[313] = new std::string("CTaskAimSweep");
	TasksArray[295] = new std::string("CTaskAimGunVehicleDriveBy");
	TasksArray[296] = new std::string("CTaskAimGunScripted");
	TasksArray[4] = new std::string("CTaskAimGunOnFoot");
	TasksArray[303] = new std::string("CTaskAimGunFromCoverOutro");
	TasksArray[302] = new std::string("CTaskAimGunFromCoverIntro");
	TasksArray[304] = new std::string("CTaskAimGunBlindFire");
	TasksArray[12] = new std::string("CTaskAimGun");
	TasksArray[291] = new std::string("CTaskAimFromGround");
	TasksArray[289] = new std::string("CTaskAimAndThrowProjectile");
	TasksArray[321] = new std::string("CTaskAgitatedAction");
	TasksArray[320] = new std::string("CTaskAgitated");
	TasksArray[147] = new std::string("CTaskAggressiveRubberneck");
	TasksArray[52] = new std::string("CTaskAffectSecondaryBehaviour");
	TasksArray[383] = new std::string("CTaskAdvance");
	TasksArray[264] = new std::string("CTaskMotionPed");
	TasksArray[199] = new std::string("CTaskVehicleMountedWeapon");
	TasksArray[498] = new std::string("CTaskVehiclePlayerDrivePlane");
	TasksArray[493] = new std::string("CTaskVehiclePlayerDriveAutomobile");
	TasksArray[501] = new std::string("CTaskVehiclePlayerDriveDiggerArm");
	TasksArray[502] = new std::string("CTaskVehiclePlayerDriveTrain");
	TasksArray[494] = new std::string("CTaskVehiclePlayerDriveBike");
	TasksArray[495] = new std::string("CTaskVehiclePlayerDriveBoat");
	TasksArray[496] = new std::string("CTaskVehiclePlayerDriveSubmarine");
	TasksArray[497] = new std::string("CTaskVehiclePlayerDriveSubmarineCar");
	TasksArray[498] = new std::string("CTaskVehiclePlayerDriveRotaryWingAircraft");
	TasksArray[499] = new std::string("CTaskVehiclePlayerDriveHeli");
	TasksArray[500] = new std::string("CTaskVehiclePlayerDriveAutogyro");
	TasksArray[209] = new std::string("CTaskMoveFollowPointRoute");
	TasksArray[239] = new std::string("CTaskMoveGoToPointOnRoute");
	TasksArray[480] = new std::string("CTaskVehicleCrash");
}


VTasks::~VTasks()
{
}

void *VTasks::GetTaskFromID(unsigned int taskID) {
	typedef void*(*__func)(unsigned int);
	((__func)(CMemory((uintptr_t)GetModuleHandle(NULL) + 0x658904)()))(taskID);
	return nullptr;
}