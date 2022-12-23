#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMState;

void WanderState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	bool isValid = pBlackboard->GetData("Agent", pAgent);
	if (!isValid || pAgent == nullptr) return;

	pAgent->SetToWander();
}

void SeekFoodState::OnEnter(Blackboard* pBlackboard)
{
	//Todo
	AgarioAgent* pAgent;

	bool isValid = pBlackboard->GetData("Agent", pAgent);
	if (!isValid || pAgent == nullptr) return;

	AgarioFood* pFood;
	pBlackboard->GetData("FoodNearBy", pFood);

	pAgent->SetToSeek(pFood->GetPosition());

}

bool ::FSMCondition::FoodNearByCondition::Evaluate(Blackboard* pBlackboard) const
{
	const float foodRadius = 50.f;
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	bool isValid = pBlackboard->GetData("Agent", pAgent);
	if (!isValid || pAgent == nullptr) return false;

	isValid = pBlackboard->GetData("FoodVecPtr", pFoodVec);
	if (!isValid || pAgent == nullptr) return false;

	Vector2 agentPos = pAgent->GetPosition();

	auto elementDis = [agentPos](AgarioFood* pFood1, AgarioFood* pFood2)
	{
		float dist1 = agentPos.DistanceSquared(pFood1->GetPosition());
		float dist2 = agentPos.DistanceSquared(pFood2->GetPosition());
		return dist1 < dist2;
	};
	auto closestFoodIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), elementDis);

	if(closestFoodIt!=pFoodVec->end())
	{
		AgarioFood* pFood = *closestFoodIt;
		if(agentPos.DistanceSquared(pFood->GetPosition()) < foodRadius * foodRadius)
		{
			pBlackboard->ChangeData("FoodNearBy", pFood);
			return true;
		}
	}

	return false;
}


bool ::FSMCondition::FoodIsEatenCondition::Evaluate(Blackboard* pBlackboard) const
{
	//returns true if the foodnearby has been eaten
	AgarioFood* pFood;
	pBlackboard->GetData("FoodNearBy", pFood);
	return pFood->CanBeDestroyed();

}