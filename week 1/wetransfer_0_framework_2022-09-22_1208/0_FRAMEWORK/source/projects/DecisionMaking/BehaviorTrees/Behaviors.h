/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace  BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
			return Elite::BehaviorState::Failure;

		pAgent->SetToWander();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		Elite::Vector2 targetPos;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
			return Elite::BehaviorState::Failure;
		if (!pBlackboard->GetData("Target", targetPos))
			return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(targetPos);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		Elite::Vector2 targetPos;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
			return Elite::BehaviorState::Failure;
		if (!pBlackboard->GetData("Target", targetPos))
			return Elite::BehaviorState::Failure;

		pAgent->SetToFlee(targetPos);
		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioFood*>* pFoodVec;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
			return false;
		if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr)
			return false;

		const float searchRadius = { 30.f + pAgent->GetRadius() };
		//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), searchRadius, { 0, 0, 1 }, 0.f);

		AgarioFood* pClosestFood = nullptr;
		float closestDistSqrt = { searchRadius * searchRadius };

		const Elite::Vector2 agentPos = pAgent->GetPosition();
		//Todo: Debug rendering!!!

		for (auto& pFood : *pFoodVec)
		{
			float distSqrt = pFood->GetPosition().DistanceSquared(agentPos);
			if (distSqrt < closestDistSqrt)
			{
				pClosestFood = pFood;
				closestDistSqrt = distSqrt;
			}
		}

		if (pClosestFood != nullptr)
		{
			DEBUGRENDERER2D->DrawSegment(pClosestFood->GetPosition(), pAgent->GetPosition(), { 0, 0, 1 });
			pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
			return true;
		}

		return false;
	}

	bool IsBiggerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pAgentsVec;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
			return false;
		if (!pBlackboard->GetData("AgentsVec", pAgentsVec) || pAgentsVec == nullptr)
			return false;

		const float searchRadius = { 40.f + pAgent->GetRadius() };
		//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), searchRadius, { 1, 0, 0 }, 0.f);

		AgarioAgent* pClosestEnemy = nullptr;
		float closestDistSqrt = { searchRadius * searchRadius };

		const Elite::Vector2 agentPos = pAgent->GetPosition();
		//Todo: Debug rendering!!!

		for (auto& pEnemy : *pAgentsVec)
		{
			float distSqrt = pEnemy->GetPosition().DistanceSquared(agentPos);
			if (distSqrt < closestDistSqrt)
			{
				pClosestEnemy = pEnemy;
				closestDistSqrt = distSqrt;
			}
		}

		if (pClosestEnemy != nullptr)
		{
			if (pClosestEnemy->GetRadius() > pAgent->GetRadius())
			{
				DEBUGRENDERER2D->DrawSegment(pClosestEnemy->GetPosition(), pAgent->GetPosition(), { 1,0,0 });
				pBlackboard->ChangeData("Target", pClosestEnemy->GetPosition());
				return true;
			}
		}

		return false;
	}

	bool IsSmallerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pAgentsVec;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
			return false;
		if (!pBlackboard->GetData("AgentsVec", pAgentsVec) || pAgentsVec == nullptr)
			return false;

		const float searchRadius = { 35.f + pAgent->GetRadius() };
		//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), searchRadius, {0, 1, 0}, 0.f);

		AgarioAgent* pClosestEnemy = nullptr;
		float closestDistSqrt = { searchRadius * searchRadius };

		const Elite::Vector2 agentPos = pAgent->GetPosition();
		//Todo: Debug rendering!!!

		for (auto& pEnemy : *pAgentsVec)
		{
			float distSqrt = pEnemy->GetPosition().DistanceSquared(agentPos);
			if (distSqrt < closestDistSqrt)
			{
				pClosestEnemy = pEnemy;
				closestDistSqrt = distSqrt;
			}
		}

		if (pClosestEnemy != nullptr)
		{
			if (pClosestEnemy->GetRadius() < pAgent->GetRadius() - pAgent->GetRadius() - 1) // there needs to be a difference of atleast 1 in size
			{
				DEBUGRENDERER2D->DrawSegment(pClosestEnemy->GetPosition(), pAgent->GetPosition(), { 0,1,0 });
				pBlackboard->ChangeData("Target", pClosestEnemy->GetPosition());
				return true;
			}
		}

		return false;
	}
}
#endif