#pragma once

#include <league_internals/offsets.hpp>
#include <memory_helper/address.hpp>

#include <vector>
#include <memory>
#include <functional>

#include <Windows.h>
#include <Psapi.h>

namespace LeagueController
{
	using Address = MemoryHelper::Address;
	enum class ApproachState
	{
		Succeeded = 0,
		Failed,
		InProgress,
		NotStarted
	};

	class BaseApproach
	{
	public:
		virtual ~BaseApproach() = default;

		virtual ApproachState Resolve(OffsetData& offsetData, HMODULE base, Address baseAddress, const MODULEINFO& mod) = 0;
		ApproachState InternalResolve(OffsetData& offsetData, HMODULE base, Address baseAddress, const MODULEINFO& mod);

		virtual bool Test();

	protected:
		ApproachState m_state = ApproachState::NotStarted;
	};

	class FunctionApproach : public BaseApproach
	{
	public:
		using Callback = std::function<ApproachState(OffsetData& offsetData, HMODULE base, Address baseAddress, const MODULEINFO& mod)>;
		FunctionApproach(Callback function);

		ApproachState Resolve(OffsetData& offsetData, HMODULE base, Address baseAddress, const MODULEINFO& mod) override;

	private:
		Callback m_function;
	};

	class BaseTask
	{
	public:
		ApproachState Resolve(OffsetData& offsetData, HMODULE base, Address baseAddress, const MODULEINFO& mod);

	protected:
		std::vector<std::unique_ptr<BaseApproach>> m_approaches;
	};
}