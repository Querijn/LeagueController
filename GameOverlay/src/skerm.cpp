#include <game_overlay/skerm.hpp>
#include <game_overlay/font.hpp>
#include <game_overlay/image.hpp>
#include <game_overlay/log.hpp>
#include <spek/file/file.hpp>
#include <spek/util/hash.hpp>

#include <skerm/asdf.hpp>
#include <skerm/compiler.hpp>
#include <skerm/source_file.hpp>
#include <sokol.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>

#include "shaders/skerm_image.hpp"
#include "shaders/skerm_container.hpp"
#include "shaders/text.hpp"

namespace GameOverlay
{
	using namespace Skerm;
	using namespace Spek;
	LogCategory g_skermLog("Skerm");

	struct TextModel
	{
		~TextModel()
		{
			GameOverlay_LogDebug(g_skermLog, "Destroying text model");
			sg_destroy_buffer(bind.vertex_buffers[0]);
			sg_destroy_buffer(bind.index_buffer);

			sg_destroy_image(bind.fs_images[0]);
		}

		sg_bindings bind;
		size_t fontSize;
		size_t indexCount = 0;
	};
	static sg_pipeline g_textPipeline;
	
	static struct
	{
		std::vector<File::Handle> files;
		std::unordered_map<File::Handle, ASDF::UINamespacePacked*> namespaces;

		std::unordered_map<std::string, Font> fonts;
		std::unordered_map<std::string, std::shared_ptr<Image>> textures;
		std::unordered_map<u32, TextModel> textModels;
		
		sg_bindings quadBindings;
		sg_pass_action passAction;
	} g_skermData;

	static struct
	{
		SkermImageVertexParams_t vertexParams;
		SkermImageFragmentParams_t fragmentParams;
		
		sg_pipeline pipeline;
		sg_shader shader;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewProj;
		bool initialised = false;
	} g_skermImageData;

	static struct
	{
		SkermContainerVertexParams_t vertexParams;
		SkermContainerFragmentParams_t fragmentParams;

		sg_pipeline pipeline;
		bool initialised = false;
	} g_skermContainerData;

	static void ResetSkerm()
	{
		Leacon_Profiler;
		g_skermData.fonts.clear();
		g_skermData.textures.clear();
		g_skermData.textModels.clear();
	}

	static void InitQuadBindings()
	{
		Leacon_Profiler;
		if (g_skermData.quadBindings.vertex_buffers[0].id == SG_INVALID_ID)
		{
			float vertices[] =
			{
				0.5f + -0.5f,  0.5f + 0.5f,
				0.5f +  0.5f,  0.5f + 0.5f,
				0.5f +  0.5f, 0.5f + -0.5f,
				0.5f + -0.5f, 0.5f + -0.5f,
			};

			sg_buffer_desc vertexBufferDesc = { 0 };
			vertexBufferDesc.data = SG_RANGE(vertices);
			vertexBufferDesc.label = "image-vertices";
			g_skermData.quadBindings.vertex_buffers[0] = sg_make_buffer(vertexBufferDesc);
		}

		if (g_skermData.quadBindings.index_buffer.id == SG_INVALID_ID)
		{
			uint16_t indices[] = { 0, 1, 2,  0, 2, 3 };
			sg_buffer_desc indexBufferDesc = { 0 };
			indexBufferDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
			indexBufferDesc.data = SG_RANGE(indices);
			indexBufferDesc.label = "image-indices";
			g_skermData.quadBindings.index_buffer = sg_make_buffer(indexBufferDesc);
		}
	}

	void SetupDefaultPipelineSettings(sg_pipeline_desc& pipelineDesc)
	{
		Leacon_Profiler;
		sg_blend_state imageBlend = { 0 };
		imageBlend.enabled = true;
		imageBlend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
		imageBlend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		imageBlend.src_factor_alpha = SG_BLENDFACTOR_ONE;
		imageBlend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
		
		pipelineDesc.index_type = SG_INDEXTYPE_UINT16;
		pipelineDesc.cull_mode = SG_CULLMODE_NONE;
		pipelineDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
		pipelineDesc.depth.write_enabled = true;
		pipelineDesc.colors[0].blend = imageBlend;
	}

	static void InitImageRenderer()
	{
		Leacon_Profiler;
		if (g_skermImageData.initialised)
			return;
		g_skermImageData.initialised = true;

		InitQuadBindings();

		sg_shader shd = sg_make_shader(SkermImage_shader_desc(sg_query_backend()));

		sg_pipeline_desc pipelineDesc = { 0 };
		pipelineDesc.shader = shd;
		SetupDefaultPipelineSettings(pipelineDesc);
		pipelineDesc.layout.attrs[ATTR_vs_vertexPosition].format = SG_VERTEXFORMAT_FLOAT2;
		pipelineDesc.label = "image-pipeline";
		g_skermImageData.pipeline = sg_make_pipeline(pipelineDesc);
	}

	static void InitContainerRenderer()
	{
		Leacon_Profiler;
		if (g_skermContainerData.initialised)
			return;
		g_skermContainerData.initialised = true;

		InitQuadBindings();

		sg_shader shd = sg_make_shader(SkermContainer_shader_desc(sg_query_backend()));
		sg_blend_state imageBlend = { 0 };
		imageBlend.enabled = true;
		imageBlend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
		imageBlend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		imageBlend.src_factor_alpha = SG_BLENDFACTOR_ONE;
		imageBlend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;

		sg_pipeline_desc pipelineDesc = { 0 };
		pipelineDesc.shader = shd;
		SetupDefaultPipelineSettings(pipelineDesc);
		pipelineDesc.layout.attrs[ATTR_vs_vertexPosition].format = SG_VERTEXFORMAT_FLOAT2;
		pipelineDesc.label = "container-pipeline";
		g_skermContainerData.pipeline = sg_make_pipeline(pipelineDesc);
	}

	TextModel* GetTextModel(Font* inFont, const ASDF::UIText* inText, size_t inFontSize, glm::vec2& ioMaxSize)
	{
		Leacon_Profiler;
		if (inFont == nullptr || inFont->HasData() == false || inFontSize == 0)
			return nullptr;

		TextModel& model = g_skermData.textModels[FNV((std::to_string(inFontSize) + " -- " + inText->Content).c_str())];
		if (model.bind.vertex_buffers[0].id != SG_INVALID_ID)
			return &model;

		if (g_textPipeline.id == SG_INVALID_ID)
		{
			sg_blend_state imageBlend = { 0 };
			imageBlend.enabled = true;
			imageBlend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
			imageBlend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			imageBlend.src_factor_alpha = SG_BLENDFACTOR_ONE;
			imageBlend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
			
			sg_pipeline_desc pipelineDesc = { 0 };
			sg_shader shd = sg_make_shader(text_shader_desc(sg_query_backend()));
			pipelineDesc.shader = shd;
			pipelineDesc.index_type = SG_INDEXTYPE_UINT16;
			pipelineDesc.cull_mode = SG_CULLMODE_NONE;
			pipelineDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
			pipelineDesc.depth.write_enabled = true;
			pipelineDesc.colors[0].blend = imageBlend;
			pipelineDesc.layout.attrs[ATTR_TextVertexShader_Positions].format = SG_VERTEXFORMAT_FLOAT2;
			pipelineDesc.layout.attrs[ATTR_TextVertexShader_UVs].format = SG_VERTEXFORMAT_FLOAT2;
			pipelineDesc.label = "text-pipeline";
			g_textPipeline = sg_make_pipeline(pipelineDesc);
		}
		
		model.fontSize = inFontSize;

		using VertexT = glm::vec4;
		using UVT = glm::vec2;
		using IndexT = uint16_t;

		std::vector<VertexT> vertices;
		std::vector<IndexT> indices;

		std::string text = inText->Content;
		glm::vec2 bounds = inFont->WrapWords(text, inFontSize, ioMaxSize.x);
		IndexT indexOffset = 0;
		float x = 0;
		float y = inFont->GetTextHeight(text, inFontSize); // This makes the top left 0, 0

		for (auto character : text)
		{
			Font::Quad glyphInfo = inFont->GetCharQuad(&character, inFontSize, x, y);
			const float& topLeftX =  glyphInfo.topLeft.pos.x;
			const float& topLeftY =  glyphInfo.topLeft.pos.y;
			const float& botRightX = glyphInfo.bottomRight.pos.x;
			const float& botRightY = glyphInfo.bottomRight.pos.y;

			const float& topLeftU = glyphInfo.topLeft.uv.x;
			const float& topLeftV = glyphInfo.topLeft.uv.y;
			const float& botRightU = glyphInfo.bottomRight.uv.x;
			const float& botRightV = glyphInfo.bottomRight.uv.y;

			vertices.emplace_back(VertexT(topLeftX, topLeftY, topLeftU-1, topLeftV+1));
			vertices.emplace_back(VertexT(topLeftX, botRightY, topLeftU-1, botRightV+1));
			vertices.emplace_back(VertexT(botRightX, botRightY, botRightU-1, botRightV+1));
			vertices.emplace_back(VertexT(botRightX, topLeftY, botRightU-1, topLeftV+1));

			indices.push_back(indexOffset);
			indices.push_back(indexOffset + 1);
			indices.push_back(indexOffset + 2);
			indices.push_back(indexOffset + 2);
			indices.push_back(indexOffset + 3);
			indices.push_back(indexOffset);

			indexOffset += 4;
		}

		sg_buffer_desc vertexBuffer = {};
		vertexBuffer.type = SG_BUFFERTYPE_VERTEXBUFFER;
		vertexBuffer.data.ptr = vertices.data();
		vertexBuffer.data.size = vertices.size() * sizeof(VertexT);
		vertexBuffer.label = "text-vertex-buffer";
		model.bind.vertex_buffers[0] = sg_make_buffer(vertexBuffer);

		sg_buffer_desc indexBufferDesc = {};
		indexBufferDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
		indexBufferDesc.data.ptr = indices.data();
		indexBufferDesc.data.size = indices.size() * sizeof(IndexT);
		indexBufferDesc.label = "text-indexbuffer";
		model.bind.index_buffer = sg_make_buffer(indexBufferDesc);

		Image* image = inFont->GetTexture(inFontSize);
		assert(image);

		model.bind.fs_images[0] = image->GetForSokol();
		model.indexCount = indices.size();
		ioMaxSize = bounds;
		return &model;
	}

	Font* GetFont(std::string_view inFont)
	{
		Leacon_Profiler;
		Font& font = g_skermData.fonts[inFont.data()];
		if (font.HasData() == false)
		{
			font.Load(inFont.data(), [](Font& inFont)
			{
				GameOverlay_Assert(g_skermLog, inFont.HasData(), "Font could not be loaded!");
			});
		}

		return font.HasData() ? &font : nullptr;
	}

	Image* GetImage(std::string_view a_FileName)
	{
		auto& imagePtr = g_skermData.textures[a_FileName.data()];
		if (imagePtr == nullptr)
			imagePtr = std::make_shared<Image>(a_FileName.data());
		
		return imagePtr.get();
	}

	bool AddNamespace(std::string_view fileName, std::function<void(ASDF::UINamespacePacked&)> onLoadFunction)
	{
		Leacon_Profiler;
		File::Handle handle = File::Load(fileName.data(), [onLoadFunction](File::Handle file)
		{
			File::LoadState loadState = file ? file->GetLoadState() : File::LoadState::FailedToLoad;
			assert(loadState == File::LoadState::Loaded);

			const std::vector<u8>& data = file->GetData();
			ASDF::UINamespacePacked*& ptr = g_skermData.namespaces[file];
			ptr = (ASDF::UINamespacePacked*)data.data();

			if (onLoadFunction)
				onLoadFunction(*ptr);
		});

		if (handle == nullptr)
			return false;

		g_skermData.files.push_back(handle);
		return true;
	}

	void SetupMaps(Token::VarMap& inVarMapHor, Token::VarMap& inVarMapVert, f64 inWidth, f64 inHeight, f64 inTightWidth, f64 inTightHeight)
	{
		Leacon_Profiler;
		inVarMapHor["%"] = inWidth * 0.01;
		inVarMapVert["%"] = inHeight * 0.01;

		inVarMapHor["w"] = inVarMapVert["w"] = inTightWidth * 0.01;
		inVarMapHor["h"] = inVarMapVert["h"] = inTightHeight * 0.01;
		inVarMapHor["vw"] = inVarMapVert["vw"] = inWidth * 0.01;
		inVarMapHor["vh"] = inVarMapVert["vh"] = inHeight * 0.01;
		inVarMapHor["nt"] = inVarMapVert["nt"] = std::min(inWidth, inHeight) * 0.01;
	}

	void RenderContainer(ASDF::UIContainer* inContainer, Token::VarMap& varMapVert, Token::VarMap& varMapHor, const glm::vec2& inParentMin, const glm::vec2& inParentMax, glm::mat4& inOrtho)
	{
		Leacon_Profiler;
		f64 left, right, top, bottom;
		f64 color[4] = { 0, 0, 0, 1 };
		size_t colorCount;

		{
			Leacon_ProfilerSection("Calculate Props");
			colorCount = inContainer->Color.Calculate(color, 4, varMapVert);

			bool failed = colorCount != 1 && colorCount != 3 && colorCount != 4;
			failed |= !inContainer->Left.Calculate(&left, 1, varMapHor);
			failed |= !inContainer->Right.Calculate(&right, 1, varMapHor);
			failed |= !inContainer->Top.Calculate(&top, 1, varMapVert);
			failed |= !inContainer->Bottom.Calculate(&bottom, 1, varMapVert);
			if (failed)
				return;
		}

		const ASDF::UIText* textContainer = static_cast<const ASDF::UIText*>(inContainer); // Not yet known to be this type
		Font* font = nullptr;
		TextModel* textModel = nullptr;
		size_t actualFontSize = 0;
		glm::vec2 tightSize(0);
		if (inContainer->Type == ASDF::TextType && textContainer != nullptr)
		{
			font = GetFont(textContainer->FontFile);

			auto max = inParentMax - glm::vec2(right, bottom);
			auto min = inParentMin + glm::vec2(left, top);
			glm::ivec2 maxSize = max - min;
			bool fits = false;
			double fontSize = 12;
			while (fits == false)
			{
				bool hasFontSize;
				if (tightSize.x == 0)
				{
					hasFontSize = textContainer->FontSize.Calculate(&fontSize, 1, varMapVert) == 1;
				}
				else
				{
					glm::vec2 overSize = tightSize / (glm::vec2)maxSize;
					f32 overSizeNum = glm::max(overSize.x, overSize.y);
					fontSize = floor(fontSize / overSizeNum);
					hasFontSize = true;
				}
				actualFontSize = (size_t)std::floor(fontSize + 0.5);
				if (!hasFontSize || fontSize == 0)
					return;

				tightSize = font->GetTextBounds(textContainer->Content, actualFontSize);
				if (tightSize.x > maxSize.x || tightSize.y > maxSize.y)
				{
					std::string text = textContainer->Content;
					tightSize = font->WrapWords(text, actualFontSize, maxSize.x);
				}

				fits = !(tightSize.x > maxSize.x || tightSize.y > maxSize.y);
			}

			SetupMaps(varMapHor, varMapVert, maxSize.x, maxSize.y, tightSize.x, tightSize.y);
			bool hasFontSize = textContainer->FontSize.Calculate(&fontSize, 1, varMapVert) == 1;
			assert((hasFontSize && fontSize > 0) || (tightSize.x < 1 && tightSize.y < 1)); // Expected FontSize to be calculable.
			if (!hasFontSize || fontSize == 0)
				return;

			bool failed = false;
			failed |= !inContainer->Left.Calculate(&left, 1, varMapHor);
			failed |= !inContainer->Right.Calculate(&right, 1, varMapHor);
			failed |= !inContainer->Top.Calculate(&top, 1, varMapVert);
			failed |= !inContainer->Bottom.Calculate(&bottom, 1, varMapVert);
			assert(failed == false);
		}

		glm::vec4 colorVec;

		glm::vec3 position(inParentMin.x + left, inParentMin.y + top, 0);
		glm::vec3 size((inParentMax.x - right) - position.x, (inParentMax.y - bottom) - position.y, 1);
		switch (inContainer->Type)
		{
		case ASDF::ContainerType:
		{
			if (colorCount == 1)
				colorVec = glm::vec4(color[0], color[0], color[0], color[0] > 0.01f ? 1.0f : 0.0f);
			else if (colorCount == 3)
				colorVec = glm::vec4(color[0], color[1], color[2], 1.0f);
			else if (colorCount > 3)
				colorVec = glm::vec4(color[0], color[1], color[2], color[3]);

			Leacon_ProfilerSection("Render ContainerType");
			glm::mat4 model = glm::translate(position) * glm::scale(size);

			InitContainerRenderer();
			sg_apply_pipeline(g_skermContainerData.pipeline);

			g_skermContainerData.vertexParams.mvp = inOrtho * model;
			g_skermData.quadBindings.fs_images[0] = g_skermData.quadBindings.fs_images[1]; // unset (since it's shared between the image rendering)

			g_skermContainerData.fragmentParams.color = colorVec;

			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_SkermContainerVertexParams, SG_RANGE(g_skermContainerData.vertexParams));
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_SkermContainerFragmentParams, SG_RANGE(g_skermContainerData.fragmentParams));
			sg_apply_bindings(g_skermData.quadBindings);
			sg_draw(0, 6, 1);
			break;
		}

		case ASDF::ImageType:
		{
			if (color[0] < 0.01f) // TODO: Figure out if color was explicitly set. If not, set to 1.
				color[0] = 1.0f;
			if (colorCount == 1)
				colorVec = glm::vec4(color[0], color[0], color[0], 1.0f);
			else if (colorCount == 3)
				colorVec = glm::vec4(color[0], color[1], color[2], 1.0f);
			else if (colorCount > 3)
				colorVec = glm::vec4(color[0], color[1], color[2], color[3]);

			Leacon_ProfilerSection("Render Image");
			glm::mat4 model = glm::translate(position) * glm::scale(size);
			const ASDF::UIImage* image = static_cast<const ASDF::UIImage*>(inContainer);
			f64 extents[4] = { 0, 0, 0, 1 };
			image->Sprite.Calculate(extents, 4);
			
			InitImageRenderer();

			auto img = GetImage(image->Source);
			g_skermData.quadBindings.fs_images[0] = img->GetForSokol();
			if (g_skermData.quadBindings.fs_images[0].id == SG_INVALID_ID)
				break;

			sg_apply_pipeline(g_skermImageData.pipeline);

			g_skermImageData.vertexParams.mvp = inOrtho * model;
			g_skermImageData.vertexParams.spriteExtents = glm::vec4(extents[0], extents[1], extents[2], extents[3]);
			
			g_skermImageData.fragmentParams.color = colorVec;

			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_SkermImageVertexParams, SG_RANGE(g_skermImageData.vertexParams));
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_SkermImageFragmentParams, SG_RANGE(g_skermImageData.fragmentParams));
			sg_apply_bindings(g_skermData.quadBindings);
			sg_draw(0, 6, 1);
			break;
		}

		case ASDF::TextType:
		{
			if (colorCount == 1)
				colorVec = glm::vec4(color[0], color[0], color[0], 1.0f);
			else if (colorCount == 3)
				colorVec = glm::vec4(color[0], color[1], color[2], 1.0f);
			else if (colorCount > 3)
				colorVec = glm::vec4(color[0], color[1], color[2], color[3]);

			textModel = GetTextModel(font, textContainer, actualFontSize, tightSize);
			if (textContainer == nullptr || font == nullptr || textModel == nullptr)
				break;

			Leacon_ProfilerSection("Render Text");
			glm::mat4 model = glm::translate(position);
			sg_apply_pipeline(g_textPipeline);

			TextVertexParams_t vertexParams;
			vertexParams.MVP = inOrtho * model;

			TextFragmentParams_t fragmentParams;
			fragmentParams.Color = glm::vec4(1);

			sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TextVertexParams, SG_RANGE(vertexParams));
			sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_TextFragmentParams, SG_RANGE(fragmentParams));
			sg_apply_bindings(textModel->bind);
			sg_draw(0, textModel->indexCount, 1);
			break;
		}

		default:
			__debugbreak();
			break;

		}

		{
			Leacon_ProfilerSection("RenderChildren");
			for (auto& child : inContainer->Children)
			{
				auto min = inParentMin + glm::vec2(left, top);
				auto max = inParentMax - glm::vec2(right, bottom);
				size = glm::vec3(max - min, 1);
				SetupMaps(varMapHor, varMapVert, size.x, size.y, size.x, size.y);
				RenderContainer(child.get(), varMapVert, varMapHor, min, max, inOrtho);
			}
		}
	}

	static std::vector<SourceHandler::Source::Ptr>::const_iterator GetSource(const std::vector<SourceHandler::Source::Ptr>& inVector, File::Handle inHandle)
	{
		auto i = inVector.begin();
		for (; i != inVector.end(); i++)
			if ((*i)->FileHandle == inHandle)
				break;

		return i;
	}

	File::Handle SourceHandler::CompileFile(const char* inUIFile, OnLoadFunction inOnLoad)
	{
		return File::Load(inUIFile, [this, inOnLoad](File::Handle inFile, File::LoadState inLoadState)
		{
			if (inLoadState != File::LoadState::Loaded)
			{
				// __debugbreak(); // TODO
				// 
				// if (inOnLoad)
				// 	inOnLoad(nullptr, nullptr, inLoadState);
				return;
			}
			
			auto index = GetSource(m_sources, inFile);
			std::string fileName = inFile->GetFullName();

			ResetSkerm();

			Source::Ptr handle;
			bool parseSucceeded = false;

			if (index == m_sources.end())
			{
				handle = m_sources.emplace_back(std::make_shared<Source>());
				handle->FileHandle = inFile;
			}
			else
			{
				handle = *index;
			}

			parseSucceeded = SourceFile::Parse(fileName.c_str(), handle->Source, &handle->Errors);
			handle->Errors.Errors.clear();
			handle->Errors.Warnings.clear();
			 
			if (parseSucceeded)
			{
				Compiler::CompileOutput result = CompileSourceFile(*handle);

				handle->Errors.Errors[fileName].insert(handle->Errors.Errors[fileName].end(), result.Errors.begin(), result.Errors.end());
				handle->Errors.Warnings[fileName].insert(handle->Errors.Warnings[fileName].end(), result.Warnings.begin(), result.Warnings.end());
			}

			if (inOnLoad)
				inOnLoad(handle, inFile, inLoadState);
		});
	}

	SourceHandler::Source::Ptr SourceHandler::GetSourceByName(const char* inSourceFile) const
	{
		auto file = File::Load(inSourceFile, [](File::Handle f) {});
		if (file == nullptr)
			return nullptr;

		auto index = GetSource(m_sources, file);
		return index == m_sources.end() ? nullptr : *index;
	}

	void SourceHandler::Clear()
	{
		m_sources.clear();
	}

	template<typename a, typename b>
	bool IsReallyEmpty(std::map<a, std::vector<b>>& a_map)
	{
		for (auto& i : a_map)
			if (i.second.empty() == false)
				return false;

		return true;
	}

	Compiler::CompileOutput SourceHandler::CompileSourceFile(Source& inSet)
	{
		inSet.Errors.Errors.clear();
		inSet.Errors.Warnings.clear();

		inSet.Source.Verify(inSet.Errors);

		Compiler::CompileOutput output;
		if (IsReallyEmpty(inSet.Errors.Errors))
		{
			// Source to packed output
			if (m_compiler.CompileFile(inSet.Source, output) && output.Data.empty() == false)
			{
				const ASDF::UINamespacePacked* packedNamespace = reinterpret_cast<const ASDF::UINamespacePacked*>(output.Data.data());
				inSet.Output = (ASDF::UINamespace)(*packedNamespace);
			}
		}

		return output;
	}
	
	void RenderUI(ASDF::UIContainer* container, int screenWidth, int screenHeight)
	{
		Leacon_Profiler;
		RenderUI(container, glm::vec2(0), glm::vec2(screenWidth, screenHeight), screenWidth, screenHeight);
	}

	void RenderUI(ASDF::UIContainer* container, const glm::vec2& min, const glm::vec2& max, int screenWidth, int screenHeight)
	{
		Leacon_Profiler;
		glm::vec2 boundsSize = max - min;
		glm::mat4 ortho = glm::ortho(0.0f, (f32)screenWidth, (f32)screenHeight, 0.f, -1.0f, 1.0f);
		Skerm::Token::VarMap varMapVert, varMapHor;
		SetupMaps(varMapHor, varMapVert, boundsSize.x, boundsSize.y, boundsSize.x, boundsSize.y);

		RenderContainer(container, varMapVert, varMapHor, min, max, ortho);
	}
	
	void DestroySkerm()
	{
		ResetSkerm();

		g_skermData.files.clear();
		g_skermData.namespaces.clear();

		sg_destroy_buffer(g_skermData.quadBindings.vertex_buffers[0]);
		sg_destroy_buffer(g_skermData.quadBindings.index_buffer);
	}
}
