/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "Exam_HelperStructs.h"

struct TargetData
{
	Elite::Vector2 Position;
	Elite::Vector2 LinearVelocity;
};


#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

///////////////////////////////////////
//flee
//****
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Flee Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	void SetFleeRadius(float radius) { m_FleeRadius = radius; }
private:
	float m_FleeRadius = 20.f;
};

///////////////////////////////////////
//Arrive
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Arrive Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

///////////////////////////////////////
//Face
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Face Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

private:
	float m_Angle = 0;
};

///////////////////////////////////////
//Wander
//****
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Face Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }

protected:
	float m_OffsetDistance = 6.f;
	float m_Radius = 4.f;
	const int m_MaxAngleChange = 45;
	float m_WanderAngle = 0.f;
	float m_time = 0.f;
	float m_timeToChange = 2.f;
	Elite::Vector2 m_OldVelocity{-1,-1};
};

///////////////////////////////////////
//Pursuit
//****
class Pursuit : public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Face Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;

private:
	Elite::Vector2 m_TempTarget, m_Vec;
};

class Evade : public ISteeringBehavior
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Flee Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	void SetFleeRadius(float radius) { m_FleeRadius = radius; }
private:
	float m_FleeRadius = 20.f;
};


#endif


