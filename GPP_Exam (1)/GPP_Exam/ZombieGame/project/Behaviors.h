#pragma once

#include <Exam_HelperStructs.h>
#include <IExamInterface.h>
#include "EBehaviorTree.h"

using namespace Elite;

namespace BB
{
	const std::string Interface = "Interface";
	const std::string Enemy = "Enemy";
	const std::string ItemToPick = "ItemToPick";
	const std::string VisitedHouses = "VisitedHouses";
	const std::string PurgeZone = "PurgeZone";
	const std::string HouseToVisit = "HouseToVisit";
	const std::string TargetPositions = "TargetPositions";
	const std::string AngularVelocity = "AngularVelocity";
	const std::string DeltaT = "DeltaT";
	const std::string CanRun = "CanRun";
}

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
	bool IsEntityInFOV(IExamInterface* pInterface, eEntityType entityType, EntityInfo& entityInfo)
	{
		EntityInfo ei = {};
		for (int i = 0;; ++i)
		{
			if (pInterface->Fov_GetEntityByIndex(i, ei))
			{
				if (ei.Type == entityType)
				{
					entityInfo = ei;
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

	bool IsEnemyInFOV(Blackboard* pBlackboard, eEnemyType type)
	{
		IExamInterface* pInterface;

		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return false;
		}

		EntityInfo ei = {};
		if (IsEntityInFOV(pInterface, eEntityType::ENEMY, ei))
		{
			EnemyInfo enemyInfo;
			pInterface->Enemy_GetInfo(ei, enemyInfo);
			if (enemyInfo.Type == type)
			{
				pInterface->Enemy_GetInfo(ei, enemyInfo);

				pBlackboard->SetData(BB::Enemy, enemyInfo);
				return true;
			}
		}
		return false;
	}

	bool IsFastEnemyInFOV(Blackboard* pBlackboard)
	{
		////std::cout << "zombie fast\n";

		return IsEnemyInFOV(pBlackboard, eEnemyType::ZOMBIE_RUNNER);
	}

	bool IsSlowEnemyInFOV(Blackboard* pBlackboard)
	{
		////std::cout << "zombie slow\n";

		return IsEnemyInFOV(pBlackboard, eEnemyType::ZOMBIE_NORMAL) || IsEnemyInFOV(pBlackboard, eEnemyType::ZOMBIE_HEAVY);
	}

	bool CheckEnergy(Blackboard* pBlackboard)
	{
		//std::cout << "CheckEnergy\n";

		IExamInterface* pInterface;

		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
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

		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
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

		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return false;
		}


		std::vector<Vector2> visitedHouses;
		if (!pBlackboard->GetData(BB::VisitedHouses, visitedHouses))
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
					pBlackboard->SetData(BB::HouseToVisit, hi);
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

		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return false;
		}

		AgentInfo ai = pInterface->Agent_GetInfo();

		HouseInfo hi;
		if (!pBlackboard->GetData(BB::HouseToVisit, hi))
		{
			return false;
		}
		return BT_Utils::IsInRect(hi.Center, hi.Size, ai.Position);
	}

	bool IsLootInFOV(Blackboard* pBlackboard)
	{
		//std::cout << "IsLootInFOV\n";

		IExamInterface* pInterface;
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return false;
		}

		EntityInfo ei = {};
		if (IsEntityInFOV(pInterface, eEntityType::ITEM, ei))
		{
			pBlackboard->SetData(BB::ItemToPick, ei);
			return true;
		}
		return false;
	}

	bool CheckForGunInInventory(Blackboard* pBlackboard)
	{
		//std::cout << "CheckForGunInInventory\n";
		IExamInterface* pInterface;

		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
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

	bool IsPurgeZoneInFOV(Blackboard* pBlackboard)
	{
		//std::cout << "IsPurgeZoneInFOV\n";
		IExamInterface* pInterface;

		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return false;
		}

		EntityInfo ei;
		if (IsEntityInFOV(pInterface, eEntityType::PURGEZONE, ei))
		{
			PurgeZoneInfo pi;
			if (pInterface->PurgeZone_GetInfo(ei, pi))
			{
				pBlackboard->SetData(BB::PurgeZone, pi);
				return true;
			}
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
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData(BB::HouseToVisit, hi))
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		if (!pBlackboard->GetData(BB::TargetPositions, pTargetPositions))
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
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		EntityInfo ei;
		if (!pBlackboard->GetData(BB::ItemToPick, ei))
		{
			return BehaviorState::Failure;
		}

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

						for (UINT idx=0; idx < pInterface->Inventory_GetCapacity(); ++idx)
						{
							ItemInfo dummy;
							if (pInterface->Inventory_GetItem(idx, dummy) == false)
							{
								//found empty slot
								pInterface->Inventory_AddItem(idx, ii);
								break;
							}
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
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		EntityInfo ei;
		if (!pBlackboard->GetData(BB::ItemToPick, ei))
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		if (!pBlackboard->GetData(BB::TargetPositions, pTargetPositions))
		{
			return BehaviorState::Failure;
		}

		BT_Utils::AddTarget(pTargetPositions, ei.Location);

		pBlackboard->SetData(BB::AngularVelocity, 0.f);

		return BehaviorState::Success;
	}

	BehaviorState UseItem(Blackboard* pBlackboard, eItemType type, std::function<bool(IExamInterface* ,ItemInfo)> removeItemCheck)
	{
		IExamInterface* pInterface;
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
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
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData(BB::HouseToVisit, hi))
		{
			return BehaviorState::Failure;
		}

		std::vector<Vector2> visitedHouses;
		if (!pBlackboard->GetData(BB::VisitedHouses, visitedHouses))
		{
			visitedHouses = std::vector<Vector2>();
		}

		if (std::find(visitedHouses.begin(), visitedHouses.end(), hi.Center) != visitedHouses.end())
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		pBlackboard->GetData(BB::TargetPositions, pTargetPositions);

		pBlackboard->SetData(BB::AngularVelocity, 1.57f);

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
		pBlackboard->RemoveData(BB::HouseToVisit);
		visitedHouses.push_back(hi.Center);
		if (visitedHouses.size() > 4)
		{
			visitedHouses.erase(visitedHouses.begin());
		}
		pBlackboard->SetData(BB::VisitedHouses, visitedHouses);

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
		//std::cout << "FaceEnemy\n";

		IExamInterface* pInterface;
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		EnemyInfo ei;
		if (!pBlackboard->GetData(BB::Enemy, ei))
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		pBlackboard->GetData(BB::TargetPositions, pTargetPositions);

		auto agentInfo = pInterface->Agent_GetInfo();

		const Elite::Vector2 toTarget{ ei.Location - agentInfo.Position };
		const float angle{ 180 / float(M_PI) * agentInfo.Orientation };
		const float angleTarget{ 180 / float(M_PI) * atan2(toTarget.y, toTarget.x) };
		float angleToRotate = angleTarget - angle;

		if (angleToRotate > 180.f)
		{
			angleToRotate -= 360.f;
		}
		if (angleToRotate < -180.f)
		{
			angleToRotate += 360.f;
		}


		auto av = min(agentInfo.MaxAngularSpeed, max(-agentInfo.MaxAngularSpeed, angleToRotate/10.f));
		//auto av = angleTarget - angle < 0 ? -5.f : 5.f;

		//std::cout << "angle: " << angle << ", angleTarget: " << angleTarget << ", angularVelocity: " << av << "\n";

		//if (angle - angleTarget > 5) pBlackboard->SetData(BB::AngularVelocity, -5);
		//else if (angle - angleTarget < -5)  pBlackboard->SetData(BB::AngularVelocity, 5);
		//else return BehaviorState::Success;
		BehaviorState result;
		if (abs(angleTarget - angle) < 3.f)
		{
			av = 0;
			result = BehaviorState::Success;
		}
		else
		{
			auto vectorlToEnemy = ei.Location - agentInfo.Position;

			if (vectorlToEnemy.Magnitude() < 5.f)
			{
				BT_Utils::AddTarget(pTargetPositions, { -10.f * vectorlToEnemy.GetNormalized() + agentInfo.Position });
				pBlackboard->SetData(BB::CanRun, true);
			}
			result = BehaviorState::Running;
		}

		////std::cout << "FaceEnemy: angle=" << angle << ", angleTarget=" << angleTarget << ", angularVelocity=" << av  << "\n";

		pBlackboard->SetData(BB::AngularVelocity, av);
		return result;
	}

	BehaviorState SearchInHouse(Blackboard* pBlackboard)
	{
		return BehaviorState::Success;
	}


	BehaviorState Wander(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		if (!pBlackboard->GetData(BB::TargetPositions, pTargetPositions))
		{
			return BehaviorState::Failure;
		}

		WorldInfo worldInfo = pInterface->World_GetInfo();
		if (pTargetPositions->size() < 1)
		{
			float angles[] = {90.f, 180.f, 270.f, 0.f, 225.f, 135.f, 315.f, 45.f  };

			float d = worldInfo.Dimensions.Magnitude() / 4.f;

			for (const float angle : angles)
			{
				const float angleRadians = angle / 180.f * float(M_PI);

				BT_Utils::AddTarget(pTargetPositions, worldInfo.Center + Vector2{d * std::cosf(angleRadians), d * std::sinf(angleRadians)});
			}
			
		}
		return BehaviorState::Success;
	}

	BehaviorState ShootEnemy(Blackboard* pBlackboard)
	{
		//std::cout << "ShootEnemy\n";

		IExamInterface* pInterface;
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		EnemyInfo ei;
		if (!pBlackboard->GetData(BB::Enemy, ei))
		{
			return BehaviorState::Failure;
		}

		auto agentInfo = pInterface->Agent_GetInfo();

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
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		pBlackboard->SetData(BB::AngularVelocity,1.f);
			
		return BehaviorState::Success;
	}

	BehaviorState EvadePurgeZone(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface;
		if (!pBlackboard->GetData(BB::Interface, pInterface) || pInterface == nullptr)
		{
			return BehaviorState::Failure;
		}

		std::deque<Vector2>* pTargetPositions;
		if (!pBlackboard->GetData(BB::TargetPositions, pTargetPositions))
		{
			return BehaviorState::Failure;
		}

		AgentInfo agentInfo = pInterface->Agent_GetInfo();

		PurgeZoneInfo pi;
		pBlackboard->GetData(BB::PurgeZone, pi);

		Vector2 pointOutside = (agentInfo.Position - pi.Center);
		pointOutside = (pointOutside * (pi.Radius + 10.f) / pointOutside.Magnitude()) + pi.Center;

		if (agentInfo.Position.Distance(pi.Center) < pointOutside.Distance(pi.Center))
		{
			BT_Utils::AddTarget(pTargetPositions, pointOutside);
		}

		return BehaviorState::Success;
	}

}
