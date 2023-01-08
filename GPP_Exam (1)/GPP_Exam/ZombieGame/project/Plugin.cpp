#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "Behaviors.h"

using namespace std;
using namespace Elite;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "zombie game";
	info.Student_FirstName = "Xander";
	info.Student_LastName = "Bartels";
	info.Student_Class = "2DAE07";

	//Build behavior tree
	m_pBlackboard = new Blackboard();
	m_pBlackboard->AddData("Interface", m_pInterface);
	m_pBehaviorTree = new BehaviorTree
	(m_pBlackboard,
		new BehaviorSelector
		(
			{
				new BehaviorSequence(
					{
						new BehaviorConditional(BT_Condition::IsPurgeZoneInFOV),
						new BehaviorAction(BT_Action::EvadePurgeZone)
					}
				),
			
				new BehaviorSequence
				(
					{

						 new BehaviorAction(BT_Action::Wander),

						 //Eat if there is food and Energy is low
						  new BehaviorSelector
						  (
							  {
								  new BehaviorInvertConditional(BT_Condition::CheckEnergy),
								  new BehaviorAction(BT_Action::Eat)
							  }
						  ),

						 //Use Medkit if available and Health is low
						 new BehaviorSelector
						 (
							 {
								 new BehaviorInvertConditional(BT_Condition::CheckHealth),
								 new BehaviorAction(BT_Action::UseMedkit)
							 }
						 ),

						 //If Gun is available, check for enemy and shoot him
						 new BehaviorMaskFailure(new BehaviorSequence
						 (
							 {
								new BehaviorConditional(BT_Condition::CheckForGunInInventory),
								new BehaviorAction(BT_Action::Turn),
								new BehaviorConditional([](Blackboard* b) {return BT_Condition::IsSlowEnemyInFOV(b) || BT_Condition::IsFastEnemyInFOV(b); }),
								new BehaviorAction(BT_Action::FaceEnemy),
								new BehaviorSelector
							 	(
									 {
									 	new BehaviorAction(BT_Action::ShootEnemy),
										new BehaviorAction(BT_Action::RunAway)
									 }
								)
							 }
						 )),

						 //Check if there is an house in FOV, if so, go inside and collect loot
						 new BehaviorSelector
						 (
							 {
								 new BehaviorInvertConditional(BT_Condition::IsHouseInFOV),
								 new BehaviorAction(BT_Action::MoveToHouse)
							 }
						 ),

						 new BehaviorSelector
						 (
							 {
								 new BehaviorInvertConditional(BT_Condition::IsAgentInHouse),
								 new BehaviorAction(BT_Action::LookForLoot)
							 }
						 ),


						 //If loot in FOV, try to grab it, if too far, set TargetPos to move to loot
						 new BehaviorSelector
						 (
							 {
								 new BehaviorInvertConditional(BT_Condition::IsLootInFOV),
								 new BehaviorAction(BT_Action::TryGrabLoot),
								 new BehaviorAction(BT_Action::MoveToLoot)
							 }
						 )
					 }
				 )
			}
		)	
	);

}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called when the plugin gets unloaded
	SAFE_DELETE(m_pBehaviorTree)
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	//only in debug  mode, examen is in release 
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = false; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = true;
	params.SpawnDebugShotgun = true;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = 34;
	params.SpawnZombieOnRightClick = true;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)

//only in debug  mode, examen is in release 

void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(eLeft))
	{
		//Update target based on input
		MouseData mouseData = m_pInterface->Input_GetMouseData(eMouseButton, eLeft);
		const Vector2 pos = Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_Left))
	{
		m_AngSpeed -= ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_Right))
	{
		m_AngSpeed += ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(eScancode_Space))
	{
		m_CanRun = false;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_Delete))
	{
		m_pInterface->RequestShutdown();
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_KP_Minus))
	{
		if (m_InventorySlot > 0)
			--m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_KP_Plus))
	{
		if (m_InventorySlot < 4)
			++m_InventorySlot;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_Q))
	{
		ItemInfo info = {};
		m_pInterface->Inventory_GetItem(m_InventorySlot, info);
		std::cout << (int)info.Type << std::endl;
	}

}

SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	auto steering = SteeringPlugin_Output();
	Vector2 nextTargetPos{};

	m_pBlackboard->SetData(BB::DeltaT, dt);
	m_pBlackboard->SetData(BB::AngularVelocity, 0.f);
	m_pBlackboard->SetData(BB::TargetPositions, &m_TargetPositions);
	m_pBlackboard->SetData(BB::CanRun, false);

	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();

	m_pBehaviorTree->Update(dt);

	// Erase all target points near to agent
	for (auto it = m_TargetPositions.begin(); it != m_TargetPositions.end();)
	{
		if ((* it).Distance(agentInfo.Position) <= agentInfo.GrabRange * .8f && m_TargetPositions.size() > 1)
			it = m_TargetPositions.erase(it);
		else
			++it;
	}

	//Erase targetPositions in walls
	for (HouseInfo hi : GetHousesInFOV())
	{
		if (m_TargetPositions.size()>0)
		{
			auto p = m_TargetPositions[0];
			auto dx = hi.Size.x*.5f - std::abs(p.x - hi.Center.x);
			auto dy = hi.Size.y*.5f - std::abs(p.y - hi.Center.y);
			if (dx >= -1 && dx < 3 || dy>=-1 && dy<3)
			{
				m_TargetPositions.erase(m_TargetPositions.begin());
			}
		}
	}

	//Set steering data
	nextTargetPos = m_TargetPositions[0];
	nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(nextTargetPos);
	steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	m_pBlackboard->GetData(BB::AngularVelocity, steering.AngularVelocity);
	steering.AutoOrient = steering.AngularVelocity == 0.f;

	//std::cout << "Target Point: " << nextTargetPos.x << ", " << nextTargetPos.y << ", " << m_TargetPositions.size() << " positions in queue, agent location: " << agentInfo.Position.x << ", " << agentInfo.Position.y << "\n";
	std::cout << "AngularVelocity : " << steering.AngularVelocity << ", AutoOrient : " << steering.AutoOrient << "\n";

	if (Distance(nextTargetPos, agentInfo.Position) < 0.1f)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}

	if (m_pBlackboard->GetData(BB::CanRun, m_CanRun))
	{
		steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)
	}

	//@End (Demo Purposes)
	return steering;
}

//Update

//This function calculates the new SteeringOutput, called once per frame

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	for (auto p : m_TargetPositions)
	{
		m_pInterface->Draw_SolidCircle(p, .7f, { 0,0 }, { 0,1,1 });
	}
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}
