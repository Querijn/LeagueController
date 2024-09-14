#include <league_controller/settings.hpp>
#include <league_controller/config.hpp>
#include <league_controller/base64.h>

#include <league_controller/json.hpp>
#include <league_controller/profiler.hpp>
#include <vector>
#include <fstream>
#include <filesystem>

#if _WIN32
#include <Windows.h>
#endif

#pragma comment(lib, "Version.lib")

namespace fs = std::filesystem;

namespace LeagueController
{
	template<typename ... Args>
	static std::string string_format(const std::string& format, Args ... args)
	{
		Leacon_Profiler;
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
		auto size = static_cast<size_t>(size_s);
		std::unique_ptr<char[]> buf(new char[size]);
		std::snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}
	
	struct
	{
		SerialisedSettings serialisedSettings;
		Settings settings;
		Settings settingsAtStartup;
		bool settingsLoaded = false;
		bool settingsChanged = false;
	} g_settingsData;

	u32 FNVa1(const char* text, int len = 0)
	{
		Leacon_Profiler;
		if (len == 0) len = strlen(text);

		u32 hash = 0x811C9DC5;
		for (int i = 0; i < len; i++)
		{
			hash ^= text[i];
			hash *= 0x01000193;
		}
		return hash;
	}

	void from_json(const nlohmann::json& j, ControllerSettings& s)
	{
	#define FieldArg(Type, Name, DefaultValue) if (j.contains(#Name)) j.at(#Name).get_to(s.Name);
		ControllerSettings_Fields(FieldArg)
	#undef FieldArg
	}

	void to_json(nlohmann::json& j, const ControllerSettings& s)
	{
	#define FieldArg(Type, Name, DefaultValue) j[#Name] = s.Name;
		ControllerSettings_Fields(FieldArg)
	#undef FieldArg
	}

	void from_json(const nlohmann::json& j, KeyboardSettings& s)
	{
	#define FieldArg(Type, Name, DefaultValue) if (j.contains(#Name)) j.at(#Name).get_to(s.Name);
		KeyboardSettings_Fields(FieldArg)
	#undef FieldArg
	}

	void to_json(nlohmann::json& j, const KeyboardSettings& s)
	{
	#define FieldArg(Type, Name, DefaultValue) j[#Name] = s.Name;
		KeyboardSettings_Fields(FieldArg)
	#undef FieldArg
	}

	void from_json(const nlohmann::json& j, InternalSettings& s)
	{
	#define FieldArg(Type, Name, DefaultValue) if (j.contains(#Name)) j.at(#Name).get_to(s.Name);
		InternalSettings_Fields(FieldArg, FieldArg)
	#undef FieldArg
	}

	void to_json(nlohmann::json& j, const InternalSettings& s)
	{
	#define FieldArg(Type, Name, DefaultValue) j[#Name] = s.Name;
		InternalSettings_Fields(FieldArg, FieldArg)
	#undef FieldArg
	}
	
	void from_json(const nlohmann::json& j, SerialisedSettings& s)
	{
	#define FieldArgNoVal(Type, Name) if (j.contains(#Name)) j.at(#Name).get_to(s.Name);
		SerialisedSettings_Fields(FieldArgNoVal)
	#undef FieldArgNoVal
	}

	void to_json(nlohmann::json& j, const SerialisedSettings& s)
	{
	#define FieldArgNoVal(Type, Name) j[#Name] = s.Name;
		SerialisedSettings_Fields(FieldArgNoVal)
	#undef FieldArgNoVal
	}

	SerialisedSettings SerialisedSettings::Load(const char* fileName, const char* gameVersion)
	{
		Leacon_Profiler;
		SerialisedSettings results;

		fs::path filePath = fs::path(fileName);
		if (fs::exists(filePath) == false)
			return results;

		std::ifstream f(filePath.c_str());
		nlohmann::json json = nlohmann::json::parse(f);

		from_json(json, results);
		return results;
	}

	void SerialisedSettings::Save(const char* fileName, nlohmann::json& json)
	{
		Leacon_Profiler;
		fs::path filePath = fs::path(fileName);
		std::ifstream f(filePath.c_str());
		
		to_json(json, *this);

		std::ofstream o(filePath.c_str());
		o << std::setw(4) << json << std::endl;
		o.close();
	}

	bool SerialisedSettings::ToSettings(Settings& results, const char* gameVersionString) const
	{
		Leacon_Profiler;
		results.serialisedSettings = *this;

		if (leagueController.gameVersion != gameVersionString)
		{
			results.gameVersionHash = 0;
			results.addressesHash = 0;
			return true;
		}

		// Get buffer from hash
		std::string decoded = base64_decode(leagueController.hash);
		std::vector<u8> bufferContents((const u8*)decoded.c_str(), (const u8*)decoded.c_str() + decoded.size());
		const u8* buffer = bufferContents.data();
		
		// Ensure we have enough data in the buffer to read the offsets
		size_t requiredSize = sizeof(u32) * 2;
		results.addresses.ForEachAddress([&buffer, &requiredSize](Offset& offset) { requiredSize += sizeof(Offset); });
		if (bufferContents.size() < requiredSize)
		{
			results.gameVersionHash = 0;
			results.addressesHash = 0;
			return true;
		}
		
		results.gameVersionHash = *(u32*)buffer; buffer += sizeof(u32);
		results.addressesHash = *(u32*)buffer; buffer += sizeof(u32);

		// Different game version, don't load address data
		u32 currentGameVersionHash = FNVa1(gameVersionString);
		if (currentGameVersionHash != results.gameVersionHash)
		{
			results.gameVersionHash = 0;
			results.addressesHash = 0;
			return true;
		}

		u32 currentHash = 0;
		results.addresses.ForEachAddress([&buffer, &currentHash](Offset& offset)
		{
			offset = *(Offset*)buffer;
			buffer += sizeof(Offset);

			currentHash = currentHash ^ FNVa1((const char*)&offset, sizeof(Offset));
		});

		// Hash mismatch, return default address data
		if (currentHash != results.addressesHash)
		{
			results.addresses = AddressData();
			results.addressesHash = 0;
		}
		return true;
	}
	
	bool SerialisedSettings::FromSettings(const Settings& settings, const char* gameVersionString)
	{
		Leacon_Profiler;
		u8 bufferContent[sizeof(AddressData) + sizeof(u32) * 2];

		u8* buffer = bufferContent;
		u32* bufferPtr = (u32*)buffer;
		u32* gameVersionHashPtr = bufferPtr;
		u32* addressHashPtr = &bufferPtr[1];
		
		u32 currentAddressHash = 0;
		buffer += sizeof(u32) * 2; // Skip 2 hashes
		settings.addresses.ForEachAddress([&buffer, &currentAddressHash](Offset offset)
		{
			currentAddressHash = currentAddressHash ^ FNVa1((const char*)&offset, sizeof(Offset));
			
			*(Offset*)buffer = offset;
			buffer += sizeof(Offset);
		});
		
		*gameVersionHashPtr = FNVa1(gameVersionString); bufferPtr++;
		*addressHashPtr = currentAddressHash; bufferPtr++;

		leagueController.settingsVersion = leagueController.currentVersion;
		leagueController.majorVersion = LeagueController::MajorVersion;
		leagueController.minorVersion = LeagueController::MinorVersion;
		leagueController.gameVersion = gameVersionString;
		leagueController.patchVerified = settings.patchVerified;
		
		auto totalSize = buffer - bufferContent;
		leagueController.hash = base64_encode(bufferContent, totalSize);
		return true;
	}

	bool AreSettingsLoaded()
	{
		return g_settingsData.settingsLoaded;
	}

	bool LoadSettings(std::string_view cwd, bool isApp)
	{
		Leacon_Profiler;
		if (g_settingsData.settingsLoaded)
			return true;

		static std::string executableVersion;
		if (executableVersion.empty())
		{
		#if !LEACON_FINAL
			if (isApp)
				executableVersion = "App";
			else
		#endif
			if (GetExecutableVersion(executableVersion) == false || executableVersion.empty())
				return false;
		}
		
		static fs::path settingsPath = fs::path(cwd) / "settings.json";

		g_settingsData.serialisedSettings = SerialisedSettings::Load(settingsPath.generic_string().c_str(), executableVersion.c_str());
		g_settingsData.settingsLoaded = g_settingsData.serialisedSettings.ToSettings(g_settingsData.settings, executableVersion.c_str());
		g_settingsData.settingsChanged = g_settingsData.settingsLoaded;
		g_settingsData.settingsAtStartup = g_settingsData.settings;
		return g_settingsData.settingsLoaded;
	}

	template< typename T >
	static std::string ToHex(T i)
	{
		std::stringstream stream;
		stream << "0x"
			<< std::setfill('0') << std::setw(sizeof(T) * 2)
			<< std::hex << i;
		return stream.str();
	}

	bool WriteSettings(std::string_view cwd)
	{
		Leacon_Profiler;
		static fs::path settingsPath = Leacon_ProfilerEvalRet(fs::path(cwd) / "settings.json");

		bool hasChanged = Leacon_ProfilerEvalRet(g_settingsData.settingsAtStartup.addresses != g_settingsData.settings.addresses);
		hasChanged |= g_settingsData.settingsAtStartup.patchVerified != g_settingsData.settings.patchVerified;
		if (!hasChanged)
			return false;

		// TODO: Add a delay here, so that we're not writing to the file every 200ms
		{
			Leacon_ProfilerSection("ActuallyWriteSettings");
			std::string executableVersion;
			if (executableVersion.empty())
			{
				if (GetExecutableVersion(executableVersion) == false || executableVersion.empty())
					return false;
			}

			nlohmann::json json;
#if !LEACON_SUBMISSION
			g_settingsData.settings.addresses.ForEachNamedAddress([&json](const char* name, Offset& offset)
			{
				json["__DEBUG"][name] = ToHex(offset);
			});
#endif

			g_settingsData.serialisedSettings.FromSettings(g_settingsData.settings, executableVersion.c_str());
			g_settingsData.serialisedSettings.Save(settingsPath.generic_string().c_str(), json);
			g_settingsData.settingsAtStartup = g_settingsData.settings;
		}
		return true;
	}

	Settings& GetSettings() { return g_settingsData.settings; }

#if _WIN32
	bool GetExecutableVersion(const char* filePath, std::string& result)
	{
		DWORD  verHandle = 0;
		UINT   size = 0;
		LPBYTE lpBuffer = NULL;
		DWORD  verSize = GetFileVersionInfoSize(filePath, &verHandle);

		if (verSize == 0)
			return false;

		std::vector<u8> data(verSize, 0);
		if (GetFileVersionInfo(filePath, verHandle, verSize, data.data()) == false)
			return false;

		if (VerQueryValue(data.data(), "\\", (VOID FAR * FAR*) & lpBuffer, &size) == false || size == 0)
			return false;

		VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
		if (verInfo->dwSignature != 0xfeef04bd)
			return false;

		result = string_format("%d.%d.%d.%d",
			(verInfo->dwFileVersionMS >> 16) & 0xffff,
			(verInfo->dwFileVersionMS >> 0) & 0xffff,
			(verInfo->dwFileVersionLS >> 16) & 0xffff,
			(verInfo->dwFileVersionLS >> 0) & 0xffff);
		// GameOverlay_LogDebug(g_offsetLog, "Game version: %s", result.c_str());
		return true;
	}

	bool GetExecutableVersion(std::string& version)
	{
		static char exeLocation[1024] = { 0 };
		if (exeLocation[0] == 0)
		{
			DWORD result = GetModuleFileNameA(nullptr, &exeLocation[0], 1024);
			if (result == 0)
			{
				exeLocation[0] = 0;
				return false;
			}
		}

		return GetExecutableVersion(exeLocation, version);
	}
#endif
}