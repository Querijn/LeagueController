#include <league_lib/navgrid/navgrid.hpp>

#include <Windows.h>
#include <glm/glm.hpp>

namespace LeagueLib
{
	bool IsCellPassable(NavGridCellFlag flag)
	{
		return (flag & LeagueLib::NavGridCellFlag::NOT_PASSABLE) == 0;
	}

	using namespace Spek;

	NavGrid::NavGrid() {}

	NavGrid::~NavGrid() {}

	NavGrid::Handle NavGrid::Load(const char* a_File, OnLoadFunction a_Callback)
	{
		struct Enabler : public NavGrid { Enabler() : NavGrid() {} };
		Handle t_Result = std::make_shared<Enabler>();

		t_Result->m_file = File::Load(a_File, [a_Callback, t_Result](File::Handle a_File)
		{
			if (a_File == nullptr || a_File->GetLoadState() != File::LoadState::Loaded)
			{
				t_Result->m_LoadState = a_File ? a_File->GetLoadState() : File::LoadState::NotFound;
				return;
			}
			t_Result->m_LoadState = a_File->GetLoadState();

			u8 t_MajorVersion;
			u16 t_MinorVersion;
			u32 t_CellCountX;
			u32 t_CellCountY;
			std::vector<u32> t_RegionTags;

			auto size = a_File->GetData().size();
			size_t t_Offset = 0;
			a_File->Get(t_MajorVersion, t_Offset);
			a_File->Get(t_MinorVersion, t_Offset);

			a_File->Get(t_Result->m_MinGridPosition, t_Offset);
			a_File->Get(t_Result->m_MaxGridPosition, t_Offset);
			a_File->Get(t_Result->m_CellSize, t_Offset);
			a_File->Get(t_CellCountX, t_Offset);
			a_File->Get(t_CellCountY, t_Offset);

			t_Result->m_CellCount.x = t_CellCountX;
			t_Result->m_CellCount.y = t_CellCountY;

			u32 t_Count = t_CellCountX * t_CellCountY;
			t_Result->m_Cells.resize(t_Count);
			t_Result->m_Flags.resize(t_Count);

			a_File->Get(t_Result->m_Cells, t_Offset);
			a_File->Get(t_Result->m_Flags, t_Offset);

			t_Result->m_OneOverGridSize = glm::vec3((f32)t_CellCountX, 0, (f32)t_CellCountY) / (t_Result->m_MaxGridPosition - t_Result->m_MinGridPosition);
			if (a_Callback)
				a_Callback(t_Result);
		});

		return t_Result;
	}

	bool NavGrid::CastRay2DPassable(const glm::vec3& origin, const glm::vec3& dir, float length, glm::vec3& outPos)
	{
		f32 t_Distance = length + 0.02f;
		if (length < 0.02f)
			return 0;

		glm::vec3 gridPos = TranslateToNavGrid(origin);
		glm::vec2 t_Delta(gridPos.x, gridPos.z);

		while (true)
		{
			int t_CellIndexX = (int)gridPos.x;
			int t_CellIndexY = (int)gridPos.z;

			if (t_CellIndexX < 0)
			{
				t_CellIndexX = 0;
			}
			else if (t_CellIndexX >= m_CellCount.x)
			{
				t_CellIndexX = m_CellCount.x - 1;
			}

			if (t_CellIndexY < 0)
			{
				t_CellIndexY = 0;
			}
			else if (t_CellIndexY >= m_CellCount.y)
			{
				t_CellIndexY = m_CellCount.y - 1;
			}

			if (IsCellPassable(m_Flags[t_CellIndexX + t_CellIndexY * m_CellCount.x]) == false)
				break;

			t_Distance -= m_CellSize;
			gridPos.x += dir.x;
			gridPos.z += dir.z;

			if (t_Distance < 0.0)
				return false;
		}

		if (length + 0.02f == t_Distance)
		{
			outPos = origin;
		}
		else
		{
			outPos.x = (float)(m_CellSize * gridPos.x) + m_MinGridPosition.x;
			outPos.y = m_MinGridPosition.y; 
			outPos.z = (float)(m_CellSize * gridPos.z) + m_MinGridPosition.z;
		}

		return true;
	}

	bool NavGrid::GetCellStateAt(const glm::vec3& pos, NavGridCellFlag& result) const
	{
		const glm::vec3 navGrid = TranslateToNavGrid(pos);
		int index = (int)floorf(navGrid.z + 0.5f) * m_CellCount.x + (int)floorf(navGrid.x + 0.5f);
		if (index < 0 || index >= m_Flags.size())
			return false;

		result = m_Flags[index];
		return true;
	}

	bool NavGrid::GetHeight(const glm::vec3& pos, f32& result) const
	{
		const glm::vec3 navGrid = TranslateToNavGrid(pos);
		int index = (int)floorf(navGrid.z + 0.5f) * m_CellCount.x + (int)floorf(navGrid.x + 0.5f);
		if (index < 0 || index >= m_Cells.size())
			return false;

		result = m_Cells[index].centerHeight;
		return true;
	}

#define _DWORD u32
#define LODWORD(x)  (*((_DWORD*)&(x)))  // low dword
#define COERCE_FLOAT(x) (*(f32*)&x)

	struct obj_AI_Base {};
	struct EffectEmitter {};
	namespace Riot { namespace ParticleSystem { struct InstanceInterface{ }; } }

	Spek::File::LoadState NavGrid::GetLoadState() const
	{
		return m_LoadState;
	}

	glm::vec3 NavGrid::TranslateToNavGrid(const glm::vec3& vector) const
	{
		return (vector - m_MinGridPosition) * m_OneOverGridSize;
	}
}