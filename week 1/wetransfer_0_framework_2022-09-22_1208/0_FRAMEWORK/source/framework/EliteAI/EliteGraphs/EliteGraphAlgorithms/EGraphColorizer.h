#pragma once


template <class T_NodeType, class T_ConnectionType>
class EGraphColorizer
{
public:
	void Colorize(Elite::IGraph<T_NodeType, T_ConnectionType>* pGraph)
	{
		std::list<Elite::Color> usedColors = std::list<Elite::Color>{};

		for(Elite::GraphNode2D* pNode : pGraph->GetAllNodes())
		{
			auto allowedColors = usedColors;
			for(auto connection : pGraph->GetNodeConnections(pNode))
			{
				auto otherNode = pGraph->GetNode(connection->GetTo());
				auto otherColor = otherNode->GetColor();

				if(otherColor.r  != 0 || otherColor.g != 0 || otherColor.b != 0)
				{
					allowedColors.remove(otherColor);
				}
			}
			if(allowedColors.size() > 0 )
			{
				pNode->SetColor(allowedColors.front());
			}
			else
			{
				auto color = m_Palette[usedColors.size()];
				pNode->SetColor(color);
				usedColors.push_back(color);
			}
		}
	}

private:
	Elite::Color m_Palette[7] = {Elite::Color{1.f,0.f,0.f}
		,Elite::Color{0.f,1.f,0.f}
		,Elite::Color{0.f,0.f,1.f}
		,Elite::Color{1.f,1.f,0.f}
		,Elite::Color{1.f,0.f,1.f}
		,Elite::Color{0.f,1.f,1.f}
		,Elite::Color{1.f,1.f,1.f}
	};

};
