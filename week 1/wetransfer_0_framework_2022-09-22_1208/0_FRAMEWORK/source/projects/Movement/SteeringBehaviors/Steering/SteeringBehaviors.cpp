//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, {0,1,0});

	return steering;
}

//Flee
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 toTarget = pAgent->GetPosition() - (m_Target).Position;
	float distanceSquared = toTarget.MagnitudeSquared();
	
	if(distanceSquared > m_FleeRadius * m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;
	}
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= -pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });

	return steering;
}

//Arrive
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const float arrivalRadius = 1.f;
	const float slowRadius = 15.f;

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	const float distance = steering.LinearVelocity.Magnitude();
	steering.LinearVelocity.Normalize();

	//stops turning
	if (distance < arrivalRadius)
	{
		steering.LinearVelocity = { 0.f,0.f };
		return steering;
	}

	//slows down
	if (distance < slowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * distance / slowRadius;
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });

	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(false);
	
	SteeringOutput steering{};

	const Elite::Vector2 toTarget{m_Target.Position - pAgent->GetPosition()};
	const float angle{180 / float(M_PI) * pAgent->GetRotation()};
	const float angleTarget{ 180 / float(M_PI) * atan2(toTarget.y, toTarget.x) };

	//std::cout << angle << "  " << angleTarget << '\n';

	if(angle - angleTarget > 5) steering.AngularVelocity = -5;
	else if (angle - angleTarget < -5) steering.AngularVelocity = 5;
	
	/*if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, {0,1,0});*/

	return steering;
}

//Wander
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	float rndAngle{ Elite::ToRadians( float(rand() % 360 - 180)) };

	Elite::Vector2 middlePCircle{ pAgent->GetPosition().x + cosf(pAgent->GetRotation()) * m_OffsetDistance,
			pAgent->GetPosition().y + sinf(pAgent->GetRotation()) * m_OffsetDistance };
	Elite::Vector2 rndPoint{ middlePCircle.x + cosf(rndAngle) * m_Radius, middlePCircle.y + sinf(rndAngle) * m_Radius };

	//Elite::Vector2 newDir{ pAgent->GetDirection().x + cosf(rndNr + pAgent->GetRotation()),
	//pAgent->GetDirection().y + sinf(rndNr + pAgent->GetRotation()) };

	Elite::Vector2 newDir{ rndPoint - pAgent->GetPosition() };
	steering.LinearVelocity = newDir;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawCircle({ pAgent->GetPosition().x + cosf(pAgent->GetRotation()) * m_OffsetDistance,
			pAgent->GetPosition().y + sinf(pAgent->GetRotation()) * m_OffsetDistance }, m_Radius, {0, 0, 1}, -0.8f);
		DEBUGRENDERER2D->DrawPoint(rndPoint, 5, { 1,0,0 });
	}

	return steering;
}

//Pursuit
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	//pAgent->SetAutoOrient(false);

	SteeringOutput steering = {};

	//Elite::Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };
	//float dToTarget{toTarget.Magnitude()};
	//Elite::Vector2 speedTarget{ m_Target.LinearVelocity };

	m_Vec = { ( m_Target.Position - pAgent->GetPosition()) + m_Target.LinearVelocity};
	steering.LinearVelocity = m_Vec;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	m_Target.GetDirection();
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), (m_Target.Position - pAgent->GetPosition()), 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), m_Target.LinearVelocity, 5, { 0,0,1 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), m_Vec, 5, { 1, 0, 0 });
	}

	return steering;
}

//evade
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Elite::Vector2 evadingVec{ (pAgent->GetPosition() - m_Target.Position) + m_Target.LinearVelocity };
	steering.LinearVelocity = evadingVec;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });

	return steering;
}


