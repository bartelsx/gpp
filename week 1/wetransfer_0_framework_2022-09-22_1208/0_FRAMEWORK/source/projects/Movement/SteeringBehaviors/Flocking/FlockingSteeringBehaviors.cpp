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
	Elite::Vector2 direction{ m_pFlock->GetAverageNeighborPos() - pAgent->GetPosition()};


	steering.LinearVelocity = direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Seperation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
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
