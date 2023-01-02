#pragma once

#include <Exam_HelperStructs.h>
#include <IExamInterface.h>
#include "EBehaviorTree.h"


namespace BT_Condition
{
	bool IsHouseInFOV(Elite::Blackboard* pBlackboard)
	{
		std::cout << "IsHouseInFOV\n";
		IExamInterface* pInterface;

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}


		std::vector<Elite::Vector2> visitedHouses;
		if (!pBlackboard->GetData("VisitedHouses", visitedHouses))
		{
			visitedHouses = std::vector<Elite::Vector2>();
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

	bool FoundLoot(Elite::Blackboard* pBlackboard)
	{
		std::cout << "FoundLoot\n";
		IExamInterface* pInterface;
		
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return false;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return false;
		}
		pSteering->AutoOrient = false;
		pSteering->AngularVelocity = 1.f;

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

}

namespace BT_Action
{
	void SetTargetPos(SteeringPlugin_Output* pSteering, Elite::Vector2& targetPos, AgentInfo& agentInfo)
	{
		pSteering->LinearVelocity = targetPos - agentInfo.Position; //Desired Velocity
		pSteering->LinearVelocity.Normalize(); //Normalize Desired Velocity
		pSteering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	}

	bool IsInRect(Elite::Vector2 center, Elite::Vector2 size, Elite::Vector2 pos)
	{
		return std::abs(pos.x - center.x) <= size.x / 2.f
			&& std::abs(pos.y - center.y) <= size.y / 2.f;
	}

	Elite::BehaviorState MoveToHouse(Elite::Blackboard* pBlackboard)
	{
		std::cout << "MoveToHouse\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData("HouseToVisit", hi))
		{
			return Elite::BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 checkPointLocation{ hi.Center };
		auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(checkPointLocation);

		auto agentInfo = pInterface->Agent_GetInfo();

		SetTargetPos(pSteering, nextTargetPos, agentInfo);

		if (agentInfo.Position.Distance(checkPointLocation) < 2.f)
		{
			pSteering->AngularVelocity = 1.f;
			pSteering->AutoOrient = false;
			pSteering->LinearVelocity = Elite::ZeroVector2;


			std::vector<Elite::Vector2> visitedHouses;
			if (!pBlackboard->GetData("VisitedHouses", visitedHouses))
			{
				visitedHouses = std::vector<Elite::Vector2>();
			}
			visitedHouses.push_back(hi.Center);
			pBlackboard->SetData("VisitedHouses", visitedHouses);

			return Elite::BehaviorState::Success;
		}

		pSteering->AutoOrient = true;
		return Elite::BehaviorState::Running;
	}

	Elite::BehaviorState MoveToLoot(Elite::Blackboard* pBlackboard)
	{
		std::cout << "MoveToLoot\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		EntityInfo ei;
		if (!pBlackboard->GetData("ItemToPick", ei))
		{
			return Elite::BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return Elite::BehaviorState::Failure;
		}

		Elite::Vector2 checkPointLocation{ ei.Location };
		auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(checkPointLocation);

		auto agentInfo = pInterface->Agent_GetInfo();

		SetTargetPos(pSteering, nextTargetPos, agentInfo);
		pSteering->AutoOrient = true;

		pBlackboard->SetData("Steering", pSteering);

		float distance = agentInfo.Position.Distance(checkPointLocation);
		if (distance < 3.f)
		{
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Running;
	}

	Elite::BehaviorState HandleLoot(Elite::Blackboard* pBlackboard)
	{
		std::cout << "TakeLoot\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		EntityInfo ei;
		if (!pBlackboard->GetData("ItemToPick", ei))
		{
			return Elite::BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return Elite::BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData("HouseToVisit", hi))
		{
			return Elite::BehaviorState::Failure;
		}

		int inventorySlot = 0;
		pBlackboard->GetData("InventorySlot", inventorySlot);

		ItemInfo ii;
		if (pInterface->Item_GetInfo(ei, ii))
		{
			SetTargetPos(pSteering, hi.Center, pInterface->Agent_GetInfo());

			if (ii.Type == eItemType::GARBAGE)
			{
				pInterface->Item_Destroy(ei);
			}
			else
			{
				pInterface->Item_Grab(ei, ii);

				if (inventorySlot < pInterface->Inventory_GetCapacity())
				{
					pInterface->Inventory_AddItem(inventorySlot, ii);
					++inventorySlot;
					pBlackboard->SetData("InventorySlot", inventorySlot);
				}
			}

			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState LeaveHouse(Elite::Blackboard* pBlackboard)
	{

		std::cout << "LeaveHouse\n";
		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return Elite::BehaviorState::Failure;
		}

		HouseInfo hi;
		if (!pBlackboard->GetData("HouseToVisit", hi))
		{
			return Elite::BehaviorState::Failure;
		}


		Elite::Vector2 checkPointLocation{ pInterface->World_GetInfo().Dimensions.x ,0.f };
		auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(checkPointLocation);

		auto agentInfo = pInterface->Agent_GetInfo();
		SetTargetPos(pSteering, nextTargetPos, agentInfo);
		pSteering->AutoOrient = true;


		return IsInRect(hi.Center, hi.Size, agentInfo.Position)
			? Elite::BehaviorState::Running
			: Elite::BehaviorState::Success;
	}
}
