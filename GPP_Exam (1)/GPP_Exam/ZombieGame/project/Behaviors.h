#pragma once

#include <Exam_HelperStructs.h>
#include <IExamInterface.h>
#include "EBehaviorTree.h"

using namespace Elite;

namespace BT_Utils
{
	int GetInventoryItem(IExamInterface* pInterface, eItemType type)
	{
		for (UINT i = 0; i < pInterface->Inventory_GetCapacity(); ++i)
		{
			ItemInfo ii;
			if (pInterface->Inventory_GetItem(i, ii) && ii.Type == type)
			{
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	bool IsInRect(Vector2 center, Vector2 size, Vector2 pos)
	{
		return std::abs(pos.x - center.x) <= size.x / 2.f
			&& std::abs(pos.y - center.y) <= size.y / 2.f;
	}

	void AddTarget(std::deque<Vector2>* pTargetPoints, Vector2 newTarget)
	{
		if (pTargetPoints->empty() || std::find(pTargetPoints->begin(), pTargetPoints->end(), newTarget) == pTargetPoints->end())
		{
			pTargetPoints->push_front(newTarget);
		}
	}

}
namespace BT_Condition
{
	bool IsEnemyInFOV(Blackboard* pBlackboard, eEnemyType type)
	{
		IExamInterface* pInterface;

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		EntityInfo ei = {};
		for (int i = 0;; ++i)
		{
			if (pInterface->Fov_GetEntityByIndex(i, ei))
			{
				if (ei.Type == eEntityType::ENEMY)
				{
					EnemyInfo enemyInfo;
					pInterface->Enemy_GetInfo(ei, enemyInfo);
					if (enemyInfo.Type == type)
					{
						pInterface->Enemy_GetInfo(ei, enemyInfo);

						pBlackboard->SetData("Enemy", enemyInfo);
						return true;
					}
				}
			}
			else
			{
				break;
			}
		}
		return false;
	}

	bool IsFastEnemyInFOV(Blackboard* pBlackboard)
	{
		//std::cout << "zombie fast\n";

		return IsEnemyInFOV(pBlackboard, eEnemyType::ZOMBIE_RUNNER);
	}

	bool IsSlowEnemyInFOV(Blackboard* pBlackboard)
	{
		//std::cout << "zombie slow\n";

		return IsEnemyInFOV(pBlackboard, eEnemyType::ZOMBIE_NORMAL) || IsEnemyInFOV(pBlackboard, eEnemyType::ZOMBIE_HEAVY);
	}

	bool CheckEnergy(Blackboard* pBlackboard)
	{
		//std::cout << "CheckEnergy\n";

		IExamInterface* pInterface;

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		AgentInfo agentInfo{ pInterface->Agent_GetInfo() };

		if (agentInfo.Energy < 3.0f)
		{
			int itemIdx{BT_Utils::GetInventoryItem(pInterface, eItemType::FOOD) };
			if (itemIdx>=0)
			{
				return true;
			}
		}

		return false;
	}

	bool CheckHealth(Blackboard* pBlackboard)
	{
		//std::cout << "CheckHealth\n";
		IExamInterface* pInterface;

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		AgentInfo agentInfo{ pInterface->Agent_GetInfo() };

		if (agentInfo.Health < 8.0f)
		{
			int itemIdx{BT_Utils::GetInventoryItem(pInterface, eItemType::MEDKIT) };
			if (itemIdx>=0)
			{
				return true;
			}
		}

		return false;
	}

	bool IsHouseInFOV(Blackboard* pBlackboard)
	{
		//std::cout << "IsHouseInFOV\n";
		IExamInterface* pInterface;

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}


		std::vector<Vector2> visitedHouses;
		if (!pBlackboard->GetData("VisitedHouses", visitedHouses))
		{
			visitedHouses = std::vector<Vector2>();
		}

		HouseInfo hi = {};
		for (int i = 0;; ++i)
		{
			if (pInterface->Fov_GetHouseByIndex(i, hi))
			{
				if (std::find(visitedHouses.begin(), visitedHouses.end(), hi.Center) == visitedHouses.end())
				{
					pBlackboard->SetData("HouseToVisit", hi);
					return true;
				}
			}
			else
			{
				break;
			}
		}
		return false;
	}

	bool IsAgentInHouse(Blackboard* pBlackboard)
	{
		//std::cout << "IsAgentInHouse\n";
		IExamInterface* pInterface;

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		AgentInfo ai = pInterface->Agent_GetInfo();

		HouseInfo hi;
		if (!pBlackboard->GetData("HouseToVisit", hi))
		{
			return false;
		}
		return BT_Utils::IsInRect(hi.Center, hi.Size, ai.Position);
	}

	bool IsLootInFOV(Blackboard* pBlackboard)
	{
		//std::cout << "IsLootInFOV\n";

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		EntityInfo ei = {};
		for (int i = 0;; ++i)
		{
			if (pInterface->Fov_GetEntityByIndex(i, ei))
			{
				if (ei.Type == eEntityType::ITEM)
				{
					pBlackboard->SetData("ItemToPick", ei);
					return true;
				}
			}
			else
			{
				break;
			}
		}
		return false;
	}

	bool CheckForGunInInventory(Blackboard* pBlackboard)
	{
		//std::cout << "CheckForGunInInventory\n";
		IExamInterface* pInterface;

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		AgentInfo agentInfo{ pInterface->Agent_GetInfo() };


			int itemIdxPistol{ BT_Utils::GetInventoryItem(pInterface, eItemType::PISTOL) };
			int itemIdxShotgun{ BT_Utils::GetInventoryItem(pInterface, eItemType::SHOTGUN) };
			if (itemIdxPistol >= 0 || itemIdxShotgun >= 0)
			{
				return true;
			}
		

		return false;
	}


}

namespace BT_Action
{

	int CountInventoryItems(IExamInterface* pInterface,  eItemType type)
	{
		int count {};
		for (UINT i=0; i < pInterface->Inventory_GetCapacity(); ++i)
		{
			ItemInfo ii;
			if (pInterface->Inventory_GetItem(i, ii) && ii.Type == type)
			{
				++count;
			}
		}
		return count;
	}

	BehaviorState MoveToHouse(Blackboard* pBlackboard)
	{
		//std::cout << "MoveToHouse\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData("HouseToVisit", hi))
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		if (!pBlackboard->GetData("TargetPositions", pTargetPositions))
		{
			return BehaviorState::Failure;
		}

		BT_Utils::AddTarget(pTargetPositions, hi.Center);
		return BehaviorState::Success;
	}

	BehaviorState TryGrabLoot(Blackboard* pBlackboard)
	{
		//std::cout << "TryGrabLoot\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		EntityInfo ei;
		if (!pBlackboard->GetData("ItemToPick", ei))
		{
			return BehaviorState::Failure;
		}

		int inventorySlot = 0;
		pBlackboard->GetData("InventorySlot", inventorySlot);

		auto agentInfo = pInterface->Agent_GetInfo();

		if (agentInfo.Position.Distance(ei.Location) <= agentInfo.GrabRange)
		{
			ItemInfo ii;
			if (pInterface->Item_GetInfo(ei, ii))
			{
				if (ii.Type == eItemType::GARBAGE)
				{
					pInterface->Item_Destroy(ei);
				}
				else
				{
					int count = CountInventoryItems(pInterface, ii.Type);

					if (count < 1 || (count < 2 && ii.Type == eItemType::FOOD))
					{
						pInterface->Item_Grab(ei, ii);

						if (inventorySlot < pInterface->Inventory_GetCapacity())
						{
							pInterface->Inventory_AddItem(inventorySlot, ii);
							++inventorySlot;
							pBlackboard->SetData("InventorySlot", inventorySlot);
						}

					}
					else
					{
						pInterface->Item_Destroy(ei);
					}
				}

			}
			return BehaviorState::Success;
		}
		return BehaviorState::Failure;
	}

	BehaviorState MoveToLoot(Blackboard* pBlackboard)
	{
		//std::cout << "MoveToLoot\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		EntityInfo ei;
		if (!pBlackboard->GetData("ItemToPick", ei))
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		if (!pBlackboard->GetData("TargetPositions", pTargetPositions))
		{
			return BehaviorState::Failure;
		}

		BT_Utils::AddTarget(pTargetPositions, ei.Location);

		pBlackboard->SetData("AngularVelocity", 0.f);

		return BehaviorState::Success;
	}

	//BehaviorState Walk(Blackboard* pBlackboard)
	//{
	//	//std::cout << "Walk\n";

	//	IExamInterface* pInterface;
	//	if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
	//	{
	//		return BehaviorState::Failure;
	//	}

	//	Vector2 checkPointLocation;
	//	if (!pBlackboard->GetData("TargetPos", checkPointLocation))
	//	{
	//		return BehaviorState::Failure;
	//	}

	//	SteeringPlugin_Output* pSteering;
	//	if (!pBlackboard->GetData("Steering", pSteering))
	//	{
	//		return BehaviorState::Failure;
	//	}

	//	auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(checkPointLocation);

	//	auto agentInfo = pInterface->Agent_GetInfo();

	//	//BT_utils::SetTargetPos(pSteering, nextTargetPos, agentInfo);
	//	pSteering->AutoOrient = true;

	//	float distance = agentInfo.Position.Distance(checkPointLocation);
	//	if (distance < 3.f)
	//	{
	//		return BehaviorState::Success;
	//	}

	//	return BehaviorState::Running;
	//}

	BehaviorState HandleLoot(Blackboard* pBlackboard)
	{
		//std::cout << "TakeLoot\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		EntityInfo ei;
		if (!pBlackboard->GetData("ItemToPick", ei))
		{
			return BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData("HouseToVisit", hi))
		{
			return BehaviorState::Failure;
		}

		int inventorySlot = 0;
		pBlackboard->GetData("InventorySlot", inventorySlot);

		ItemInfo ii;
		if (pInterface->Item_GetInfo(ei, ii))
		{
			//BT_utils::SetTargetPos(pSteering, hi.Center, pInterface->Agent_GetInfo());

			if (ii.Type == eItemType::GARBAGE)
			{
				pInterface->Item_Destroy(ei);
			}
			else
			{
				int count = CountInventoryItems(pInterface, ii.Type);

				if (count < 1 || (count < 2 && ii.Type == eItemType::FOOD))
				{
					pInterface->Item_Grab(ei, ii);

					if (inventorySlot < pInterface->Inventory_GetCapacity())
					{
						pInterface->Inventory_AddItem(inventorySlot, ii);
						++inventorySlot;
						pBlackboard->SetData("InventorySlot", inventorySlot);
					}

				}
				else
				{
					pInterface->Item_Destroy(ei);
				}
			}

			return BehaviorState::Success;
		}

		return BehaviorState::Failure;
	}

	BehaviorState LeaveHouse(Blackboard* pBlackboard)
	{

		//std::cout << "LeaveHouse\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData("HouseToVisit", hi))
		{
			return BehaviorState::Failure;
		}


		Vector2 checkPointLocation{ pInterface->World_GetInfo().Dimensions.x ,0.f };
		auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(checkPointLocation);

		auto agentInfo = pInterface->Agent_GetInfo();
		//BT_utils::SetTargetPos(pSteering, nextTargetPos, agentInfo);
		pSteering->AutoOrient = true;


		return BT_Utils::IsInRect(hi.Center, hi.Size, agentInfo.Position)
			       ? BehaviorState::Running
			       : BehaviorState::Success;
	}

	BehaviorState UseItem(Blackboard* pBlackboard, eItemType type, std::function<bool(IExamInterface* ,ItemInfo)> removeItemCheck)
	{
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		int itemIndex = BT_Utils::GetInventoryItem(pInterface, type);
		ItemInfo itemInfo;
		pInterface->Inventory_GetItem(itemIndex, itemInfo);

		if (itemIndex >= 0)
		{
			pInterface->Inventory_UseItem(itemIndex);

			if (removeItemCheck(pInterface, itemInfo))
			{
				pInterface->Inventory_RemoveItem(itemIndex);
			}

			return BehaviorState::Success;
		}
		return BehaviorState::Failure;
	}

	bool HasToRemoveFood(IExamInterface* pInterface, ItemInfo& itemInfo)
	{
		return pInterface->Food_GetEnergy(itemInfo) <= 0;
	}

	BehaviorState Eat(Blackboard* pBlackboard)
	{

		//std::cout << "Eat\n";
		
		return UseItem(pBlackboard, eItemType::FOOD, HasToRemoveFood);
	}

	BehaviorState LookForLoot(Blackboard* pBlackboard)
	{
		//std::cout << "LookForLoot\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData("HouseToVisit", hi))
		{
			return BehaviorState::Failure;
		}

		std::vector<Vector2> visitedHouses;
		if (!pBlackboard->GetData("VisitedHouses", visitedHouses))
		{
			visitedHouses = std::vector<Vector2>();
		}

		if (std::find(visitedHouses.begin(), visitedHouses.end(), hi.Center) != visitedHouses.end())
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		if (!pBlackboard->GetData("TargetPositions", pTargetPositions))
		pBlackboard->SetData("AngularVelocity", 1.f);
		auto agentInfo = pInterface->Agent_GetInfo();

		Vector2 spacing{ hi.Size.x > 22.f ? 10.f : (hi.Size.x / 2.f - 2.f),hi.Size.y > 22.f ? 10.f : (hi.Size.y / 2.f - 2.f) };
		Vector2 corner1{ hi.Center - hi.Size / 2.f + spacing};
		Vector2 corner3{ hi.Center + hi.Size / 2.f - spacing};
		Vector2 corner2{ corner1.x, corner3.y };
		Vector2 corner4{ corner3.x, corner1.y };

		BT_Utils::AddTarget(pTargetPositions, corner1);
		BT_Utils::AddTarget(pTargetPositions, corner2);
		BT_Utils::AddTarget(pTargetPositions, corner3);
		BT_Utils::AddTarget(pTargetPositions, corner4);

		//std::sort(pTargetPositions->begin(), pTargetPositions->end(), [agentInfo](Vector2 c1, Vector2 c2) {return c1.Distance(agentInfo.Position) < c2.Distance(agentInfo.Position); });
		pBlackboard->RemoveData("HouseToVisit");
		visitedHouses.push_back(hi.Center);
		pBlackboard->SetData("VisitedHouses", visitedHouses);

		return BehaviorState::Success;
	}

	BehaviorState UseMedkit(Blackboard* pBlackboard)
	{
		//std::cout << "UseMedkit\n";
		return UseItem(pBlackboard, eItemType::MEDKIT,
			[](IExamInterface* pInterface, ItemInfo& itemInfo) {return pInterface->Medkit_GetHealth(itemInfo) <= 0; });
	}

	BehaviorState FaceEnemy(Blackboard* pBlackboard)
	{
		//std::cout << "ShootEnemy\n";

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return BehaviorState::Failure;
		}

		EnemyInfo ei;
		if (!pBlackboard->GetData("Enemy", ei))
		{
			return BehaviorState::Failure;
		}

		auto agentInfo = pInterface->Agent_GetInfo();
		pSteering->AutoOrient = false;

		const Elite::Vector2 toTarget{ ei.Location - agentInfo.Position };
		const float angle{ 180 / float(M_PI) * agentInfo.Orientation };
		const float angleTarget{ 180 / float(M_PI) * atan2(toTarget.y, toTarget.x) };

		if (angle - angleTarget > 5) pSteering->AngularVelocity = -5;
		else if (angle - angleTarget < -5) pSteering->AngularVelocity = 5;
		else return BehaviorState::Success;
		
		return BehaviorState::Running;
	}

	BehaviorState SearchInHouse(Blackboard* pBlackboard)
	{
		return BehaviorState::Success;
	}


	BehaviorState Wander(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		if (!pBlackboard->GetData("TargetPositions", pTargetPositions))
		{
			return BehaviorState::Failure;
		}

		WorldInfo worldInfo = pInterface->World_GetInfo();
		BT_Utils::AddTarget(pTargetPositions, worldInfo.Center + worldInfo.Dimensions / 2);
		return BehaviorState::Success;
	}

	BehaviorState ShootEnemy(Blackboard* pBlackboard)
	{
		//std::cout << "ShootEnemy\n";

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return BehaviorState::Failure;
		}

		EnemyInfo ei;
		if (!pBlackboard->GetData("Enemy", ei))
		{
			return BehaviorState::Failure;
		}

		auto agentInfo = pInterface->Agent_GetInfo();
		pSteering->AutoOrient = false;

		auto result = UseItem(pBlackboard, eItemType::SHOTGUN, [](IExamInterface* pInterface, ItemInfo& itemInfo) {return pInterface->Weapon_GetAmmo(itemInfo) <= 0; });

		if (result == BehaviorState::Failure)
		{
			 result = UseItem(pBlackboard, eItemType::PISTOL, [](IExamInterface* pInterface, ItemInfo& itemInfo) {return pInterface->Weapon_GetAmmo(itemInfo) <= 0; });
		}

		return result;
	}
BehaviorState Turn(Blackboard* pBlackboard)
	{
		//std::cout << "turn\n";

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return BehaviorState::Failure;
		}

		pSteering->AutoOrient = false;
		pSteering->AngularVelocity = 1.f;

		
			
		return BehaviorState::Success;
		
	}

}

namespace BT_Steering
{

	//-----------------------------------------------------------------
	// STEERING BEHAVIORS (IBehavior)
	//-----------------------------------------------------------------
	class BehaviorSteering : public IBehavior
	{
	public:
		BehaviorSteering(ISteeringBehavior* pSteeringBehavior, std::string name)
		: m_pSteeringBehavior(pSteeringBehavior)
		, m_debugName(name)
		{}

		~BehaviorSteering() override
		{
			SAFE_DELETE(m_pSteeringBehavior);
		}

		virtual BehaviorState Execute(Blackboard* pBlackboard) override
		{
			//std::cout << m_debugName << "\n";

			IExamInterface* pInterface;
			if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
			{
				return BehaviorState::Failure;
			}

			SteeringPlugin_Output* pSteering;
			if (!pBlackboard->GetData("Steering", pSteering))
			{
				return BehaviorState::Failure;
			}

			float deltaT;
			if (!pBlackboard->GetData("DeltaT", deltaT))
			{
				return BehaviorState::Failure;
			}

			AgentInfo agentInfo = pInterface->Agent_GetInfo();
			auto steering = m_pSteeringBehavior->CalculateSteering(deltaT, &agentInfo);
			*pSteering = steering;

			//Elite::Vector2 worldCenter{ pInterface->World_GetInfo().Center };
			//Elite::Vector2 worldsize{ pInterface->World_GetInfo().Dimensions };
			//
			//Elite::Vector2 newDir = { -1,-1 };
			//
			//if (agentInfo.Position.x <= worldCenter.x - worldsize.x / 4 && agentInfo.Position.y <= worldCenter.y - worldsize.y / 4)
			//{
			//	newDir = Elite::Vector2{ worldCenter.x + worldsize.x / 2 - agentInfo.Position.x, worldCenter.y - worldsize.y / 2 - agentInfo.Position.y
			//};
			//}
			//
			//if(agentInfo.Position.x >= worldCenter.x + worldsize.x/4 && agentInfo.Position.y <= worldCenter.y - worldsize.y / 4)
			//{
			//	newDir = Elite::Vector2{ worldCenter.x - worldsize.x / 2 - agentInfo.Position.x, worldCenter.y + worldsize.y / 2 };
			//}
			//
			//if(agentInfo.Position.x <= worldCenter.x - worldsize.x/4 && agentInfo.Position.y >= worldCenter.y + worldsize.y / 4)
			//{
			//	newDir = Elite::Vector2{ worldCenter.x + worldsize.x / 2 - agentInfo.Position.x,  worldCenter.y + worldsize.y / 2 - agentInfo.Position.y };
			//}
			//
			//if(agentInfo.Position.x >= worldCenter.x + worldsize.x/4 && agentInfo.Position.y < worldCenter.y + worldsize.y / 4)
			//{
			//	newDir = Elite::Vector2{ worldCenter.x - worldsize.x / 2 - agentInfo.Position.x ,  worldCenter.y - worldsize.y / 2 - agentInfo.Position.y };
			//}
			//
			//pSteering->LinearVelocity = newDir ;
			//pSteering->LinearVelocity.Normalize();
			//pSteering->LinearVelocity *= agentInfo.MaxLinearSpeed;
			return BehaviorState::Success;
		}

	protected:
		ISteeringBehavior* m_pSteeringBehavior;
		std::string m_debugName;
	};

	class BehaviorWander : public BehaviorSteering
	{
	public:
		explicit BehaviorWander() : BehaviorSteering(new Wander(), "Wander")
		{  }
	};

}
