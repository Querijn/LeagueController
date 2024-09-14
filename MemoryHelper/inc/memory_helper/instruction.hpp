#pragma once

#include <spek/util/types.hpp>
#include <memory_helper/address.hpp>

#include <cstddef>
#include <memory>
#include <vector>
#include <initializer_list>

struct cs_insn;
struct cs_x86_op;
namespace MemoryHelper
{
	using Address = u64;

	class InstructionSet;
	class Instruction
	{
	public:
#if ENV_64
		static constexpr Address defaultBaseAddress = 0x140000000;
#elif ENV_32
		static constexpr Address defaultBaseAddress = 0x400000;
#endif

		Instruction(Instruction& other);
		~Instruction();

		static std::vector<Instruction> FromPattern(const std::initializer_list<u8>& bytes, int count = -1, Address baseAddress = defaultBaseAddress);
		static std::vector<Instruction> FromPattern(const u8* bytes, int byteCount, int count = -1, Address baseAddress = defaultBaseAddress);

		const cs_insn* GetFromCapstone() const;
		const cs_x86_op* GetOperands() const;
		size_t GetOperandCount() const;

	private:
		Instruction(cs_insn* instruction, std::shared_ptr<InstructionSet> set);

		cs_insn* m_instruction;
		std::shared_ptr<InstructionSet> m_set;
	};
}
