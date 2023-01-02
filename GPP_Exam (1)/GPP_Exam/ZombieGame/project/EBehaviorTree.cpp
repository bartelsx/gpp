//=== General Includes ===
#include "stdafx.h"
#include "EBehaviorTree.h"

#include <chrono>
using namespace Elite;

//-----------------------------------------------------------------
// BEHAVIOR TREE COMPOSITES (IBehavior)
//-----------------------------------------------------------------
#pragma region COMPOSITES
//SELECTOR
BehaviorState BehaviorSelector::Execute(Blackboard* pBlackBoard)
{
	//TODO: Fill in this code
	// Loop over all children in m_ChildBehaviors
	for(auto& child : m_ChildBehaviors)
	{
		m_CurrentState = child->Execute(pBlackBoard);
		

		switch (m_CurrentState)
		{
		case Elite::BehaviorState::Failure:
			continue;
		case Elite::BehaviorState::Success:
			return m_CurrentState;
		case Elite::BehaviorState::Running:
			return m_CurrentState;
		default:
			break;
		}
	
	}
	
		//Every Child: Execute and store the result in m_CurrentState

		//Check the currentstate and apply the selector Logic:
		//if a child returns Success:
			//stop looping over all children and return Success
		//if a child returns Running:
			//Running: stop looping and return Running

		//The selector fails if all children failed.

	//All children failed
	m_CurrentState = BehaviorState::Failure;
	return m_CurrentState;
}
//SEQUENCE
BehaviorState BehaviorSequence::Execute(Blackboard* pBlackBoard)
{
	//TODO: FIll in this code
	//Loop over all children in m_ChildBehaviors

		//Every Child: Execute and store the result in m_CurrentState

		//Check the currentstate and apply the sequence Logic:
		//if a child returns Failed:
			//stop looping over all children and return Failed
		//if a child returns Running:
			//Running: stop looping and return Running

		//The selector succeeds if all children succeeded.
	for (auto& child : m_ChildBehaviors)
	{
		m_CurrentState = child->Execute(pBlackBoard);


		
		
		if(m_CurrentState == Elite::BehaviorState::Success)
		{
			continue;
		}
		if(m_CurrentState == Elite::BehaviorState::Running)
		{
			return m_CurrentState;
		}
		if(m_CurrentState == Elite::BehaviorState::Failure)
		{
			return m_CurrentState;
		}
	}
	//All children succeeded 
	m_CurrentState = BehaviorState::Success;
	return m_CurrentState;
}
//PARTIAL SEQUENCE
BehaviorState BehaviorPartialSequence::Execute(Blackboard* pBlackBoard)
{
	while (m_CurrentBehaviorIndex < m_ChildBehaviors.size())
	{
		m_CurrentState = m_ChildBehaviors[m_CurrentBehaviorIndex]->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			m_CurrentBehaviorIndex = 0;
			return m_CurrentState;
		case BehaviorState::Success:
			++m_CurrentBehaviorIndex;
			m_CurrentState = BehaviorState::Running;
			return m_CurrentState;
		case BehaviorState::Running:
			return m_CurrentState;
		}
	}

	m_CurrentBehaviorIndex = 0;
	m_CurrentState = BehaviorState::Success;
	return m_CurrentState;
}
#pragma endregion

//-----------------------------------------------------------------
// BEHAVIOR TREE DECORATORS (IBehavior)
//-----------------------------------------------------------------
BehaviorState BehaviorRepeat::Execute(Blackboard* pBlackBoard)
{
	auto result = m_Child->Execute(pBlackBoard);

	if (result == BehaviorState::Failure)
	{
		return BehaviorState::Success;
	}
	return BehaviorState::Running;
}

BehaviorState BehaviorMaskFailure::Execute(Blackboard* pBlackBoard)
{
	auto result = m_Child->Execute(pBlackBoard);

	if (result == BehaviorState::Running)
	{
		return BehaviorState::Running;
	}
	return BehaviorState::Success;
}

//-----------------------------------------------------------------
// BEHAVIOR TREE CONDITIONAL (IBehavior)
//-----------------------------------------------------------------
BehaviorState BehaviorConditional::Execute(Blackboard* pBlackBoard)
{
	if (m_fpConditional == nullptr)
		return BehaviorState::Failure;

	switch (m_fpConditional(pBlackBoard))
	{
	case true:
		m_CurrentState = BehaviorState::Success;
		return m_CurrentState;
	default:
	case false:
		m_CurrentState = m_CurrentState = BehaviorState::Failure;
		return m_CurrentState;
	}
}

BehaviorState BehaviorTimedConditional::Execute(Blackboard* pBlackBoard)
{
	if (m_fpConditional == nullptr)
		return BehaviorState::Failure;

	if (!m_isRunning) //not yet running
	{
		m_startTime = time(nullptr);
		m_isRunning = true;
	}

	switch (m_fpConditional(pBlackBoard))
	{
	case true:
		m_isRunning = false;
		m_CurrentState = BehaviorState::Success;
		return m_CurrentState;
	default:
	case false:
		time_t now = time(nullptr);
		if (now-m_startTime >= m_timeoutSeconds)
		{
			m_isRunning = false;
			m_CurrentState = BehaviorState::Failure;
		}
		else
		{
			m_CurrentState = m_CurrentState = BehaviorState::Running;
		}
		return m_CurrentState;
	}

}
//-----------------------------------------------------------------
// BEHAVIOR TREE ACTION (IBehavior)
//-----------------------------------------------------------------
BehaviorState BehaviorAction::Execute(Blackboard* pBlackBoard)
{
	if (m_fpAction == nullptr)
		return BehaviorState::Failure;

	m_CurrentState = m_fpAction(pBlackBoard);
	return m_CurrentState;
}