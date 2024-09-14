#include <league_internals/offsets.hpp>
#include <league_internals/offset_loader.hpp>
#include <league_internals/game_object.hpp>

#include <league_internals/task_hud_manager.hpp>
#include <league_internals/task_rendering_flag.hpp>
#include <league_internals/task_spellbook.hpp>
#include <league_internals/task_gameobject.hpp>

#include <spek/util/duration.hpp>
#include <league_controller/json.hpp>
#include <league_controller/launcher_data.hpp>
#include <league_controller/connection.hpp>
#include <league_controller/settings.hpp>

#if LEAGUEHACKS_DEBUG_LOG
#include <game_overlay/overlay.hpp>
#include <game_overlay/log.hpp>
#endif

#include <memory_helper/instruction.hpp>
#include <memory_helper/helper_functions.hpp>

#include <capstone/capstone.h>
#include <Windows.h>
#include <psapi.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <memory>
#include <string>
#include <stdexcept>
#include <thread>
#include <mutex>

#pragma comment(lib, "Version.lib")

using namespace MemoryHelper;
namespace fs = std::filesystem;

namespace LeagueController
{
#if LEAGUEHACKS_DEBUG_LOG
	extern GameOverlay::LogCategory g_offsetLog;
#else
	extern int g_offsetLog;

#define GameOverlay_AssertF(Category, Cond, Format, ...) assert(Cond);
#define GameOverlay_Assert(Category, Cond)				 assert(Cond);

#define GameOverlay_WaitForDebugger(a)
#define GameOverlay_LogWarning
#define GameOverlay_LogDebug
#define GameOverlay_LogDebugOnce
#define GameOverlay_Stringify
	namespace GameOverlay { void ForceFlush() {} }

#endif

	struct
	{
		HudManagerTask hudManager;
		RenderingFlagTask renderingFlag;
		SpellBookTask spellBook;
		GameObjectTask gameObject;
	} g_tasks;
	
	extern std::unique_ptr<LeagueController::Connection<LeagueController::LauncherData>> g_sharedMemory;

	static const Spek::Duration g_retryDelay = 5_sec; // Only retry getting data every 5 sec.

	static constexpr std::initializer_list<u8>	gameTimePattern = { 0xF3, 0x0F, 0x5C, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0x0F, 0x2F, 0xC1, 0xF3 };
	static constexpr u8							gameTimeIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	heroManagerPattern = { 0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0x3B, 0xC3, 0x8B, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, 0x0F, 0x44, 0xC1, 0xA3 }; // A1 ? ? ? ? 3B C3 8B 15 ? ? ? ? 0F 44 C1 A3
	static constexpr u8							heroManagerIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	objManagerPattern = { 0x89, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x57, 0xC7, 0x06, 0xCC, 0xCC, 0xCC, 0xCC, 0x66, 0xC7, 0x46, 0x04 };
	static constexpr u8							objManagerIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	attackableManagerPattern = { 0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0xFF, 0xB4, 0x24, 0xCC, 0xCC, 0xCC, 0xCC, 0xF3, 0x0F, 0x10, 0x44, 0x24, 0xCC, 0xF3 }; // A1 ? ? ? ? FF B4 24 ? ? ? ? F3 0F 10 44 24 ? F3
	static constexpr u8							attackableManagerIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	initRendererFuncPattern = { 0xEB, 0xCC, 0x33, 0xCC, 0xFF, 0x35, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC }; // EB ? 33 ? FF 35 ? ? ? ? 8B 0D ? ? ? ?
	static constexpr u8							initRendererFuncIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	attackRangePattern = { 0xD9, 0x83, 0xCC, 0xCC, 0xCC, 0xCC, 0x5F, 0x5E, 0x5B, 0x83, 0xC4, 0x08, 0xC3 }; // D9 83 ? ? ? ? 5F 5E 5B 83 C4 08 C3
	static constexpr u8							attackRangeIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	underMouseObjectPattern = { 0x8B, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0xC7, 0x04, 0x24, 0xCC, 0xCC, 0xCC, 0xCC, 0xFF, 0x74, 0x24, 0x58, 0xFF, 0x74, 0x24, 0x58, 0xFF, 0x74, 0x24, 0x58, 0xFF, 0x74, 0x24, 0x58, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC }; // 8B 0D ? ? ? ? C7 04 24 ? ? ? ? FF 74 24 58 FF 74 24 58 FF 74 24 58 FF 74 24 58 E8 ? ? ? ? 
	static constexpr u8							underMouseObjectIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	viewProjPattern = { 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0xB9, 0xCC, 0xCC, 0xCC, 0xCC, 0xE9, 0xCC, 0xCC, 0xCC, 0xCC }; // B9 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E9 ? ? ? ? CC CC
	static constexpr u8							viewProjIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	buyItemPattern = { 0x81, 0xEC, 0xCC, 0xCC, 0xCC, 0xCC, 0x55, 0x56, 0x8B, 0xB4, 0x24, 0xCC, 0xCC, 0xCC, 0xCC, 0x57, 0x8B, 0xF9, 0x8B, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0x56, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0xE8, 0x85, 0xED, 0x75, 0x0E }; // 81 EC ? ? ? ? 55 56 8B B4 24 ? ? ? ? 57 8B F9 8B 0D ? ? ? ? 56 E8 ? ? ? ? 8B E8 85 ED 75 0E
	static constexpr u8							buyItemIgnore = 0xCC;

	static constexpr std::initializer_list<u8>	guiMenuPattern = { 0xC6, 0x47, 0x0F, 0x00, 0x8B, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC }; // C6 47 0F 00 8B 0D ? ? ? ? 
	static constexpr u8							guiMenuIgnore = 0xCC;

	struct
	{
		std::string workingDirectory;
		HMODULE base;
		Address baseAddress;
		HANDLE process;
		MODULEINFO mod = {};
		bool success;
		bool hasWrittenToFile;

		std::mutex workerMutex;
		std::thread worker;
		bool workerActive;
		bool shouldWork;

		bool isFirstAttempt = true;
		Spek::Duration lastAttempt = 0_ms;
	} g_offsetLoaderData;

	Address DwordPtrOpStrToAddress(const cs_insn* e, bool useEnd)
	{
		Leacon_Profiler;
		auto func = [e, useEnd](char c) {
			if (useEnd)
				return strrchr(e->op_str, c);
			return strchr(e->op_str, c);
		};

		// Since the imm value doesn't work, we have to decode the string op_str
		const char* begin = func('['); // mov ecx, dword ptr>>[<<0x33f68d4]
		const char* end = func(']'); // mov ecx, dword ptr[0x33f68d4>>]<<
		GameOverlay_Assert(g_offsetLog, begin && end);

		std::string addressAsString(begin + 1, end); char* p; // Contains 0x33f68d4
		return strtol(addressAsString.c_str(), &p, 16);
	}
		
	void* GetByOperand(Address& relativeAddress, const std::initializer_list<u8>& pattern, u8 ignoreCase, int initialInstructionIndex, int initialOperandIndex, const MODULEINFO& mod, Address inBegin, Address inEnd, Address* outFoundLocation)
	{
		Leacon_Profiler;
		if (relativeAddress != 0)
		{
			return (u8*)mod.lpBaseOfDll + relativeAddress;
		}

		u8* current = (u8*)inBegin;
		u8* end = (u8*)inEnd == 0 ? current + mod.SizeOfImage : (u8*)inEnd;
		while (true)
		{
			// Copy these numbers (since we're changing them)
			int instructionIndex = initialInstructionIndex;
			int operandIndex = initialOperandIndex;

			u8* code = (u8*)FindArrayOfBytesRPM(current, end, pattern, ignoreCase);
			if (code == nullptr)
			{
				GameOverlay_LogDebug(g_offsetLog, "GetByOperand failed: Unable to find array of bytes: %s\n", PrintBytePattern(pattern, ignoreCase).c_str());
				return 0;
			}

			auto instructions = ::Instruction::FromPattern(code, pattern.size(), -1, (Address)mod.lpBaseOfDll);

			GameOverlay_LogDebug(g_offsetLog, "GetByOperand found a pattern at: %p\n", code);
			GameOverlay_LogDebug(g_offsetLog, "GetByOperand: %s %s\n", instructions[0].GetFromCapstone()->mnemonic, instructions[0].GetFromCapstone()->op_str);

			int instructionCount = (int)instructions.size();
			while (instructionIndex < 0)
				instructionIndex += instructionCount;

			if (instructionIndex >= instructionCount)
			{
				GameOverlay_LogDebug(g_offsetLog, "GetByOperand is ignoring 0x%p: Not enough instructions. Instruction Count: %u, Needed: %u, Pattern: %s\n", code, instructionCount, instructionIndex, PrintBytePattern(pattern, ignoreCase).c_str());
				current = code + 1;
				continue;
			}

			auto& instruction = instructions[instructionIndex];
			int opCount = (int)instruction.GetOperandCount();
			while (operandIndex < 0)
				operandIndex += opCount;

			if (operandIndex >= opCount)
			{
				GameOverlay_LogDebug(g_offsetLog, "GetByOperand is ignoring 0x%p: Not enough operands. Operand Count: %u, Needed: %u, Pattern: %s\n", code, opCount, operandIndex, PrintBytePattern(pattern, ignoreCase).c_str());
				current = code + 1;
				continue;
			}

			if (outFoundLocation != nullptr)
			{
				*outFoundLocation = (Address)code - (Address)mod.lpBaseOfDll;
			}

			auto* ops = instruction.GetOperands();
			auto operand = ops[operandIndex];
			GameOverlay_LogDebug(g_offsetLog, "GetByOperand: Getting the data for %s %s (type = %d, %x, %x)\n", instructions[0].GetFromCapstone()->mnemonic, instructions[0].GetFromCapstone()->op_str, operand.type, operand.mem.disp, operand.imm);

			Address offset = operand.type == X86_OP_IMM ? operand.imm : operand.mem.disp;
			relativeAddress = offset - (Address)mod.lpBaseOfDll;
			return (void*)offset;
		}

		return nullptr;
	}

	void LoadFromResetMatrices(OffsetData& data, MODULEINFO& mod)
	{
		Leacon_Profiler;
		if (data.viewMatrix == nullptr && data.viewMatrixAddress != 0)
			data.viewMatrix = (glm::mat4*)(data.viewMatrixAddress + (Address)mod.lpBaseOfDll);

		if (data.projMatrix == nullptr && data.projMatrixAddress != 0)
			data.projMatrix = (glm::mat4*)(data.projMatrixAddress + (Address)mod.lpBaseOfDll);

		if (data.viewMatrix != nullptr && data.projMatrix != nullptr)
			return;

		u8* current = (u8*)mod.lpBaseOfDll;
		u8* end = current + mod.SizeOfImage;

		std::vector<u8*> codeLocations;
		size_t patternSize = viewProjPattern.size();
		while (current < end)
		{
			u8* code = (u8*)FindArrayOfBytesRPM(current, end, viewProjPattern, viewProjIgnore);
			if (code == nullptr)
				break;

			current = code + 1;
			codeLocations.push_back(code);
			GameOverlay_LogDebug(g_offsetLog, "LoadFromResetMatrices: 0x%p", code);
		}

		for (u8* codeLoc : codeLocations)
		{
			auto instructions = Instruction::FromPattern(codeLoc, patternSize, (Address)mod.lpBaseOfDll);
			if (instructions.size() != 6)
				continue;

			// mov     ecx, ViewMatrix
			if (strcmp(instructions[0].GetFromCapstone()->mnemonic, "mov") != 0)
				continue;

			// call    MakeIdentity
			if (strcmp(instructions[1].GetFromCapstone()->mnemonic, "call") != 0)
				continue;

			// mov     ecx, ProjMatrix
			if (strcmp(instructions[2].GetFromCapstone()->mnemonic, "mov") != 0)
				continue;

			// call    MakeIdentity
			if (strcmp(instructions[3].GetFromCapstone()->mnemonic, "call") != 0)
				continue;

			// Both calls are to the same function
			const cs_x86_op* callOps1 = instructions[1].GetOperands(); // call MakeIdentity
			const cs_x86_op* callOps2 = instructions[3].GetOperands(); // call MakeIdentity
			if (callOps1->imm != callOps2->imm)
				continue;

			// Don't care about the last two instructions
			// mov     ecx, UnknownItem
			// jmp     UnknownFunction

			auto viewOperands = instructions[0].GetOperands();
			auto projOperands = instructions[2].GetOperands();

			Address viewMatrixAddress = viewOperands[1].imm;
			Address projMatrixAddress = projOperands[1].imm;

			if (projMatrixAddress != viewMatrixAddress + sizeof(glm::mat4))
				continue;

			data.viewMatrixAddress = viewMatrixAddress - (Address)mod.lpBaseOfDll;
			data.projMatrixAddress = projMatrixAddress - (Address)mod.lpBaseOfDll;
			break;
		}

		if (data.viewMatrix == nullptr && data.viewMatrixAddress != 0)
			data.viewMatrix = (glm::mat4*)(data.viewMatrixAddress + (Address)mod.lpBaseOfDll);

		if (data.projMatrix == nullptr && data.projMatrixAddress != 0)
			data.projMatrix = (glm::mat4*)(data.projMatrixAddress + (Address)mod.lpBaseOfDll);
	}

	void LoadFromStopCommand(OffsetData& data, int depth = -1)
	{
		// REDACTED: This function used to contain code that would stop the player from moving, but it has been removed for security reasons.
	}

	void OffsetLoaderThread(OffsetData& data);
	void InitOffsetLoadData(OffsetData& data)
	{
		GameOverlay_Assert(g_offsetLog, g_offsetLoaderData.workerMutex.try_lock() == false);
		if (g_offsetLoaderData.success)
			return;

		const auto& sharedMemoryData = g_sharedMemory->GetData();

		g_offsetLoaderData.workingDirectory = sharedMemoryData.currentWorkingDirectory;
		g_offsetLoaderData.base = GetModuleHandle(nullptr);
		g_offsetLoaderData.baseAddress = (Address)g_offsetLoaderData.base;
		g_offsetLoaderData.process = GetCurrentProcess();
		g_offsetLoaderData.mod = {};
		g_offsetLoaderData.success = GetModuleInformation(g_offsetLoaderData.process, g_offsetLoaderData.base, &g_offsetLoaderData.mod, sizeof(g_offsetLoaderData.mod));
		g_offsetLoaderData.workerActive = true;
		g_offsetLoaderData.shouldWork = false;
		g_offsetLoaderData.worker = std::thread([&data]() { OffsetLoaderThread(data); });
		g_offsetLoaderData.worker.detach();
		GameOverlay_LogDebug(g_offsetLog, "GetModuleInformation %s\n", g_offsetLoaderData.success ? "succeeded" : "failed");
	}

	void OffsetLoaderThread(OffsetData& data)
	{
		Leacon_Profiler;
		while (g_offsetLoaderData.workerActive)
		{
			if (g_offsetLoaderData.shouldWork == false)
			{
				Sleep(100);
				continue;
			}

			Leacon_ProfilerSection("LoadingOffsets");
			std::lock_guard t(g_offsetLoaderData.workerMutex);
			auto start = Spek::GetTimeSinceStart();
			auto&& base = g_offsetLoaderData.base;
			auto&& baseAddress = g_offsetLoaderData.baseAddress;
			auto&& process = g_offsetLoaderData.process;
			auto&& mod = g_offsetLoaderData.mod;
			auto&& success = g_offsetLoaderData.success;

			if (GameObject::attackRangeOffset == 0)
			{
				GameObject::attackRangeOffset = data.attackRangeOffset;
				if (data.attackRangeOffset == 0)
				{
					u8* current = (u8*)base;
					u8* end = current + mod.SizeOfImage;
					while (true)
					{
						u8* code = (u8*)FindArrayOfBytesRPM(current, end, attackRangePattern, attackRangeIgnore);
						if (code == nullptr)
						{
							GameOverlay_LogDebug(g_offsetLog, "FindArrayOfBytesRPM(attackRangeOffset) failed: Unable to find array of bytes: %s\n", PrintBytePattern(attackRangePattern, attackRangeIgnore).c_str());
							break;
						}

						if (code[0] != 0xD9)
						{
							current = code + 1;
							continue;
						}

						GameObject::attackRangeOffset = *(u32*)(code + 2); // D9 83 04 13 00 00 -> 04 13 00 00 (not technically correct, but close enough)
						data.attackRangeOffset = GameObject::attackRangeOffset;
						break;
					}
					GameOverlay_LogDebug(g_offsetLog, "Attack range offset: 0x%x", GameObject::attackRangeOffset);
				}
			}

			if (data.underMouseObject == nullptr)
			{
				data.underMouseObject = (GameObject*)GetByOperand(data.underMouseObjectAddress, underMouseObjectPattern, underMouseObjectIgnore, 0, 1, mod, baseAddress);
				GameOverlay_LogDebug(g_offsetLog, "Under Mouse Object: %p", data.underMouseObject);
			}

			LoadSingleton("guiMenu", data.guiMenu, data.guiMenuAddress, guiMenuPattern, guiMenuIgnore, 1, 1, base, baseAddress, mod);
			LoadSingleton("heroManager", data.heroManager, data.heroManagerAddress, heroManagerPattern, heroManagerIgnore, 2, 1, base, baseAddress, mod);
			LoadSingleton("attackableManager", data.attackableManager, data.attackableManagerAddress, attackableManagerPattern, attackableManagerIgnore, 0, 0, base, baseAddress, mod);

			g_tasks.hudManager.Resolve(data, base, baseAddress, mod);
			g_tasks.renderingFlag.Resolve(data, base, baseAddress, mod);
			g_tasks.spellBook.Resolve(data, base, baseAddress, mod);
			g_tasks.gameObject.Resolve(data, base, baseAddress, mod);

			LoadFromStopCommand(data);
			LoadFromResetMatrices(data, mod);

			auto end = Spek::GetTimeSinceStart();
			GameOverlay_LogDebug(g_offsetLog, "LoadOffsetsInternal took %.1f ms (%s)", (end - start).ToSecF64() * 1000, AreOffsetsLoaded() ? "done" : "not ready yet");
			GameOverlay::ForceFlush();

			// Save data to settings
			AddressData& settingsAddresses = GetSettings().addresses;
			if (data != settingsAddresses)
			{
				settingsAddresses.ForEachNamedAddress([data](AddressData::AddressName name, Address& address)
				{
					const Offset* foundOffset = data.GetByName(name);
					if (foundOffset && *foundOffset != 0)
						address = *foundOffset;
				});
				
			}

			g_offsetLoaderData.shouldWork = false;
		}
	}
	
	void FetchGameTime(OffsetData& data)
	{
		if (data.gameTime)
			return;

		std::lock_guard t(g_offsetLoaderData.workerMutex);
		InitOffsetLoadData(data);

		auto&& base = g_offsetLoaderData.base;
		auto&& baseAddress = g_offsetLoaderData.baseAddress;
		auto&& process = g_offsetLoaderData.process;
		auto&& mod = g_offsetLoaderData.mod;
		auto&& success = g_offsetLoaderData.success;

		if (success == false)
			return;

		data.gameTime = (f32*)GetByOperand(data.gameTimeAddress, gameTimePattern, gameTimeIgnore, 0, 1, mod, baseAddress);
		GameOverlay_LogDebug(g_offsetLog, "GetByOperand(gameTime) %s (ptr: %x, base: %p, offset: %p)\n",
			data.gameTime != nullptr ? "succeeded" : "failed",
			(Address)data.gameTime - baseAddress,
			base,
			data.gameTime);
		GameOverlay::ForceFlush();
	}

	void LoadOffsetsInternal(OffsetData& data)
	{
		Leacon_Profiler;

		GetSettings().patchVerified = AreOffsetsLoaded();
		if (AreOffsetsLoaded() || AreSettingsLoaded() == false)
			return;

		// Check if the settings have changed (only copy data that isn't null or offset 0)
		const AddressData& settingsAddresses = GetSettings().addresses;
		if (data != settingsAddresses)
		{
			data.ForEachNamedAddress([settingsAddresses](AddressData::AddressName name, Address& address)
			{
				const Offset* settingsOffset = settingsAddresses.GetByName(name);
				if (settingsOffset && *settingsOffset != 0)
					address = *settingsOffset;
			});
		}

		if (g_sharedMemory == nullptr) // This probably means we're running as an app outside of League of Legends, no offsets available
			return;

		std::lock_guard t(g_offsetLoaderData.workerMutex);
		InitOffsetLoadData(data);

		auto&& base = g_offsetLoaderData.base;
		auto&& baseAddress = g_offsetLoaderData.baseAddress;
		auto&& process = g_offsetLoaderData.process;
		auto&& mod = g_offsetLoaderData.mod;
		auto&& success = g_offsetLoaderData.success;

		if (success == false)
			return;

		// Only try every x seconds
		auto&& isFirstAttempt = g_offsetLoaderData.isFirstAttempt;
		auto&& lastAttempt = g_offsetLoaderData.lastAttempt;
		if (isFirstAttempt == false && (Spek::GetTimeSinceStart() - lastAttempt) < g_retryDelay)
			return;
		lastAttempt = Spek::GetTimeSinceStart();
		isFirstAttempt = false;

		FetchGameTime(data);

		if (data.gameTime == nullptr || *data.gameTime < 1.0f) // Wait for us to be ingame.
			return;

		// Retry finding our stuff
		g_offsetLoaderData.shouldWork = true;
	}

	bool IsInGameInternal(OffsetData& data)
	{
		Leacon_Profiler;
		static bool isInGame = false;

		if (isInGame)
			return true;

		if (HasGameTime() == false)
			FetchGameTime(data);

		if (HasGameTime() && GetGameTime() > 0.1f)
		{
			isInGame = true;
			return true;
		}

		return isInGame;
	}

	void* FindArrayOfBytes(void* startAddress, void* endAddress, const std::vector<u8>& pattern, u8 ignoreCase)
	{
		Leacon_Profiler;
		u8* begin = (u8*)startAddress;
		u8* end = (u8*)endAddress;

		int patternIterator = 0;
		for (int i = 0; begin + i < end; i++)
		{
			if (begin[i] == pattern[patternIterator] || pattern[patternIterator] == ignoreCase)
			{
				patternIterator++;

				if (patternIterator == pattern.size())
					return begin + i - patternIterator + 1;
			}
			else
			{
				i -= patternIterator;
				patternIterator = 0;
			}
		}

		return 0;
	}
	
	bool LoadSingletonInternal(const char* name, void*& dest, Address& address, const std::initializer_list<u8>& pattern, u8 ignoreCase, int instructionIndex, int operandIndex, HMODULE base, Address baseAddress, const MODULEINFO& mod, Address* outFoundLocation)
	{
		Leacon_Profiler;
		void* offset = nullptr;
		if (address == 0 || outFoundLocation != nullptr)
			offset = GetByOperand(address, pattern, ignoreCase, instructionIndex, operandIndex, mod, baseAddress, 0, outFoundLocation);
		else
			offset = (u8*)address + baseAddress;

		if (dest != nullptr)
			return true;

#if !LEACON_SUBMISSION
		GameOverlay::ForceFlush();
#endif

		dest = offset ? *(void**)offset : nullptr;

		GameOverlay_LogDebug(g_offsetLog, "LoadSingleton(%s) %s (Singleton: %x, Base Image: %p, offset: %p)\n",
			name,
			dest != nullptr ? "succeeded" : "failed",
			address,
			base,
			dest);
		GameOverlay::ForceFlush();
		return dest != nullptr;
	}

	u32 GetImageBase()
	{
		return g_offsetLoaderData.baseAddress;
	}
}