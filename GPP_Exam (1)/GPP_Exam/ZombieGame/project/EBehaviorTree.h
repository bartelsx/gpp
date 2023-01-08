/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EBehaviorTree.h: Implementation of a BehaviorTree and the components of a Behavior Tree
/*=============================================================================*/
#ifndef ELITE_BEHAVIOR_TREE
#define ELITE_BEHAVIOR_TREE

//--- Includes ---
#include <chrono>

#include "EBlackboard.h"
#include "EDecisionMaking.h"

namespace Elite
{
	//-----------------------------------------------------------------
	// BEHAVIOR TREE HELPERS
	//-----------------------------------------------------------------
	enum class BehaviorState
	{
		Failure,
		Success,
		Running
	};

	//-----------------------------------------------------------------
	// BEHAVIOR INTERFACES (BASE)
	//-----------------------------------------------------------------
	class IBehavior
	{
	public:
		IBehavior() = default;
		virtual ~IBehavior() = default;
		virtual BehaviorState Execute(Blackboard* pBlackBoard) = 0;

	protected:
		BehaviorState m_CurrentState = BehaviorState::Failure;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE COMPOSITES (IBehavior)
	//-----------------------------------------------------------------
#pragma region COMPOSITES
	//--- COMPOSITE BASE ---
	class BehaviorComposite : public IBehavior
	{
	public:
		explicit BehaviorComposite(std::vector<IBehavior*> childBehaviors)
		{ m_ChildBehaviors = childBehaviors;	}
		virtual ~BehaviorComposite()
		{
			for (auto pb : m_ChildBehaviors)
				SAFE_DELETE(pb);
			m_ChildBehaviors.clear();
		}

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override = 0;

	protected:
		std::vector<IBehavior*> m_ChildBehaviors = {};
	};

	//--- SELECTOR ---
	class BehaviorSelector : public BehaviorComposite
	{
	public:
		explicit BehaviorSelector(std::vector<IBehavior*> childBehaviors) :
			BehaviorComposite(childBehaviors) {}
		virtual ~BehaviorSelector() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	//--- SEQUENCE ---
	class BehaviorSequence : public BehaviorComposite
	{
	public:
		explicit BehaviorSequence(std::vector<IBehavior*> childBehaviors) :
			BehaviorComposite(childBehaviors) {}
		virtual ~BehaviorSequence() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	//--- PARTIAL SEQUENCE ---
	class BehaviorPartialSequence : public BehaviorSequence
	{
	public:
		explicit BehaviorPartialSequence(std::vector<IBehavior*> childBehaviors)
			: BehaviorSequence(childBehaviors) {}
		virtual ~BehaviorPartialSequence() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		unsigned int m_CurrentBehaviorIndex = 0;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE DECORATORS (IBehavior)
	//-----------------------------------------------------------------
	class BehaviorDecorator : public IBehavior
	{
	public:
		explicit BehaviorDecorator(IBehavior* child) : m_Child(child)
		{}

		~BehaviorDecorator()
		{
			SAFE_DELETE(m_Child);
		}

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override = 0;

	protected:
		IBehavior* m_Child;
	};

	//Repeats childBehavior until the childbehavior returns Failure
	class BehaviorRepeat : public BehaviorDecorator
	{
	public:
		explicit BehaviorRepeat(IBehavior* childBehaviors) :
			BehaviorDecorator(childBehaviors) {}

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	class BehaviorMaskFailure : public BehaviorDecorator
	{
	public:
		explicit BehaviorMaskFailure(IBehavior* childBehaviors) :
			BehaviorDecorator(childBehaviors) {}

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	//Inverts the result of childBehavior (returns Failure instead of Success and vice versa)
	class BehaviorInvert: public BehaviorDecorator
	{
	public:
		explicit BehaviorInvert(IBehavior* childBehaviors) :
			BehaviorDecorator(childBehaviors) {}

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};


#pragma endregion

	//-----------------------------------------------------------------
	// BEHAVIOR TREE CONDITIONAL (IBehavior)
	//-----------------------------------------------------------------
	class BehaviorConditional : public IBehavior
	{
	public:
		explicit BehaviorConditional(std::function<bool(Blackboard*)> fp) : m_fpConditional(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
	};

	class BehaviorInvertConditional : public BehaviorInvert
	{
	public:
		explicit BehaviorInvertConditional(std::function<bool(Blackboard*)> fp) : BehaviorInvert(new BehaviorConditional(fp)){};
		//virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	class BehaviorTimedConditional : public IBehavior
	{
	public:
		explicit BehaviorTimedConditional(int timeoutSeconds , std::function<bool(Blackboard*)> fp)
		: m_fpConditional(fp), m_timeoutSeconds(timeoutSeconds), m_startTime(-1), m_isRunning(false)
		{}

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
		int m_timeoutSeconds;
		time_t m_startTime;
		bool m_isRunning;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE ACTION (IBehavior)
	//-----------------------------------------------------------------
	class BehaviorAction : public IBehavior
	{
	public:
		explicit BehaviorAction(std::function<BehaviorState(Blackboard*)> fp) : m_fpAction(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		std::function<BehaviorState(Blackboard*)> m_fpAction = nullptr;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE (BASE)
	//-----------------------------------------------------------------
	class BehaviorTree final : public Elite::IDecisionMaking
	{
	public:
		explicit BehaviorTree(Blackboard* pBlackBoard, IBehavior* pRootBehavior)
			: m_pBlackBoard(pBlackBoard), m_pRootBehavior(pRootBehavior) {};
		~BehaviorTree()
		{
			SAFE_DELETE(m_pRootBehavior);
			SAFE_DELETE(m_pBlackBoard); //Takes ownership of passed blackboard!
		};

		virtual void Update(float deltaTime) override
		{
			if (m_pRootBehavior == nullptr)
			{
				m_CurrentState = BehaviorState::Failure;
				return;
			}
				
			m_CurrentState = m_pRootBehavior->Execute(m_pBlackBoard);
		}
		Blackboard* GetBlackboard() const
		{ return m_pBlackBoard;	}

	private:
		BehaviorState m_CurrentState = BehaviorState::Failure;
		Blackboard* m_pBlackBoard = nullptr;
		IBehavior* m_pRootBehavior = nullptr;
	};
}
#endif