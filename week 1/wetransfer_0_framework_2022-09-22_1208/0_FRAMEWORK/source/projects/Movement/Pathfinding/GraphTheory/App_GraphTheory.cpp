//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_GraphTheory.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EEularianPath.h"

using namespace Elite;
using namespace std;
//Destructor
App_GraphTheory::~App_GraphTheory()
{
	SAFE_DELETE(M_PGraph2D);
}

//Functions
void App_GraphTheory::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(0, 0));
	DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(false);
	DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(false);

	M_PGraph2D = new Graph2D<GraphNode2D, GraphConnection2D>(false);
	M_PGraph2D->AddNode(new GraphNode2D(0, { 20,30 }));
	M_PGraph2D->AddNode(new GraphNode2D(1, { -10,-10 }));
	M_PGraph2D->AddConnection(new GraphConnection2D(0, 1));
}

void App_GraphTheory::Update(float deltaTime)
{
	m_TotalTime += deltaTime;
	m_GraphEditor.UpdateGraph(M_PGraph2D);
	M_PGraph2D->SetConnectionCostsToDistance();
	

	auto eulerFinder = EulerianPath<GraphNode2D, GraphConnection2D>(M_PGraph2D);
	Eulerianity eulerianity;
	eulerianity = eulerFinder.IsEulerian();
	switch (eulerianity)
	{
	case Eulerianity::notEulerian:
		cout << "not eulerian" << '\n';
		break;
	case Eulerianity::semiEulerian:
		cout << "semi eulerian" << '\n';
		break;
	case Eulerianity::eulerian:
		cout << " eulerian" << '\n';
		break;
	default:
		break;
	}

	//show path
	auto path = eulerFinder.FindPath(eulerianity);
	if(path.size() > 1)
	{
		int coloredConnections = int(m_TotalTime) % (path.size());
		for(int index{ int(path.size()-2 )}; index >= 0 ; --index)
		{
			auto connection = M_PGraph2D->GetConnection(path[index]->GetIndex(), path[index + 1]->GetIndex());
			connection->SetColor(coloredConnections > 0 ? Color{ 1.f,0.f,0.f } : Color{ 1.f,1.f,1.f });
			connection = M_PGraph2D->GetConnection(path[index+1]->GetIndex(), path[index]->GetIndex());
			connection->SetColor(coloredConnections > 0 ? Color{ 1.f,0.f,0.f } : Color{ 1.f,1.f,1.f });
			//cout << connection->GetFrom() << ", " << connection->GetTo() << ", " << coloredConnections <<", " << connection << ", "<< (coloredConnections > 0) << '\n';
			--coloredConnections;
		}
	}
	else
	{
		for(auto node : M_PGraph2D->GetAllNodes())
		{
			for(auto connection : M_PGraph2D->GetNodeConnections(node))
			{
				connection->SetColor(Color{});
			}
		}
	}

	//reveal odd nodes
	for(auto node : M_PGraph2D->GetAllNodes())
	{
		if(M_PGraph2D->GetNodeConnections(node).size() & 1 )
		{
			node->SetColor(Color{ 1.f,0,0 });
		}
		else
		{
			node->SetColor(Color{ 1.f,1.f,1.f });
		}
	}
	
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
	

}

void App_GraphTheory::Render(float deltaTime) const
{
	m_Graphrenderer.RenderGraph(M_PGraph2D, true, true);
}
