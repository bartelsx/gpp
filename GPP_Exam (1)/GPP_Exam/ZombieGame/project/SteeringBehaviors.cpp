//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"

#include <IExamInterface.h>

#include "EBehaviorTree.h"

//SEEK
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

//Flee
//****
SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	Elite::Vector2 toTarget = pAgent->Position - m_Target.Position;
	float distanceSquared = toTarget.MagnitudeSquared();
	
	if(distanceSquared > m_FleeRadius * m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	steering.LinearVelocity = toTarget;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

//Arrive
//****
SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	const float arrivalRadius = 1.f;
	const float slowRadius = 15.f;

	steering.LinearVelocity = m_Target.Position - pAgent->Position;
	const float distance = steering.LinearVelocity.Magnitude();
	steering.LinearVelocity.Normalize();

	//stops turning
	if (distance < arrivalRadius)
	{
		steering.LinearVelocity = Elite::Vector2{  0.f,0.f };
		return steering;
	}

	//slows down
	if (distance < slowRadius)
	{
		steering.LinearVelocity *= pAgent->MaxLinearSpeed * distance / slowRadius;
	}
	else
	{
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
	}

	return steering;
}

SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};

	steering.AutoOrient = false;

	const Elite::Vector2 toTarget{m_Target.Position - pAgent->Position};
	const float angle{180 / float(M_PI) * pAgent->Orientation};
	const float angleTarget{ 180 / float(M_PI) * atan2(toTarget.y, toTarget.x) };

	if(angle - angleTarget > 5) steering.AngularVelocity = -5;
	else if (angle - angleTarget < -5) steering.AngularVelocity = 5;
	
	/*if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->Position, steering.LinearVelocity, 5, {0,1,0});*/

	return steering;
}

//Wander
//****
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	//IExamInterface* pInterface;
	//if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
	//{
	//	return steering;
	//}



	

	float rndAngle{ Elite::ToRadians( 40) };

	Elite::Vector2 rndPoint{ pAgent->Position.x + cosf(pAgent->Orientation + rndAngle) * pAgent->MaxAngularSpeed,
			pAgent->Position.y + sinf(pAgent->Orientation+rndAngle) * pAgent->MaxAngularSpeed };
	
	//Elite::Vector2 newDir{ pAgent->GetDirection().x + cosf(rndNr + pAgent->Orientation),
	//pAgent->GetDirection().y + sinf(rndNr + pAgent->Orientation) };
	if(m_time >m_timeToChange)
	{
		Elite::Vector2 newDir{ rndPoint - pAgent->Position };
		steering.AutoOrient = false;
		steering.AngularVelocity = 1.f;
		steering.LinearVelocity = newDir;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
		m_OldVelocity = newDir;
		m_time = 0;
		m_timeToChange += 0.5f;
	}
	else if( m_time< 2 )
	{
		steering.LinearVelocity = m_OldVelocity;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
		steering.AutoOrient = true;
		m_time += deltaT;
	}
	else
	{
		steering.LinearVelocity = m_OldVelocity;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
		m_time += deltaT;
		
	}


	

	

	return steering;
}

//Pursuit
//****
SteeringPlugin_Output Pursuit::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	
	SteeringPlugin_Output steering = {};

	m_Vec = {  m_Target.Position - pAgent->Position + m_Target.LinearVelocity};
	steering.LinearVelocity = m_Vec;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	return steering;
}

//evade
SteeringPlugin_Output Evade::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};

	Elite::Vector2 evadingVec{ pAgent->Position - (m_Target.Position + m_Target.LinearVelocity)};
	if(evadingVec.Magnitude() < 10.f)
	{
		
		steering.LinearVelocity = evadingVec;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->MaxLinearSpeed;
		steering.IsValid = true;
	}else
	{
		steering.IsValid = false;
	}

	return steering;
}


