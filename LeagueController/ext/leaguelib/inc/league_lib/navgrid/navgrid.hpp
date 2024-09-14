#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <spek/file/file.hpp>
#include <spek/util/types.hpp>

namespace LeagueLib
{
	enum NavGridCellFlag : u16
	{
		HAS_GRASS = 0x1,
		NOT_PASSABLE = 0x2,
		BUSY = 0x4,
		TARGETTED = 0x8,
		MARKED = 0x10,
		PATHED_ON = 0x20,
		SEE_THROUGH = 0x40,
		OTHER_DIRECTION_END_TO_START = 0x80,
		HAS_GLOBAL_VISION = 0x100,
	};

#pragma pack(push, 1)
	struct NavGridCell
	{
		f32 centerHeight;
		i32 sessionId;
		f32 arrivalCost;
		u32 isOpen;
		f32 heuristic;
		i16 x;
		i16 y;
		u32 actorList;
		u32 unknown0;
		i32 goodCellSessionId;
		f32 refHintWeight;
		u16 unknown1;
		i16 arrivalDirection;
		u16 unknown2; // This is some kind of index?
		u16 unknown3; // This draws squares as 4 triangles across the map. Maybe nearest exit to larger block?
	};
#pragma pack(pop)

	bool IsCellPassable(NavGridCellFlag flag);

	class NavGrid
	{
	public:
		using Handle = std::shared_ptr<NavGrid>;

		using OnLoadFunction = std::function<void(NavGrid::Handle)>; 
		~NavGrid();

		static NavGrid::Handle Load(const char* a_File, OnLoadFunction a_Callback = nullptr);
		bool CastRay2DPassable(const glm::vec3& origin, const glm::vec3& dir, float length, glm::vec3& outPos);

		Spek::File::LoadState GetLoadState() const;

		int GetCellCountX() { return m_CellCount.x; }
		int GetCellCountY() { return m_CellCount.y; }

		glm::vec3 GetMinGridPos() { return m_MinGridPosition; }
		glm::vec3 GetMaxGridPos() { return m_MaxGridPosition; }
		glm::vec3 GetOneOverGridSize() { return m_OneOverGridSize; }

		glm::vec3 TranslateToNavGrid(const glm::vec3& vector) const;
		const std::vector<NavGridCellFlag>& GetCellState() const { return m_Flags; }
		bool GetCellStateAt(const glm::vec3& pos, NavGridCellFlag& result) const;
		bool GetHeight(const glm::vec3& pos, f32& result) const;

	private:
		Spek::File::LoadState m_LoadState = Spek::File::LoadState::NotLoaded;
		Spek::File::Handle m_file;

		glm::vec3 m_MinGridPosition;
		glm::vec3 m_MaxGridPosition;
		glm::vec3 m_OneOverGridSize;

		std::vector<NavGridCellFlag> m_Flags;
		std::vector<NavGridCell> m_Cells;
		glm::ivec3 m_CellCount;
		f32 m_CellSize;
		f32 m_OneOverDelta;

		NavGrid();
	};
}