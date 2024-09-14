#include <game_overlay/radial_menu.hpp>
#include <game_overlay/input.hpp>
#include <game_overlay/window.hpp>

#include <glm/glm.hpp>
#include "shaders/radial_menu.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

#include <game_overlay/skerm.hpp>
#include <game_overlay/log.hpp>
#include <algorithm>

namespace GameOverlay
{
	using UISrcFile = GameOverlay::SourceHandler::Source::Ptr;
	
	//static GameOverlay::SourceHandler g_sourceHandler;
	static UISrcFile g_radialMenuItem;
	static std::string* g_textPointer;
	static const Spek::Duration maxHoverTime = 150_ms;

	std::vector<RadialMenu*>& GetMenus()
	{
		Leacon_Profiler;
		static std::vector<RadialMenu*> g_menus;
		return g_menus;
	}
	
	std::vector<glm::vec2> GetCircleEdgePoints(float circleSize, float circleSegments, int circlePieces)
	{
		Leacon_Profiler;
		static constexpr float PI = 3.14159265358979323846f;
		float pieceSice = (2 * PI) / circlePieces;

		std::vector<glm::vec2> points;

		float step = (1.0f / circleSegments) * 2 * PI;
		float halfWay = pieceSice * 0.5f;
		for (int i = 0; i <= circleSegments; i++)
		{
			float angle = step * i;
			if (angle > pieceSice)
				break;
			float x = circleSize * glm::cos(angle - halfWay);
			float y = circleSize * glm::sin(angle - halfWay);
			points.emplace_back(y, x);
		}

		return points;
	}

	struct Triangle
	{
		Triangle(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
		{
			Leacon_Profiler;
			vertices[0] = a;
			vertices[1] = b;
			vertices[2] = c;
		}

		const glm::vec2& operator[](int i) const { return vertices[i]; }

		glm::vec2 vertices[3];
	};

	std::vector<Triangle> Triangulate(const glm::vec2& outerA, const glm::vec2& outerB, const glm::vec2& innerA, const glm::vec2& innerB)
	{
		Leacon_Profiler;
		std::vector<Triangle> result;

		result.emplace_back(outerA, outerB, innerA);
		result.emplace_back(outerB, innerB, innerA);

		return result;
	}

	void CreateSokolData(CirclePieceMesh& inMesh)
	{
		Leacon_Profiler;
		sg_buffer_desc vbuf_desc = {};
		vbuf_desc.data.ptr = inMesh.pos.data();
		vbuf_desc.data.size = inMesh.pos.size() * sizeof(glm::vec2);
		inMesh.drawData->bind.vertex_buffers[0] = sg_make_buffer(&vbuf_desc);

		sg_buffer_desc ibuf_desc = {};
		ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
		ibuf_desc.data.ptr = inMesh.indices.data();
		ibuf_desc.data.size = inMesh.indices.size() * sizeof(int);
		// inMesh.drawData->bind.index_buffer = sg_make_buffer(&ibuf_desc);

		sg_blend_state imageBlend = { 0 };
		imageBlend.enabled = true;
		imageBlend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
		imageBlend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		imageBlend.src_factor_alpha = SG_BLENDFACTOR_ONE;
		imageBlend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;

		sg_pipeline_desc pipelineDesc = {};
		pipelineDesc.layout.attrs[ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT2;
		pipelineDesc.shader = sg_make_shader(texcube_shader_desc(sg_query_backend()));
		// pipelineDesc.index_type = SG_INDEXTYPE_UINT32;
		pipelineDesc.cull_mode = SG_CULLMODE_NONE;
		pipelineDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
		pipelineDesc.depth.write_enabled = true;
		pipelineDesc.colors[0].blend = imageBlend;

		inMesh.drawData->pip = sg_make_pipeline(pipelineDesc);
	}

	CirclePieceMesh::DrawData::~DrawData()
	{
		Leacon_Profiler;
		sg_destroy_buffer(bind.vertex_buffers[0]);
		sg_destroy_buffer(bind.index_buffer);
		sg_destroy_pipeline(pip);
	}

	CirclePieceMesh::CirclePieceMesh()
	{
		Leacon_Profiler;
	}

	void CirclePieceMesh::Draw(const glm::mat4& inMvp, Spek::Duration inHoverTime)
	{
		Leacon_Profiler;
		vs_params_t vs_params;
		vs_params.mvp = inMvp;
		vs_params.hoverTime = inHoverTime.ToSecF64() / maxHoverTime.ToSecF64();
		vs_params.blend = 0.7f;

		sg_apply_pipeline(drawData->pip);
		sg_apply_bindings(&drawData->bind);
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
		sg_draw(0, indices.size(), 1);
	}

	void AdjustBounds(glm::vec2& min, glm::vec2& max, const glm::vec2& pos)
	{
		Leacon_Profiler;
		if (pos.x < min.x)
			min.x = pos.x;
		if (pos.y < min.y)
			min.y = pos.y;

		if (pos.x > max.x)
			max.x = pos.x;
		if (pos.y > max.y)
			max.y = pos.y;
	}

	CirclePieceMesh::CirclePieceMesh(float inInnerSize, float inOuterSize, int inSliceSize, int inCircleSegments)
	{
		Leacon_Profiler;
		drawData = (std::make_shared<DrawData>());
		auto pointsOuter = GetCircleEdgePoints(inOuterSize, inCircleSegments, inSliceSize);
		auto pointsInner = GetCircleEdgePoints(inInnerSize, inCircleSegments, inSliceSize);
		
		for (int i = 0; i < pointsOuter.size() - 1; i++)
		{
			glm::vec2 quad[4] =
			{
				pointsOuter[i + 0],
				pointsOuter[i + 1],
				pointsInner[i + 0],
				pointsInner[i + 1],
			};

			auto triangles = Triangulate(quad[0], quad[1], quad[2], quad[3]);
			
			pos.push_back(triangles[0][0]);
			pos.push_back(triangles[0][1]);
			pos.push_back(triangles[0][2]);
			
			pos.push_back(triangles[1][0]);
			pos.push_back(triangles[1][1]);
			pos.push_back(triangles[1][2]);
			
			int index = pos.size() - 6;
			indices.push_back(index + 0);
			indices.push_back(index + 1);
			indices.push_back(index + 2);
			indices.push_back(index + 4);
			indices.push_back(index + 5);
			indices.push_back(index + 6);
		}

		min = glm::vec2(9e9, 9e9);
		max = glm::vec2(-9e9, -9e9);
		for (auto p : pos)
			AdjustBounds(min, max, p);

		CreateSokolData(*this);
	}

	RadialMenu::RadialMenu(const glm::vec2& inPos, float inSize) :
		m_pos(inPos),
		m_size(inSize)
	{
		Leacon_Profiler;
		GetMenus().push_back(this);
	}

	RadialMenu::~RadialMenu()
	{
		Leacon_Profiler;
		auto position = std::find(GetMenus().begin(), GetMenus().end(), this);
		if (position != GetMenus().end())
			GetMenus().erase(position);
	}

	void RadialMenu::Reset()
	{
		Leacon_Profiler;
		m_mesh = nullptr;
	}
	
	void RadialMenu::AddMenuItem(const MenuItem& item)
	{
		Leacon_Profiler;
		m_items.push_back(item);
		m_mesh = nullptr; // Dirty
	}

	void RadialMenu::Set(const glm::vec2& inPos, float inSize)
	{
		Leacon_Profiler;
		SetPos(inPos);
		SetSize(inSize);
	}

	void RadialMenu::SetPos(const glm::vec2& inPos)
	{
		Leacon_Profiler;
		m_pos = inPos;
	}

	const glm::vec2& RadialMenu::GetPos() const
	{
		return m_pos;
	}

	void RadialMenu::SetSize(float inSize)
	{
		Leacon_Profiler;
		m_size = inSize;
		m_mesh = nullptr; // Dirty
	}

	const float RadialMenu::GetSize() const
	{
		return m_size;
	}

	void RadialMenu::Init()
	{
		Leacon_Profiler;
		/*g_sourceHandler.CompileFile("data/ui/radial_menu.ui", [](UISrcFile source, Spek::File::Handle file, Spek::File::LoadState state)
		{
			Leacon_Profiler;
			assert(state == Spek::File::LoadState::Loaded);
			g_radialMenuItem = source;
			g_textPointer = nullptr;
		});*/
	}

	bool RadialMenu::WantsInput()
	{
		Leacon_Profiler;
		int mouseX, mouseY;
		bool hasMousePos = GetMousePos(mouseX, mouseY);
		glm::vec2 mousePos = hasMousePos ? glm::vec2(mouseX, mouseY) : glm::vec2(0, 0);
		if (hasMousePos == false)
			return false;

		for (const auto& menu : GetMenus())
		{
			float innerSize = 0.25f * menu->m_size;
			f32 mouseDistance = glm::distance2(mousePos, menu->m_pos);
			if (mouseDistance > innerSize * innerSize &&
				mouseDistance < menu->m_size * menu->m_size)
				return true;
		}

		return false;
	}

	std::string* GetTextPointer(std::shared_ptr<Skerm::ASDF::UIContainer>& container)
	{
		Leacon_Profiler;
		if (g_textPointer == nullptr)
		{
			for (auto child : container->Children)
			{
				if (child->Name == "Text" && child->Type == Skerm::ASDF::UIType::TextType)
				{
					auto text = (Skerm::ASDF::UIText*)child.get();
					g_textPointer = &text->Content;
					break;
				}
			}
		}

		return g_textPointer;
	}
	
	void RadialMenu::Draw(Spek::Duration inDt, const glm::mat4& viewProjection, f32 screenWidth, f32 screenHeight)
	{
		Leacon_Profiler;
		if (m_items.empty() || g_radialMenuItem == nullptr)
			return;

		float innerSize = 0.25f * m_size;
		if (m_mesh == nullptr)
			m_mesh = std::make_shared<CirclePieceMesh>(innerSize, m_size, m_items.size());

		static constexpr float PI = 3.14159265358979323846f;
		static constexpr float distFromCenter = 10.0f;

		int mouseX, mouseY;
		bool hasMousePos = GetMousePos(mouseX, mouseY);
		glm::vec2 mousePos = hasMousePos ? glm::vec2(mouseX, mouseY) : glm::vec2(0, 0);
		f32 mouseDistance = glm::distance2(mousePos, m_pos);
		bool itemCanBeSelected = hasMousePos && mouseDistance > innerSize * innerSize && mouseDistance < m_size * m_size;
		if (itemCanBeSelected == false)
		{
			m_lastHoverItem = m_hoverItem;
			m_hoverItem.index = -1;
			m_hoverItem.time = 0_ms;
		}

		m_lastHoverItem.time -= inDt;
		m_lastHoverItem.time -= inDt;
		if (m_lastHoverItem.time < 0_ms)
			m_lastHoverItem.time = 0_ms;
		
		f32 middlePoint = 1.25f * m_size * 0.5f; // The point between the outer ring (1.0) and the inner one (0.25)
		f32 halfSize = 0.3333f * (m_size - innerSize);
		glm::mat4 offset = glm::translate(glm::vec3(0, distFromCenter, 0.0f)); // offset from center of menu
		float step = (1.0f / m_items.size()) * 2 * PI;
		float angle = 0;
		auto& container = g_radialMenuItem->Output.Elements[0];
		glm::vec2 right;
		std::string* text = GetTextPointer(container);

		f32 closestDist = 9e9;
		int closestItem = -1;

		for (int i = 0; i < m_items.size(); i++)
		{
			auto& item = m_items[i];
			glm::mat4 rot = glm::rotate(angle, glm::vec3(0, 0, 1)) * offset;
			glm::mat4 model = glm::translate(glm::vec3(m_pos, 0.0f)) * rot;
			glm::vec2 centerPoint(model[3].x, model[3].y);
			glm::vec2 up = glm::vec2(rot[1]) * glm::vec2(middlePoint, -middlePoint);

			Spek::Duration hoverTime = 0_ms;
			if (m_hoverItem.index == i)
				hoverTime = m_hoverItem.time;
			else if (m_lastHoverItem.index == i)
				hoverTime = m_lastHoverItem.time;

			float distToCenter = glm::distance2(mousePos - m_pos, up);
			if (closestDist >= distToCenter && itemCanBeSelected)
			{
				closestDist = distToCenter;
				closestItem = i;
			}

			// Render background
			m_mesh->Draw(viewProjection * model, hoverTime);

			// Render text/icon
			glm::vec2 min = centerPoint + up - glm::vec2(halfSize);
			glm::vec2 max = centerPoint + up + glm::vec2(halfSize);

			f32 anglePi = angle > PI ? angle - PI : angle;
			min.y -= (1 - (anglePi / PI)) * 40.0f + 20.0f;
			max.y -= (1 - (anglePi / PI)) * 40.0f + 20.0f;
			
			if (text) *text = item.text;
			GameOverlay::RenderUI(container.get(), min, max, screenWidth, screenHeight);
			angle += step;
		}

		if (closestItem != m_hoverItem.index)
		{
			m_lastHoverItem = m_hoverItem;
			m_hoverItem.time = 0_ms;
			m_hoverItem.index = closestItem;
		}

		m_hoverItem.time += inDt;
		if (m_hoverItem.time > maxHoverTime)
			m_hoverItem.time = maxHoverTime;

		m_lastHoverItem.time -= inDt;
		if (m_hoverItem.time < 0_ms)
			m_hoverItem.time = 0_ms;
	}
}
