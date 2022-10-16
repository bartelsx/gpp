#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	
	float cellWidth = m_SpaceWidth / m_NrOfCols;
	float cellHeight = m_SpaceHeight / m_NrOfRows;
	for (int r = 0 ; r< m_NrOfRows; ++r)
	{
		for (int c = 0; c < m_NrOfCols; ++c)
		{
			m_Cells.push_back(Cell{ c * cellWidth,r * cellHeight,cellWidth, cellHeight });
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	int index = PositionToIndex(agent->GetPosition());
	m_Cells[index].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	int oldIndex = PositionToIndex(oldPos);
	int newIndex = PositionToIndex(agent->GetPosition());
	if(oldIndex != newIndex)
	{
		m_Cells[oldIndex].agents.remove(agent);
		m_Cells[newIndex].agents.push_back(agent);
	}
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	m_NrOfNeighbors = 0;
	float cellWidth = m_SpaceWidth / m_NrOfCols;
	float cellHeight = m_SpaceHeight / m_NrOfRows;
	int rb = Elite::Clamp( int((agent->GetPosition().x - queryRadius)/cellWidth),0,m_NrOfRows-1);
	int re = Elite::Clamp( int((agent->GetPosition().x + queryRadius)/cellWidth),0,m_NrOfRows-1);
	int cb = Elite::Clamp( int((agent->GetPosition().y - queryRadius)/cellHeight),0,m_NrOfCols-1);
	int ce = Elite::Clamp( int((agent->GetPosition().y + queryRadius)/cellHeight),0,m_NrOfCols-1);
	for(int r = rb; r<= re; ++r)
	{
		for(int c = cb; c<= ce ; ++c)
		{
			int index = r * m_NrOfCols + c;
			for (SteeringAgent* pAgent : m_Cells[index].agents)
			{
				if (agent != pAgent)
				{
					Elite::Vector2 ToAgent{ pAgent->GetPosition() - pAgent->GetPosition() };
					if (ToAgent.Magnitude() < queryRadius)
					{
						m_Neighbors.push_back(pAgent);
						++m_NrOfNeighbors;
					}
				}
			}
		}
	}
	
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	float offset = 0.5f;
	for (int index = 0; index < m_Cells.size();++index)
	{
		std::vector<Elite::Vector2> points = m_Cells[index].GetRectPoints();
		auto polygon = Elite::Polygon{ points };
		DEBUGRENDERER2D->DrawPolygon(&polygon,{1,0,0});
		auto number = std::to_string( m_Cells[index].agents.size());
	
		DEBUGRENDERER2D->DrawString(m_Cells[index].boundingBox.bottomLeft + Elite::Vector2{ offset,m_Cells[index].boundingBox.height-offset }, number.c_str());
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int column = int(pos.x/(m_SpaceWidth / m_NrOfCols));
	if(column >= m_NrOfCols)
	{
		column = m_NrOfCols - 1;
	}
	int row = int(pos.y / (m_SpaceHeight / m_NrOfRows));
	if (row >= m_NrOfRows)
	{
		row = m_NrOfRows - 1;
	}
	return row * m_NrOfCols+column;
}