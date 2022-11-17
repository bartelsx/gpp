#ifndef AGARIO_GAME_APPLICATION_H
#define AGARIO_GAME_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"

class AgarioFood;
class AgarioAgent;
class AgarioContactListener;

class App_AgarioGame final : public IApp
{
public:
	App_AgarioGame();
	~App_AgarioGame();

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;
private:
	float m_TrimWorldSize = 100.f;
	const int m_AmountOfAgents{ 30 };
	std::vector<AgarioAgent*> m_pAgentVec{};

	AgarioAgent* m_pCustomAgent = nullptr;

	const int m_AmountOfFood{ 40 };
	const float m_FoodSpawnDelay{ 2.f };
	float m_TimeSinceLastFoodSpawn{ 0.f };
	std::vector<AgarioFood*> m_pFoodVec{};

	AgarioContactListener* m_pContactListener = nullptr;
	bool m_GameOver = false;

	std::vector<Elite::FSMState*> m_pStates{};
	std::vector<Elite::FSMCondition*> m_pConditions{};

private:	
	template<class T_AgarioType>
	void UpdateAgarioEntities(std::vector<T_AgarioType*>& entities, float deltaTime);

	Elite::Blackboard* CreateBlackboard(AgarioAgent* a);
	void UpdateImGui();
private:
	//C++ make the class non-copyable
	App_AgarioGame(const App_AgarioGame&) {};
	App_AgarioGame& operator=(const App_AgarioGame&) {};
};

template<class T_AgarioType>
inline void App_AgarioGame::UpdateAgarioEntities(std::vector<T_AgarioType*>&entities, float deltaTime)
{
	for (auto& e : entities)
	{
		e->Update(deltaTime);

	
		auto agent = dynamic_cast<AgarioAgent*>(e);
		if (agent)
		{
			//Trim agent to world bounds
			agent->TrimToWorld(m_TrimWorldSize, false);
		}
			
		if (e->CanBeDestroyed())
			SAFE_DELETE(e);
	}

	auto toRemoveEntityIt = std::remove_if(entities.begin(), entities.end(),
		[](T_AgarioType* e) {return e == nullptr; });
	if (toRemoveEntityIt != entities.end())
	{
		entities.erase(toRemoveEntityIt, entities.end());
	}
}

#endif
