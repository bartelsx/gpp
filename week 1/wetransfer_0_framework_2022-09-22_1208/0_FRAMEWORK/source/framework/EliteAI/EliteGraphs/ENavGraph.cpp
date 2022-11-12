#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	//1.A
	for (auto pLine : m_pNavMeshPolygon->GetLines())
	{
		auto triangles = m_pNavMeshPolygon->GetTrianglesFromLineIndex(pLine->index);
		if (triangles.size() > 1)
		{
			auto pNode = new NavGraphNode{ GetNextFreeNodeIndex(), pLine->index, (pLine->p1 + pLine->p2) / 2};
			AddNode(pNode);
		}
	}

	//1.B
	for (auto pTriangle : m_pNavMeshPolygon->GetTriangles())
	{
		std::vector<int> nodeIndices{};
		for (auto lineIdx : pTriangle->metaData.IndexLines)
		{
			int nodeIdx{ GetNodeIdxFromLineIdx(lineIdx) };
			if (nodeIdx != invalid_node_index)
			{
				nodeIndices.push_back(nodeIdx);
			}
		}

		if (nodeIndices.size() >= 2)
		{
			auto pCon = new GraphConnection2D{ nodeIndices[0], nodeIndices[1] };
			AddConnection(pCon);

			if (nodeIndices.size() == 3)
			{
				pCon = new GraphConnection2D{ nodeIndices[1], nodeIndices[2] };
				AddConnection(pCon);
				pCon = new GraphConnection2D{ nodeIndices[2], nodeIndices[0] };
				AddConnection(pCon);
			}
		}
	}

	//1.C
	SetConnectionCostsToDistance();
	
	//2. Create connections now that every node is created
	
	//3. Set the connections cost to the actual distance

}

