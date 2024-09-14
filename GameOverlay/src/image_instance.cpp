#include <game_overlay/image_instance.hpp>
#include <game_overlay/image.hpp>
#include <game_overlay/log.hpp>

namespace GameOverlay
{
	std::set<ImageInstance*> ImageInstance::g_instances;

	ImageInstance::ImageInstance(std::string_view imageFileName, const glm::ivec2& pos) :
		m_image(std::make_shared<Image>(imageFileName)),
		m_pos(pos)
	{
		g_instances.insert(this);
	}

	ImageInstance::ImageInstance(uint32_t* imageData, int imageWidth, int imageHeight, const glm::ivec2& pos) :
		m_image(std::make_shared<Image>(imageData, imageWidth, imageHeight)),
		m_pos(pos)
	{
		g_instances.insert(this);
	}

	ImageInstance::~ImageInstance()
	{
		g_instances.erase(g_instances.find(this));
	}

	void ImageInstance::RenderAll()
	{
		Leacon_Profiler;
		for (auto instance : g_instances)
			if (instance->m_shouldRender)
				instance->m_image->Render(instance->m_pos);
	}
	
	void ImageInstance::SetPos(int x, int y)
	{
		SetPos(glm::ivec2(x, y));
	}
	
	void ImageInstance::SetPos(const glm::ivec2& pos) { m_pos = pos; }
	glm::ivec2 ImageInstance::GetPos() const { return m_pos; }

	void ImageInstance::SetShouldRender(bool shouldRender) { m_shouldRender = shouldRender; }
	bool ImageInstance::ShouldRender() const { return m_shouldRender; }
}