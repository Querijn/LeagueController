#include <memory_helper/instruction.hpp>

#include <capstone/capstone.h>
#include <cstring>
#include <cassert>
#include <vector>
#include <memory>

namespace MemoryHelper
{
	class InstructionSet
	{
	public:
		InstructionSet(cs_insn* instruction, size_t count) :
			m_set(instruction),
			m_count(count),
			m_referenceCount(count)
		{}
		~InstructionSet() { Release(); }

	private:
		InstructionSet(InstructionSet&) = delete;

		friend class Instruction;

		void IncrementRefCount()
		{
			assert(m_referenceCount != 0 && m_set != nullptr);
			m_referenceCount++;
		}

		void DecrementRefCount()
		{
			m_referenceCount--;
			if (m_referenceCount == 0)
				Release();
		}

		void Release()
		{
			if (m_set)
			{
				cs_free(m_set, m_count);
				m_set = nullptr;
			}

			m_count = 0;
			m_referenceCount = 0;
		}

		cs_insn* m_set;
		size_t m_count;
		size_t m_referenceCount;
	};

	struct GlobalData
	{
		csh handle = 0;
		bool initialised = false;
		std::vector<InstructionSet> sets;
	};

	GlobalData& GetData()
	{
		static GlobalData data;
		return data;
	}

	static void InitGlobal()
	{
		if (GetData().initialised)
			return;

		cs_opt_mem setup;

		setup.malloc = malloc;
		setup.calloc = calloc;
		setup.realloc = realloc;
		setup.free = free;
		setup.vsnprintf = vsnprintf;

		auto err = cs_option(0, CS_OPT_MEM, (size_t)&setup);
		assert(err == CS_ERR_OK);

		err = cs_open(CS_ARCH_X86, CS_MODE_32, &GetData().handle);
		assert(err == CS_ERR_OK);
		
		err = cs_option(GetData().handle, CS_OPT_DETAIL, true);
		assert(err == CS_ERR_OK);
	}

	Instruction::Instruction(cs_insn* instruction, std::shared_ptr<InstructionSet> set) :
		m_instruction(instruction),
		m_set(set)
	{

	}

	Instruction::Instruction(Instruction& other) :
		m_instruction(other.m_instruction),
		m_set(other.m_set)
	{
	}

	Instruction::~Instruction()
	{
		m_instruction = nullptr;
	}
	
	std::vector<Instruction> Instruction::FromPattern(const std::initializer_list<u8>& bytes, int inputCount, Address baseAddress)
	{
		return FromPattern(bytes.begin(), bytes.size(), inputCount, baseAddress);
	}

	std::vector<Instruction> Instruction::FromPattern(const u8* bytes, int byteCount, int inputCount, Address baseAddress)
	{
		InitGlobal();

		std::vector<Instruction> results;

		cs_insn* first;
		size_t count = cs_disasm(GetData().handle, bytes, byteCount, (Address)bytes - baseAddress, inputCount < 0 ? 0 : inputCount, & first);
		if (count > 0)
		{
			auto setPtr = std::make_shared<InstructionSet>(first, count);
			
			struct Enabler : Instruction { Enabler(cs_insn* i, std::shared_ptr<InstructionSet>& set) : Instruction(i, set) {} };
			for (size_t i = 0; i < count; i++)
				results.push_back(Enabler(&first[i], setPtr));
		}

		return results;
	}

	const cs_x86_op* Instruction::GetOperands() const
	{
		if (m_instruction->detail->x86.op_count == 0)
			return nullptr;
		return m_instruction->detail->x86.operands;
	}

	size_t Instruction::GetOperandCount() const
	{
		return m_instruction->detail->x86.op_count;
	}

	const cs_insn* Instruction::GetFromCapstone() const
	{
		return m_instruction;
	}
}
