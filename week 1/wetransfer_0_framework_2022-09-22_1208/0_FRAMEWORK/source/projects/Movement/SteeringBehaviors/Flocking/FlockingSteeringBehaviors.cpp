#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Elite::Vector2 direction{ m_pFlock->GetAverageNeighborPos() - pAgent->GetPosition() };


	steering.LinearVelocity = direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Elite::Vector2 direction{ };
	Elite::Vector2 endvec{};

	for (int i = 0; i < m_pFlock->GetNrOfNeighbors(); ++i)
	{
		direction = m_pFlock->GetNeighbors()[i]->GetPosition() - pAgent->GetPosition();

		endvec -= direction;
	}

	steering.LinearVelocity = endvec;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}
//evade
SteeringOutput EvadeFlock::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	SteeringAgent* pAgentToEvade{ m_pFlock->GetAgentToEvade() };
	Elite::Vector2 evadingVec{ (pAgent->GetPosition() - pAgentToEvade->GetPosition()) };
	if (evadingVec.Magnitude() < 10.f)
	{

		steering.LinearVelocity = evadingVec;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
		steering.IsValid = true;
	}
	else
	{
		steering.IsValid = false;
	}
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), evadingVec, 5, { 1,0,0 });

	return steering;
}
