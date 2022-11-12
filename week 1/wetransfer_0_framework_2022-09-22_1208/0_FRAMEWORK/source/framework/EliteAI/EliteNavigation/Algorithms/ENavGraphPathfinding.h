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
