#pragma once
#include "Common.h"
#include <atomic>

struct Operation
{
	enum class OpState
	{
		OP_PENDING,
		OP_RUNNING,
		OP_FINISHED,
		OP_ERRORED
	};

	struct Status
	{
	private:
		OpState m_state;
		std::string m_description;
	public:
		OpState State();
		std::string Description();
		Status(OpState state, std::string desc);
	};

	// TODO: Oops, this is a bound member function call.
	//       I guess we need to forward declare the Application class?
	//       Beware thread safety!
	typedef void (Operation::* OpFunc)();

	OpFunc Process;
	std::atomic<Status*> Status;

};