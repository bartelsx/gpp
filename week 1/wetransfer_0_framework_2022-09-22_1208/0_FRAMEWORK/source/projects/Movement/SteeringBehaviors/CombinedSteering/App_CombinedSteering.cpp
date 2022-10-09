//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_CombinedSteering.h"
#include "../SteeringAgent.h"
#include "CombinedSteeringBehaviors.h"
#include "projects\Movement\SteeringBehaviors\Obstacle.h"

using namespace Elite;
App_CombinedSteering::~App_CombinedSteering()
{	
	SAFE_DELETE(m_pDrunkAgent);
	SAFE_DELETE(m_pBlenderSteering);
	SAFE_DELETE(m_pDrunkWander);
	SAFE_DELETE(m_pSeek);

	SAFE_DELETE(m_pEvadingAgent);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pFlee);
	SAFE_DELETE(m_pSoberWander);
	SAFE_DELETE(m_pEvade);
}



void App_CombinedSteering::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(55.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize / 1.5f, m_TrimWorldSize / 2));

	m_pSeek = new Seek();
	m_pDrunkWander = new Wander();
	m_pDrunkWander->SetWanderOffset(0);
	m_pBlenderSteering = new BlendedSteering({{ m_pSeek,0.5f }, { m_pDrunkWander, 0.5f }});

	m_pDrunkAgent = new SteeringAgent();
	m_pDrunkAgent->SetSteeringBehavior(m_pSeek);
	m_pDrunkAgent->SetMaxLinearSpeed(15.0f);
	m_pDrunkAgent->SetAutoOrient(true);
	m_pDrunkAgent->SetBodyColor({ 1.f, 0.f, 0.f });
	m_pDrunkAgent->SetMass(0.3f);

	m_pSoberWander = new Wander;
	m_pFlee = new Flee;
	m_pPrioritySteering = new PrioritySteering({m_pFlee, m_pSoberWander});

	m_pEvade = new Evade();
	m_pEvadingAgent = new SteeringAgent();
	m_pEvadingAgent->SetSteeringBehavior(m_pEvade);
	m_pEvadingAgent->SetMaxLinearSpeed(15.0f);
	m_pEvadingAgent->SetAutoOrient(true);
	m_pEvadingAgent->SetBodyColor({ 0.f, 1.f, 0.f });
	m_pDrunkAgent->SetMass(0.3f);
}


void App_CombinedSteering::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}

#ifdef PLATFORM_WINDOWS
	#pragma region UI
	//UI
	{
		//Setup
		int const menuWidth = 235;
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
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
		ImGui::Checkbox("Trim World", &m_TrimWorld);
		if (m_TrimWorld)
		{
			ImGui::SliderFloat("Trim Size", &m_TrimWorldSize, 0.f, 500.f, "%1.");
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();
		
		ImGui::SliderFloat("Seek", &m_pBlenderSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
		ImGui::SliderFloat("Wander", &m_pBlenderSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	#pragma endregion
#endif
	m_pSeek->SetTarget(m_MouseTarget);
	m_pDrunkAgent->Update(deltaTime);
	if(m_TrimWorld)
	{
		m_pDrunkAgent->TrimToWorld(m_TrimWorldSize);
		m_pEvadingAgent->TrimToWorld(m_TrimWorldSize);
		
	}
	m_pDrunkAgent->GetSteeringBehavior()->CalculateSteering(deltaTime, m_pDrunkAgent);
	m_pEvadingAgent->GetSteeringBehavior()->CalculateSteering(deltaTime, m_pEvadingAgent);
	m_pEvadingAgent->Update(deltaTime);

	/*TargetData evadeTarget;
	evadeTarget.LinearVelocity = m_pDrunkAgent->GetLinearVelocity();
	evadeTarget.Position = m_pDrunkAgent->GetPosition();

	m_pFlee->SetTarget(evadeTarget);*/

}

void App_CombinedSteering::Render(float deltaTime) const
{
	m_pDrunkAgent->Render(deltaTime);
	m_pDrunkAgent->SetRenderBehavior(m_CanDebugRender);

	m_pEvadingAgent->Render(deltaTime);
	m_pEvadingAgent->SetRenderBehavior(m_CanDebugRender);

	if (m_TrimWorld)
	{
		RenderWorldBounds(m_TrimWorldSize);
		m_pEvadingAgent->TrimToWorld(m_TrimWorldSize);
	}
	DEBUGRENDERER2D->DrawDirection(m_pEvadingAgent->GetPosition(), (m_pDrunkAgent->GetLinearVelocity() - m_pEvadingAgent->GetLinearVelocity()), 10.f, Color(255, 0, 0));
	DEBUGRENDERER2D->DrawDirection(m_pEvadingAgent->GetPosition(), (m_pDrunkAgent->GetPosition() - m_pEvadingAgent->GetPosition()), 10.f, Color(0, 255, 0));
	DEBUGRENDERER2D->DrawDirection(m_pEvadingAgent->GetPosition(), (m_pDrunkAgent->GetPosition() - m_pEvadingAgent->GetPosition())+ (m_pDrunkAgent->GetLinearVelocity() - m_pEvadingAgent->GetLinearVelocity()), 10.f, Color(0, 0, 255));


	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);
}
