#pragma once

#include <Exam_HelperStructs.h>
#include <IExamInterface.h>
#include "EBehaviorTree.h"

using namespace Elite;

namespace BT_Condition
{
	bool IsHouseInFOV(Blackboard* pBlackboard)
	{
		std::cout << "IsHouseInFOV\n";
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

	bool FoundLoot(Blackboard* pBlackboard)
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
	void SetTargetPos(SteeringPlugin_Output* pSteering, Vector2& targetPos, AgentInfo& agentInfo)
	{
		pSteering->LinearVelocity = targetPos - agentInfo.Position; //Desired Velocity
		pSteering->LinearVelocity.Normalize(); //Normalize Desired Velocity
		pSteering->LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed
	}

	bool IsInRect(Vector2 center, Vector2 size, Vector2 pos)
	{
		return std::abs(pos.x - center.x) <= size.x / 2.f
			&& std::abs(pos.y - center.y) <= size.y / 2.f;
	}

	BehaviorState MoveToHouse(Blackboard* pBlackboard)
	{
		std::cout << "MoveToHouse\n";
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

		SteeringPlugin_Output* pSteering;
		if (!pBlackboard->GetData("Steering", pSteering))
		{
			return BehaviorState::Failure;
		}

		Vector2 checkPointLocation{ hi.Center };
		auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(checkPointLocation);

		auto agentInfo = pInterface->Agent_GetInfo();

		SetTargetPos(pSteering, nextTargetPos, agentInfo);

		if (agentInfo.Position.Distance(checkPointLocation) < 2.f)
		{
			pSteering->AngularVelocity = 1.f;
			pSteering->AutoOrient = false;
			pSteering->LinearVelocity = Elite::ZeroVector2;


			std::vector<Vector2> visitedHouses;
			if (!pBlackboard->GetData("VisitedHouses", visitedHouses))
			{
				visitedHouses = std::vector<Vector2>();
			}
			visitedHouses.push_back(hi.Center);
			pBlackboard->SetData("VisitedHouses", visitedHouses);

			return BehaviorState::Success;
		}

		pSteering->AutoOrient = true;
		return BehaviorState::Running;
	}

	BehaviorState MoveToLoot(Blackboard* pBlackboard)
	{
		std::cout << "MoveToLoot\n";
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

		Vector2 checkPointLocation{ ei.Location };
		auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(checkPointLocation);

		auto agentInfo = pInterface->Agent_GetInfo();

		SetTargetPos(pSteering, nextTargetPos, agentInfo);
		pSteering->AutoOrient = true;

		pBlackboard->SetData("Steering", pSteering);

		float distance = agentInfo.Position.Distance(checkPointLocation);
		if (distance < 3.f)
		{
			return BehaviorState::Success;
		}

		return BehaviorState::Running;
	}

	BehaviorState HandleLoot(Blackboard* pBlackboard)
	{
		std::cout << "TakeLoot\n";
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

			return BehaviorState::Success;
		}

		return BehaviorState::Failure;
	}

	BehaviorState LeaveHouse(Blackboard* pBlackboard)
	{

		std::cout << "LeaveHouse\n";
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
		SetTargetPos(pSteering, nextTargetPos, agentInfo);
		pSteering->AutoOrient = true;


		return IsInRect(hi.Center, hi.Size, agentInfo.Position)
			? BehaviorState::Running
			: BehaviorState::Success;
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
			std::cout << m_debugName << "\n";

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
