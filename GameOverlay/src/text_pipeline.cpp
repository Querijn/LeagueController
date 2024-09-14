#include <spek/renderer/text_pipeline.hpp>
#include <spek/ecs/components/transform.hpp>

#include <sokol_gfx.h>
#include <sokol_shape.h>

namespace Spek
{
	TextPipeline::TextPipeline() :
		m_vertexParams()
	{
	}

	TextPipeline::~TextPipeline()
	{
	}

	void TextPipeline::OnMeshRender(const glm::mat4& inViewProj, const Entity& inEntity, const ASDF::MeshComponent& inMesh)
	{
		ASDF::TransformComponent* transform = inEntity.TryGet<ASDF::TransformComponent>();
		if (transform)
			m_vertexParams.MVP = inViewProj * transform->Transform;
		else
			m_vertexParams.MVP = inViewProj;
	}

	void TextPipeline::OnPrimRender(const glm::mat4& inViewProj, const Entity& inEntity, const ASDF::Primitive& inPrimitive)
	{
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_TextVertexParams, SG_RANGE(m_vertexParams));
	}

	sg_pipeline_desc TextPipeline::CreatePipelineDesc()
	{
		sg_pipeline_desc pipelineDesc = {};
		m_shader = pipelineDesc.shader = sg_make_shader(text_shader_desc(sg_query_backend()));
		pipelineDesc.layout.attrs[ATTR_TextVertexShader_Positions].format = SG_VERTEXFORMAT_FLOAT3;
		pipelineDesc.layout.attrs[ATTR_TextVertexShader_UVs].format = SG_VERTEXFORMAT_FLOAT2;
		pipelineDesc.index_type = SG_INDEXTYPE_UINT16;
		pipelineDesc.cull_mode = SG_CULLMODE_NONE;
		pipelineDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
		pipelineDesc.depth.write_enabled = true;

		return pipelineDesc;
	}

	void Spek::TextPipeline::DestroyPipelineDesc()
	{
		sg_destroy_shader(m_shader);
	}
}
