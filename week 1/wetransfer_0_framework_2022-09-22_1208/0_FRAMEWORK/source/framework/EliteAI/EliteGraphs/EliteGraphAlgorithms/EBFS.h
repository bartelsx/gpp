namespace Elite
{
    template <class T_NodeType, class T_ConnectionType>
    class BFS
    {
    public:
        BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

        std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
    private:
        IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
    };

    template <class T_NodeType, class T_ConnectionType>
    BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
        : m_pGraph(pGraph)
    {

    }

    template <class T_NodeType, class T_ConnectionType>
    std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
    {
        std::queue<T_NodeType*> openList{};
        std::map<T_NodeType*, T_NodeType*> closedList{};

        openList.push(pStartNode);

        while (openList.empty() == false)
        {
            T_NodeType* pCurrenNode = openList.front();
            openList.pop();

            if (pCurrenNode == pDestinationNode)
            {
                break;
            }

            for (auto& connection : m_pGraph->GetNodeConnections(pCurrenNode))
            {
                T_NodeType* pNextNode = m_pGraph->GetNode(connection->GetTo());

                //Check if we already visited this node
                if (closedList.find(pNextNode) == closedList.end())
                {
                    openList.push(pNextNode);
                    closedList[pNextNode] = pCurrenNode;
                }
            }
        }

        //Check if we reached the destination
        if (closedList.find(pDestinationNode) == closedList.end())
        {
            return std::vector<T_NodeType*>();
        }

        //Backtracking
        std::vector<T_NodeType*> path{};

        //Go through the closedList from destination -> start
        T_NodeType* pCurrentNode = pDestinationNode;
        while (pCurrentNode != pStartNode)
        {
            path.push_back(pCurrentNode);
            pCurrentNode = closedList[pCurrentNode];
        }

        path.push_back(pStartNode);

        std::reverse(path.begin(), path.end());

        return path;
    }
}