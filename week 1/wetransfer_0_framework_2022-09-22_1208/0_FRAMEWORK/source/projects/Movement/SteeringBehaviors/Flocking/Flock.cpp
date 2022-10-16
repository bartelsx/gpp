#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

using namespace Elite;



//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	//, m_NrOfNeighbors{0}
{
	m_Agents.resize(m_FlockSize);

	// TODO: initialize the flock and the memory pool
	//m_pBlendedSteering = new BlendedSteering()
	
	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pEvadeBehavior = new EvadeFlock(this);

	m_pCellSpace = new CellSpace(m_WorldSize, m_WorldSize, m_Rows, m_Cols, m_FlockSize);

	std::vector< BlendedSteering::WeightedBehavior> vecWeightedB{ { m_pCohesionBehavior, 5 },
		{ m_pSeparationBehavior, 6 }, { m_pVelMatchBehavior, 1 } };
	m_pBlendedSteering = new BlendedSteering{ vecWeightedB };

	std::vector<ISteeringBehavior*> priority_steerings{ m_pEvadeBehavior ,m_pBlendedSteering };
	m_pPrioritySteering =  new PrioritySteering(priority_steerings) ;

	for (int i = 0; i < m_FlockSize; ++i)
	{
		m_Agents[i] = new SteeringAgent();
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMaxLinearSpeed(100.0f);
		m_Agents[i]->SetMass(0.3f);
		m_Agents[i]->SetPosition({ float(rand() % int(worldSize + 1)), float(rand() % int(worldSize + 1)) });
		m_pCellSpace->AddAgent(m_Agents[i]);
	}


	m_pSeek = new Seek();
	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetSteeringBehavior(m_pSeek);
	m_pAgentToEvade->SetMaxLinearSpeed(15.0f);
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetBodyColor({ 1.f, 0.f, 0.f });
	m_pAgentToEvade->SetMass(0.3f);

}

Flock::~Flock()
{
	// TODO: clean up any additional data

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pAgentToEvade);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pEvadeBehavior);


	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();
	m_pCellSpace->EmptyCells();
	SAFE_DELETE(m_pCellSpace);
}

void Flock::Update(float deltaT)
{
	// TODO: update the flock
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}


	for (size_t i = 0; i < m_FlockSize; ++i)
	{
		if (m_Agents[i])
		{
			m_pCellSpace->RegisterNeighbors(m_Agents[i], m_NeighborhoodRadius,&m_Neighbors);
			m_NrOfNeighbors = m_Neighbors.size();
			//RegisterNeighbors(m_Agents[i]);

			m_pCellSpace->UpdateAgentCell(m_Agents[i], m_Agents[i]->GetOldPos());
			m_Agents[i]->Update(deltaT);

			if (m_TrimWorld)
			{
				m_Agents[i]->TrimToWorld(m_WorldSize);
			}

			if (i == 10)
			{
				for (int j{}; j < m_NrOfNeighbors; ++j)
				{
					DEBUGRENDERER2D->DrawPoint(m_Neighbors[j]->GetPosition(), 10, { 1,0,1 });
					//DEBUGRENDERER2D->DrawPoint(this->GetAverageNeighborPos(), 10, { 0,1,0 });
					DEBUGRENDERER2D->DrawCircle(m_Agents[i]->GetPosition(), m_NeighborhoodRadius, { 1,0,0 }, -0.8f);
					DEBUGRENDERER2D->DrawPoint(GetAverageNeighborPos(), 5, { 0,1,0 }, -0.8f);
					
				}
			}
			m_Neighbors.clear();
		}
	}
	m_pCellSpace->RenderCells();
	m_pSeek->SetTarget(m_MouseTarget);
	m_pAgentToEvade->Update(deltaT);
	if (m_TrimWorld)
	{
		m_pAgentToEvade->TrimToWorld(m_WorldSize);
	}

}

void Flock::Render(float deltaT)
{
	// TODO: render the flock
	for (int i = 0; i < m_FlockSize; ++i)
	{	
		if (m_Agents[i])
		{
			m_Agents[i]->Render(deltaT);
		}
	}
	//m_pAgentToEvade->SetRenderBehavior(m_CanDebugRender);
	m_pAgentToEvade->Render(deltaT);


}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// TODO: Implement checkboxes for debug rendering and weight sliders here
	ImGui::SliderFloat("Weight Cohesion", GetWeight(m_pCohesionBehavior), 0, 10);
	ImGui::SliderFloat("Weight Separation", GetWeight(m_pSeparationBehavior), 0, 10);
	ImGui::SliderFloat("Weight MatchVelocity", GetWeight(m_pVelMatchBehavior), 0, 10);
	

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();



}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	
	m_NrOfNeighbors = 0;
	
	for (int i = 0; i < m_FlockSize; ++i)
	{
		if (m_Agents[i] != pAgent)
		{
			Elite::Vector2 ToAgent{ pAgent->GetPosition() - m_Agents[i]->GetPosition()};
			if (ToAgent.Magnitude() < m_NeighborhoodRadius)
			{
				m_Neighbors.push_back(m_Agents[i]);
				++m_NrOfNeighbors;
			}
		}
	}

	
	
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	
	Elite::Vector2 average{};
	for (size_t i = 0; i < m_Neighbors.size(); ++i)
	{
		average += m_Neighbors[i]->GetPosition();
	}
	average /= m_NrOfNeighbors;

	return average;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	
	Elite::Vector2 averageSpeed{};
	for (size_t i = 0; i < m_Neighbors.size(); ++i)
	{
		averageSpeed += m_Neighbors[i]->GetLinearVelocity();
	}
	averageSpeed /= m_NrOfNeighbors;

	return averageSpeed;
}

void Flock::SetTarget_Seek(TargetData target)
{
	// TODO: Set target for seek behavior
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

SteeringAgent*  Flock::GetAgentToEvade() const
{
	return m_pAgentToEvade;

}