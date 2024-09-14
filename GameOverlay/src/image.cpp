#include <game_overlay/image.hpp>
#include <game_overlay/overlay.hpp>
#include <game_overlay/window.hpp>
#include <game_overlay/log.hpp>

#include "renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <d3d11.h>
#include <sokol.hpp>
#include "shaders/image.hpp"
#include <spek/file/file.hpp>

namespace GameOverlay
{
	GameOverlay::LogCategory g_imageLog("Image");

	struct ImageData
	{
		~ImageData()
		{
			if (textureSokol.id == SG_INVALID_ID)
			{
				sg_destroy_image(textureSokol);
				textureSokol.id = SG_INVALID_ID;
			}
		}

		glm::ivec2 size;
		int bytesPerPixel = 4;
		ID3D11ShaderResourceView* textureDirectX11 = nullptr;
		sg_image textureSokol;
		sg_bindings bind;
		std::vector<uint8_t> buffer;
		Spek::File::Handle file;
	};

	static struct ImgRenderData
	{
		ImageVertexParams_t vertexParams;
		sg_pass_action passAction;
		sg_pipeline pipeline;
		sg_bindings bind;
		sg_shader shader;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewProj;
		bool initialised = false;
	} g_imageData;

	static void InitImageRenderer()
	{
		Leacon_Profiler;
		if (g_imageData.initialised)
			return;
		g_imageData.initialised = true;

		float vertices[] =
		{
			-0.5f,  0.5f,
			 0.5f,  0.5f,
			 0.5f, -0.5f,
			-0.5f, -0.5f,
		};

		sg_buffer_desc vertexBufferDesc = { 0 };
		vertexBufferDesc.data = SG_RANGE(vertices);
		vertexBufferDesc.label = "image-vertices";
		g_imageData.bind.vertex_buffers[0] = sg_make_buffer(vertexBufferDesc);

		/* an index buffer with 2 triangles */
		uint16_t indices[] = { 0, 1, 2,  0, 2, 3 };
		sg_buffer_desc indexBufferDesc = { 0 };
		indexBufferDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
		indexBufferDesc.data = SG_RANGE(indices);
		indexBufferDesc.label = "image-indices";
		g_imageData.bind.index_buffer = sg_make_buffer(indexBufferDesc);

		sg_shader shd = sg_make_shader(image_shader_desc(sg_query_backend()));
		sg_blend_state imageBlend = { 0 };
		imageBlend.enabled = true;
		imageBlend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
		imageBlend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		imageBlend.src_factor_alpha = SG_BLENDFACTOR_ONE;
		imageBlend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;

		sg_pipeline_desc pipelineDesc = { 0 };
		pipelineDesc.shader = shd;
		pipelineDesc.index_type = SG_INDEXTYPE_UINT16;
		pipelineDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
		pipelineDesc.blend_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		pipelineDesc.colors[0].blend = imageBlend;
		pipelineDesc.layout.attrs[ATTR_vs_vertexPosition].format = SG_VERTEXFORMAT_FLOAT2;
		pipelineDesc.label = "image-pipeline";
		g_imageData.pipeline = sg_make_pipeline(pipelineDesc);

		g_imageData.passAction.colors[0].action = SG_ACTION_DONTCARE;
		g_imageData.passAction.depth.action = SG_ACTION_CLEAR;
	}

	void Image::OnFrameStart()
	{
		Leacon_Profiler;
		g_imageData.view = glm::identity<glm::mat4>();
		g_imageData.proj = glm::ortho(0.0f, (float)GetWidth(), (float)GetHeight(), 0.0f, -5.0f, 5.0f);

		g_imageData.viewProj = g_imageData.proj * g_imageData.view;
	}

	uint8_t* Image::GetBuffer() const
	{
		Leacon_Profiler;
		return m_data->buffer.data();
	}

	void Image::Render(const glm::ivec2& intPos)
	{
		Leacon_Profiler;
		InitImageRenderer();
		if (m_data->bind.index_buffer.id != g_imageData.bind.index_buffer.id)
			memcpy(&m_data->bind, &g_imageData.bind, sizeof(sg_bindings));

		m_data->bind.fs_images[0] = m_data->textureSokol;
		if (m_data->bind.fs_images[0].id == SG_INVALID_ID)
			return; // Don't render

		int width = GetWidth() ? GetWidth() : sapp_width();
		int height = GetHeight() ? GetHeight() : sapp_height();

		sg_begin_default_pass(&g_imageData.passAction, width, height);
		sg_apply_pipeline(g_imageData.pipeline);

		glm::vec2 windowSize(width, height);
		glm::vec2 invSize = 1.0f / windowSize;
		glm::vec2 relPos = glm::vec2(intPos) * invSize;
		relPos.y = 1.0f - relPos.y;

		g_imageData.vertexParams.pos = relPos * 2.0f - 1.0f; // From 0~1 to -1~1
		g_imageData.vertexParams.size = glm::vec2(m_data->size) * invSize * 2.0f;

		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_ImageVertexParams, SG_RANGE(g_imageData.vertexParams));
		sg_apply_bindings(m_data->bind);
		sg_draw(0, 6, 1);

		sg_end_pass();
	}

	void LoadForSokol(uint8_t* imageData, int imageWidth, int imageHeight, int bpp, sg_image& result)
	{
		Leacon_Profiler;
		if (result.id != SG_INVALID_ID)
		{
			sg_destroy_image(result);
			result.id = SG_INVALID_ID;
			GameOverlay_LogDebug(g_imageLog, "Destroyed old Sokol image (%u)", result.id);
		}

		sg_image_desc imageDesc = {};
		imageDesc.width = imageWidth;
		imageDesc.height = imageHeight;
		imageDesc.pixel_format = bpp == 4 ? SG_PIXELFORMAT_RGBA8 : SG_PIXELFORMAT_R8;
		imageDesc.data.subimage[0][0].ptr = imageData;
		imageDesc.data.subimage[0][0].size = (size_t)imageWidth * (size_t)imageHeight * bpp;
		result = sg_make_image(imageDesc);
		GameOverlay_LogDebug(g_imageLog, "Loaded Sokol image of size %dx%d (%d bpp, id = %u)", imageWidth, imageHeight, bpp, result.id);
	}

	bool TryLoadForDX11(stbi_uc* imageData, int imageWidth, int imageHeight, ID3D11ShaderResourceView** outView)
	{
		Leacon_Profiler;
		if (GameOverlay::GetRendererType() != RendererType::DX11)
			return false;

		ID3D11Device* device = (ID3D11Device*)GameOverlay::GetDevice();
		GameOverlay_Assert(g_imageLog, device);

		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = imageWidth;
		desc.Height = imageHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D* texture = NULL;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = imageData;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		device->CreateTexture2D(&desc, &subResource, &texture);

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		GameOverlay_Assert(g_imageLog, texture);

		device->CreateShaderResourceView(texture, &srvDesc, outView);
		texture->Release();
		return true;
	}

	bool Image::Reload()
	{
		Leacon_Profiler;
		int imageWidth = m_data->size.x;
		int imageHeight = m_data->size.y;
		int imageBpp = m_data->bytesPerPixel;
		uint8_t* imageData = m_data->buffer.data();

		LoadForSokol(imageData, imageWidth, imageHeight, imageBpp, m_data->textureSokol);

		if (TryLoadForDX11(imageData, imageWidth, imageHeight, &m_data->textureDirectX11))
			return true;

		// TODO: Add DX9 support (TryLoadForDX9)

		return m_data->textureSokol.id != 0;
	}

	bool Image::TryLoadResource(uint8_t* imageData, int imageWidth, int imageHeight)
	{
		Leacon_Profiler;
		m_data->size.x = imageWidth;
		m_data->size.y = imageHeight;

		int sizeToCopy = m_data->size.x * m_data->size.y * m_data->bytesPerPixel;
		m_data->buffer.resize(sizeToCopy);
		memcpy(m_data->buffer.data(), imageData, sizeToCopy);
		
		return Reload();
	}

	Image::Image(std::string_view imageFileName) :
		m_data(std::make_shared<ImageData>())
	{
		Leacon_Profiler;
		m_data->file = Spek::File::Load(imageFileName.data(), [this](Spek::File::Handle file)
		{
			std::string fileName = file ? file->GetName() : "unknown";
			GameOverlay_AssertF(g_imageLog, file && file->GetLoadState() == Spek::File::LoadState::Loaded, "Failed to load image file: %s", fileName.c_str());

			auto data = file->GetData();
			int imageWidth, imageHeight, bytesPerPixel;
			stbi_uc* imageData = stbi_load_from_memory(data.data(), data.size(), &imageWidth, &imageHeight, &bytesPerPixel, 4);
			GameOverlay_AssertF(g_imageLog, imageData, "Was unable to load image '%s'", fileName.c_str());
			if (imageData == nullptr)
				return;

			bool result = TryLoadResource(imageData, imageWidth, imageHeight);
			GameOverlay_Assert(g_imageLog, result);

			stbi_image_free(imageData);
		});
	}

	Image::Image(uint32_t* imageData, int imageWidth, int imageHeight) :
		m_data(std::make_shared<ImageData>())
	{
		Leacon_Profiler;
		TryLoadResource((uint8_t*)imageData, imageWidth, imageHeight);
	}

	Image::Image(int imageWidth, int imageHeight, int bpp) :
		m_data(std::make_shared<ImageData>())
	{
		Leacon_Profiler;
		m_data->bytesPerPixel = bpp;

		std::vector<uint8_t> data(imageWidth * imageHeight * bpp);
		memset(data.data(), 0, data.size());
		TryLoadResource((uint8_t*)data.data(), imageWidth, imageHeight);
	}
	
	Image::~Image()
	{
		m_data = nullptr;
	}

	bool Image::HasData() const
	{
		return !!m_data && GetAsDX11() != nullptr;
	}

	ID3D11ShaderResourceView* Image::GetAsDX11() const { return m_data ? m_data->textureDirectX11 : nullptr; }
	
	sg_image Image::GetForSokol() const
	{
		return m_data ? m_data->textureSokol : sg_image{0};
	}
}