#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			auto pStartTriangle =pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
			auto pEndTriangle =pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);

			NavGraphNode* pStartNode{};
			NavGraphNode* pEndNode{};
			
			if (pStartTriangle != nullptr && pEndTriangle != nullptr)
			{
				if (pStartTriangle == pEndTriangle)
				{
					finalPath.push_back(endPos);
				}
				else
				{
					std::shared_ptr<IGraph<NavGraphNode, GraphConnection2D>> pClone = pNavGraph->Clone();

					for (auto lineIdx : pStartTriangle->metaData.IndexLines)
					{
						int nodeIdx = pNavGraph->GetNodeIdxFromLineIdx(lineIdx);
						if (nodeIdx != invalid_node_index)
						{
							auto pNode = pClone->GetNode(nodeIdx);
							pStartNode = new NavGraphNode{ pClone->GetNextFreeNodeIndex(), -1, startPos };
							int startNodeIdx = pClone->AddNode(pStartNode);
							pClone->AddConnection(new GraphConnection2D{startNodeIdx, nodeIdx, Distance(startPos, pNode->GetPosition())});
						}
					}

					for (auto lineIdx : pEndTriangle->metaData.IndexLines)
					{
						int nodeIdx = pNavGraph->GetNodeIdxFromLineIdx(lineIdx);
						if (nodeIdx != invalid_node_index)
						{
							auto pNode = pClone->GetNode(nodeIdx);
							pEndNode = new NavGraphNode{ pClone->GetNextFreeNodeIndex(), -1, endPos };
							int endNodeIdx = pClone->AddNode(pEndNode);
							pClone->AddConnection(new GraphConnection2D{endNodeIdx, nodeIdx, Distance(endPos, pNode->GetPosition()) });
						}
					}

					auto aStar = AStar<NavGraphNode, GraphConnection2D>(&(*pClone), Elite::HeuristicFunctions::Euclidean);
					auto path{ aStar.FindPath(pStartNode, pEndNode) };

					debugNodePositions.clear();
					for (auto pNode : path)
					{
						debugNodePositions.push_back(pNode->GetPosition());
						finalPath.push_back(pNode->GetPosition());
					}
				}
			}

			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			
			//Create extra node for the Start Node (Agent's position
			
			//Create extra node for the endNode
			
			//Run A star on new graph
			
			//OPTIONAL BUT ADVICED: Debug Visualisation

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			//m_Portals = SSFA::FindPortals(nodes, m_pNavGraph->GetNavMeshPolygon());
			//finalPath = SSFA::OptimizePortals(m_Portals);

			return finalPath;
		}
	};
}
