#pragma once
/*
    #version:1# (machine generated, don't edit!)

    Generated by sokol-shdc (https://github.com/floooh/sokol-tools)

    Cmdline: sokol-shdc --input C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/image.glsl --output C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/src/shaders/image.hpp --slang glsl330:glsl300es:hlsl5:glsl100

    Overview:

        Shader program 'image':
            Get shader desc: image_shader_desc(sg_query_backend());
            Vertex shader: vs
                Attribute slots:
                    ATTR_vs_vertexPosition = 0
                Uniform block 'ImageVertexParams':
                    C struct: ImageVertexParams_t
                    Bind slot: SLOT_ImageVertexParams = 0
            Fragment shader: fs
                Image 'diffuse':
                    Type: SG_IMAGETYPE_2D
                    Component Type: SG_SAMPLERTYPE_FLOAT
                    Bind slot: SLOT_diffuse = 0


    Shader descriptor structs:

        sg_shader image = sg_make_shader(image_shader_desc(sg_query_backend()));

    Vertex attribute locations for vertex shader 'vs':

        sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
            .layout = {
                .attrs = {
                    [ATTR_vs_vertexPosition] = { ... },
                },
            },
            ...});

    Image bind slots, use as index in sg_bindings.vs_images[] or .fs_images[]

        SLOT_diffuse = 0;

    Bind slot and C-struct for uniform block 'ImageVertexParams':

        ImageVertexParams_t ImageVertexParams = {
            .pos = ...;
            .size = ...;
        };
        sg_apply_uniforms(SG_SHADERSTAGE_[VS|FS], SLOT_ImageVertexParams, &SG_RANGE(ImageVertexParams));

*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#if !defined(SOKOL_SHDC_ALIGN)
  #if defined(_MSC_VER)
    #define SOKOL_SHDC_ALIGN(a) __declspec(align(a))
  #else
    #define SOKOL_SHDC_ALIGN(a) __attribute__((aligned(a)))
  #endif
#endif
#define ATTR_vs_vertexPosition (0)
#define SLOT_diffuse (0)
#define SLOT_ImageVertexParams (0)
#pragma pack(push,1)
SOKOL_SHDC_ALIGN(16) typedef struct ImageVertexParams_t {
    glm::vec2 pos;
    glm::vec2 size;
} ImageVertexParams_t;
#pragma pack(pop)
/*
    #version 330
    
    uniform vec4 ImageVertexParams[1];
    layout(location = 0) in vec4 vertexPosition;
    out vec2 uv;
    
    void main()
    {
        gl_Position = vec4(ImageVertexParams[0].xy, 0.0, 0.0) + (vertexPosition * vec4(ImageVertexParams[0].zw, 1.0, 1.0));
        uv = vertexPosition.xy + vec2(0.5);
    }
    
*/
static const char vs_source_glsl330[286] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x33,0x33,0x30,0x0a,0x0a,0x75,0x6e,
    0x69,0x66,0x6f,0x72,0x6d,0x20,0x76,0x65,0x63,0x34,0x20,0x49,0x6d,0x61,0x67,0x65,
    0x56,0x65,0x72,0x74,0x65,0x78,0x50,0x61,0x72,0x61,0x6d,0x73,0x5b,0x31,0x5d,0x3b,
    0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,
    0x20,0x3d,0x20,0x30,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x34,0x20,0x76,0x65,
    0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x6f,0x75,
    0x74,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,
    0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,
    0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x76,0x65,0x63,0x34,
    0x28,0x49,0x6d,0x61,0x67,0x65,0x56,0x65,0x72,0x74,0x65,0x78,0x50,0x61,0x72,0x61,
    0x6d,0x73,0x5b,0x30,0x5d,0x2e,0x78,0x79,0x2c,0x20,0x30,0x2e,0x30,0x2c,0x20,0x30,
    0x2e,0x30,0x29,0x20,0x2b,0x20,0x28,0x76,0x65,0x72,0x74,0x65,0x78,0x50,0x6f,0x73,
    0x69,0x74,0x69,0x6f,0x6e,0x20,0x2a,0x20,0x76,0x65,0x63,0x34,0x28,0x49,0x6d,0x61,
    0x67,0x65,0x56,0x65,0x72,0x74,0x65,0x78,0x50,0x61,0x72,0x61,0x6d,0x73,0x5b,0x30,
    0x5d,0x2e,0x7a,0x77,0x2c,0x20,0x31,0x2e,0x30,0x2c,0x20,0x31,0x2e,0x30,0x29,0x29,
    0x3b,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,0x76,0x65,0x72,0x74,0x65,
    0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x2e,0x78,0x79,0x20,0x2b,0x20,0x76,
    0x65,0x63,0x32,0x28,0x30,0x2e,0x35,0x29,0x3b,0x0a,0x7d,0x0a,0x0a,0x00,
};
/*
    #version 330
    
    uniform sampler2D diffuse;
    
    layout(location = 0) out vec4 fragColor;
    in vec2 uv;
    
    void main()
    {
        fragColor = texture(diffuse, uv);
    }
    
*/
static const char fs_source_glsl330[152] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x33,0x33,0x30,0x0a,0x0a,0x75,0x6e,
    0x69,0x66,0x6f,0x72,0x6d,0x20,0x73,0x61,0x6d,0x70,0x6c,0x65,0x72,0x32,0x44,0x20,
    0x64,0x69,0x66,0x66,0x75,0x73,0x65,0x3b,0x0a,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,
    0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,0x29,0x20,0x6f,
    0x75,0x74,0x20,0x76,0x65,0x63,0x34,0x20,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,
    0x72,0x3b,0x0a,0x69,0x6e,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,0x0a,0x0a,
    0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,0x20,
    0x20,0x20,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,0x74,0x65,
    0x78,0x74,0x75,0x72,0x65,0x28,0x64,0x69,0x66,0x66,0x75,0x73,0x65,0x2c,0x20,0x75,
    0x76,0x29,0x3b,0x0a,0x7d,0x0a,0x0a,0x00,
};
/*
    #version 100
    
    uniform vec4 ImageVertexParams[1];
    attribute vec4 vertexPosition;
    varying vec2 uv;
    
    void main()
    {
        gl_Position = vec4(ImageVertexParams[0].xy, 0.0, 0.0) + (vertexPosition * vec4(ImageVertexParams[0].zw, 1.0, 1.0));
        uv = vertexPosition.xy + vec2(0.5);
    }
    
*/
static const char vs_source_glsl100[276] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x31,0x30,0x30,0x0a,0x0a,0x75,0x6e,
    0x69,0x66,0x6f,0x72,0x6d,0x20,0x76,0x65,0x63,0x34,0x20,0x49,0x6d,0x61,0x67,0x65,
    0x56,0x65,0x72,0x74,0x65,0x78,0x50,0x61,0x72,0x61,0x6d,0x73,0x5b,0x31,0x5d,0x3b,
    0x0a,0x61,0x74,0x74,0x72,0x69,0x62,0x75,0x74,0x65,0x20,0x76,0x65,0x63,0x34,0x20,
    0x76,0x65,0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,
    0x76,0x61,0x72,0x79,0x69,0x6e,0x67,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,
    0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,
    0x3d,0x20,0x76,0x65,0x63,0x34,0x28,0x49,0x6d,0x61,0x67,0x65,0x56,0x65,0x72,0x74,
    0x65,0x78,0x50,0x61,0x72,0x61,0x6d,0x73,0x5b,0x30,0x5d,0x2e,0x78,0x79,0x2c,0x20,
    0x30,0x2e,0x30,0x2c,0x20,0x30,0x2e,0x30,0x29,0x20,0x2b,0x20,0x28,0x76,0x65,0x72,
    0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x2a,0x20,0x76,0x65,
    0x63,0x34,0x28,0x49,0x6d,0x61,0x67,0x65,0x56,0x65,0x72,0x74,0x65,0x78,0x50,0x61,
    0x72,0x61,0x6d,0x73,0x5b,0x30,0x5d,0x2e,0x7a,0x77,0x2c,0x20,0x31,0x2e,0x30,0x2c,
    0x20,0x31,0x2e,0x30,0x29,0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,
    0x20,0x76,0x65,0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x2e,
    0x78,0x79,0x20,0x2b,0x20,0x76,0x65,0x63,0x32,0x28,0x30,0x2e,0x35,0x29,0x3b,0x0a,
    0x7d,0x0a,0x0a,0x00,
};
/*
    #version 100
    precision mediump float;
    precision highp int;
    
    uniform highp sampler2D diffuse;
    
    varying highp vec2 uv;
    
    void main()
    {
        gl_FragData[0] = texture2D(diffuse, uv);
    }
    
*/
static const char fs_source_glsl100[181] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x31,0x30,0x30,0x0a,0x70,0x72,0x65,
    0x63,0x69,0x73,0x69,0x6f,0x6e,0x20,0x6d,0x65,0x64,0x69,0x75,0x6d,0x70,0x20,0x66,
    0x6c,0x6f,0x61,0x74,0x3b,0x0a,0x70,0x72,0x65,0x63,0x69,0x73,0x69,0x6f,0x6e,0x20,
    0x68,0x69,0x67,0x68,0x70,0x20,0x69,0x6e,0x74,0x3b,0x0a,0x0a,0x75,0x6e,0x69,0x66,
    0x6f,0x72,0x6d,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x73,0x61,0x6d,0x70,0x6c,0x65,
    0x72,0x32,0x44,0x20,0x64,0x69,0x66,0x66,0x75,0x73,0x65,0x3b,0x0a,0x0a,0x76,0x61,
    0x72,0x79,0x69,0x6e,0x67,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x76,0x65,0x63,0x32,
    0x20,0x75,0x76,0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,
    0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x46,0x72,0x61,0x67,0x44,
    0x61,0x74,0x61,0x5b,0x30,0x5d,0x20,0x3d,0x20,0x74,0x65,0x78,0x74,0x75,0x72,0x65,
    0x32,0x44,0x28,0x64,0x69,0x66,0x66,0x75,0x73,0x65,0x2c,0x20,0x75,0x76,0x29,0x3b,
    0x0a,0x7d,0x0a,0x0a,0x00,
};
/*
    #version 300 es
    
    uniform vec4 ImageVertexParams[1];
    layout(location = 0) in vec4 vertexPosition;
    out vec2 uv;
    
    void main()
    {
        gl_Position = vec4(ImageVertexParams[0].xy, 0.0, 0.0) + (vertexPosition * vec4(ImageVertexParams[0].zw, 1.0, 1.0));
        uv = vertexPosition.xy + vec2(0.5);
    }
    
*/
static const char vs_source_glsl300es[289] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x33,0x30,0x30,0x20,0x65,0x73,0x0a,
    0x0a,0x75,0x6e,0x69,0x66,0x6f,0x72,0x6d,0x20,0x76,0x65,0x63,0x34,0x20,0x49,0x6d,
    0x61,0x67,0x65,0x56,0x65,0x72,0x74,0x65,0x78,0x50,0x61,0x72,0x61,0x6d,0x73,0x5b,
    0x31,0x5d,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,
    0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x34,
    0x20,0x76,0x65,0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,
    0x0a,0x6f,0x75,0x74,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,0x0a,0x0a,0x76,
    0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,
    0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x76,
    0x65,0x63,0x34,0x28,0x49,0x6d,0x61,0x67,0x65,0x56,0x65,0x72,0x74,0x65,0x78,0x50,
    0x61,0x72,0x61,0x6d,0x73,0x5b,0x30,0x5d,0x2e,0x78,0x79,0x2c,0x20,0x30,0x2e,0x30,
    0x2c,0x20,0x30,0x2e,0x30,0x29,0x20,0x2b,0x20,0x28,0x76,0x65,0x72,0x74,0x65,0x78,
    0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x2a,0x20,0x76,0x65,0x63,0x34,0x28,
    0x49,0x6d,0x61,0x67,0x65,0x56,0x65,0x72,0x74,0x65,0x78,0x50,0x61,0x72,0x61,0x6d,
    0x73,0x5b,0x30,0x5d,0x2e,0x7a,0x77,0x2c,0x20,0x31,0x2e,0x30,0x2c,0x20,0x31,0x2e,
    0x30,0x29,0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,0x76,0x65,
    0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x2e,0x78,0x79,0x20,
    0x2b,0x20,0x76,0x65,0x63,0x32,0x28,0x30,0x2e,0x35,0x29,0x3b,0x0a,0x7d,0x0a,0x0a,
    0x00,
};
/*
    #version 300 es
    precision mediump float;
    precision highp int;
    
    uniform highp sampler2D diffuse;
    
    layout(location = 0) out highp vec4 fragColor;
    in highp vec2 uv;
    
    void main()
    {
        fragColor = texture(diffuse, uv);
    }
    
*/
static const char fs_source_glsl300es[219] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x33,0x30,0x30,0x20,0x65,0x73,0x0a,
    0x70,0x72,0x65,0x63,0x69,0x73,0x69,0x6f,0x6e,0x20,0x6d,0x65,0x64,0x69,0x75,0x6d,
    0x70,0x20,0x66,0x6c,0x6f,0x61,0x74,0x3b,0x0a,0x70,0x72,0x65,0x63,0x69,0x73,0x69,
    0x6f,0x6e,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x69,0x6e,0x74,0x3b,0x0a,0x0a,0x75,
    0x6e,0x69,0x66,0x6f,0x72,0x6d,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x73,0x61,0x6d,
    0x70,0x6c,0x65,0x72,0x32,0x44,0x20,0x64,0x69,0x66,0x66,0x75,0x73,0x65,0x3b,0x0a,
    0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,
    0x20,0x3d,0x20,0x30,0x29,0x20,0x6f,0x75,0x74,0x20,0x68,0x69,0x67,0x68,0x70,0x20,
    0x76,0x65,0x63,0x34,0x20,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,
    0x69,0x6e,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,
    0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,
    0x0a,0x20,0x20,0x20,0x20,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,0x72,0x20,0x3d,
    0x20,0x74,0x65,0x78,0x74,0x75,0x72,0x65,0x28,0x64,0x69,0x66,0x66,0x75,0x73,0x65,
    0x2c,0x20,0x75,0x76,0x29,0x3b,0x0a,0x7d,0x0a,0x0a,0x00,
};
/*
    cbuffer ImageVertexParams : register(b0)
    {
        float2 _21_pos : packoffset(c0);
        float2 _21_size : packoffset(c0.z);
    };
    
    
    static float4 gl_Position;
    static float4 vertexPosition;
    static float2 uv;
    
    struct SPIRV_Cross_Input
    {
        float4 vertexPosition : TEXCOORD0;
    };
    
    struct SPIRV_Cross_Output
    {
        float2 uv : TEXCOORD0;
        float4 gl_Position : SV_Position;
    };
    
    #line 17 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/image.glsl"
    void vert_main()
    {
    #line 17 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/image.glsl"
        gl_Position = float4(_21_pos, 0.0f, 0.0f) + (vertexPosition * float4(_21_size, 1.0f, 1.0f));
    #line 18 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/image.glsl"
        uv = vertexPosition.xy + 0.5f.xx;
    }
    
    SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
    {
        vertexPosition = stage_input.vertexPosition;
        vert_main();
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.uv = uv;
        return stage_output;
    }
*/
static const char vs_source_hlsl5[1062] = {
    0x63,0x62,0x75,0x66,0x66,0x65,0x72,0x20,0x49,0x6d,0x61,0x67,0x65,0x56,0x65,0x72,
    0x74,0x65,0x78,0x50,0x61,0x72,0x61,0x6d,0x73,0x20,0x3a,0x20,0x72,0x65,0x67,0x69,
    0x73,0x74,0x65,0x72,0x28,0x62,0x30,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,
    0x6c,0x6f,0x61,0x74,0x32,0x20,0x5f,0x32,0x31,0x5f,0x70,0x6f,0x73,0x20,0x3a,0x20,
    0x70,0x61,0x63,0x6b,0x6f,0x66,0x66,0x73,0x65,0x74,0x28,0x63,0x30,0x29,0x3b,0x0a,
    0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x5f,0x32,0x31,0x5f,0x73,
    0x69,0x7a,0x65,0x20,0x3a,0x20,0x70,0x61,0x63,0x6b,0x6f,0x66,0x66,0x73,0x65,0x74,
    0x28,0x63,0x30,0x2e,0x7a,0x29,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x0a,0x73,0x74,0x61,
    0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x67,0x6c,0x5f,0x50,0x6f,
    0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,
    0x6c,0x6f,0x61,0x74,0x34,0x20,0x76,0x65,0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,
    0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,
    0x61,0x74,0x32,0x20,0x75,0x76,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,
    0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,
    0x74,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x76,
    0x65,0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3a,0x20,
    0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x30,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x73,
    0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,
    0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,
    0x6c,0x6f,0x61,0x74,0x32,0x20,0x75,0x76,0x20,0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,
    0x4f,0x52,0x44,0x30,0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,
    0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3a,0x20,0x53,
    0x56,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,
    0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x37,0x20,0x22,0x43,0x3a,0x5c,0x55,0x73,0x65,
    0x72,0x73,0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,0x69,0x76,
    0x65,0x5c,0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,0x67,0x75,
    0x65,0x43,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,0x6d,0x65,
    0x4f,0x76,0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,0x72,0x73,
    0x2f,0x69,0x6d,0x61,0x67,0x65,0x2e,0x67,0x6c,0x73,0x6c,0x22,0x0a,0x76,0x6f,0x69,
    0x64,0x20,0x76,0x65,0x72,0x74,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,
    0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x37,0x20,0x22,0x43,0x3a,0x5c,0x55,0x73,0x65,
    0x72,0x73,0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,0x69,0x76,
    0x65,0x5c,0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,0x67,0x75,
    0x65,0x43,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,0x6d,0x65,
    0x4f,0x76,0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,0x72,0x73,
    0x2f,0x69,0x6d,0x61,0x67,0x65,0x2e,0x67,0x6c,0x73,0x6c,0x22,0x0a,0x20,0x20,0x20,
    0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x66,
    0x6c,0x6f,0x61,0x74,0x34,0x28,0x5f,0x32,0x31,0x5f,0x70,0x6f,0x73,0x2c,0x20,0x30,
    0x2e,0x30,0x66,0x2c,0x20,0x30,0x2e,0x30,0x66,0x29,0x20,0x2b,0x20,0x28,0x76,0x65,
    0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x2a,0x20,0x66,
    0x6c,0x6f,0x61,0x74,0x34,0x28,0x5f,0x32,0x31,0x5f,0x73,0x69,0x7a,0x65,0x2c,0x20,
    0x31,0x2e,0x30,0x66,0x2c,0x20,0x31,0x2e,0x30,0x66,0x29,0x29,0x3b,0x0a,0x23,0x6c,
    0x69,0x6e,0x65,0x20,0x31,0x38,0x20,0x22,0x43,0x3a,0x5c,0x55,0x73,0x65,0x72,0x73,
    0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,0x69,0x76,0x65,0x5c,
    0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,0x67,0x75,0x65,0x43,
    0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,0x6d,0x65,0x4f,0x76,
    0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,0x72,0x73,0x2f,0x69,
    0x6d,0x61,0x67,0x65,0x2e,0x67,0x6c,0x73,0x6c,0x22,0x0a,0x20,0x20,0x20,0x20,0x75,
    0x76,0x20,0x3d,0x20,0x76,0x65,0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x2e,0x78,0x79,0x20,0x2b,0x20,0x30,0x2e,0x35,0x66,0x2e,0x78,0x78,0x3b,
    0x0a,0x7d,0x0a,0x0a,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,
    0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x6d,0x61,0x69,0x6e,0x28,0x53,0x50,0x49,0x52,
    0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x20,0x73,0x74,
    0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,
    0x20,0x76,0x65,0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,
    0x3d,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x2e,0x76,0x65,
    0x72,0x74,0x65,0x78,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x20,0x20,
    0x20,0x20,0x76,0x65,0x72,0x74,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x3b,0x0a,0x20,
    0x20,0x20,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,
    0x75,0x74,0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,
    0x75,0x74,0x3b,0x0a,0x20,0x20,0x20,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,
    0x74,0x70,0x75,0x74,0x2e,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,
    0x20,0x3d,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,
    0x20,0x20,0x20,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,
    0x2e,0x75,0x76,0x20,0x3d,0x20,0x75,0x76,0x3b,0x0a,0x20,0x20,0x20,0x20,0x72,0x65,
    0x74,0x75,0x72,0x6e,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,
    0x74,0x3b,0x0a,0x7d,0x0a,0x00,
};
/*
    Texture2D<float4> diffuse : register(t0);
    SamplerState _diffuse_sampler : register(s0);
    
    static float4 fragColor;
    static float2 uv;
    
    struct SPIRV_Cross_Input
    {
        float2 uv : TEXCOORD0;
    };
    
    struct SPIRV_Cross_Output
    {
        float4 fragColor : SV_Target0;
    };
    
    #line 13 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/image.glsl"
    void frag_main()
    {
    #line 13 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/image.glsl"
        fragColor = diffuse.Sample(_diffuse_sampler, uv);
    }
    
    SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
    {
        uv = stage_input.uv;
        frag_main();
        SPIRV_Cross_Output stage_output;
        stage_output.fragColor = fragColor;
        return stage_output;
    }
*/
static const char fs_source_hlsl5[724] = {
    0x54,0x65,0x78,0x74,0x75,0x72,0x65,0x32,0x44,0x3c,0x66,0x6c,0x6f,0x61,0x74,0x34,
    0x3e,0x20,0x64,0x69,0x66,0x66,0x75,0x73,0x65,0x20,0x3a,0x20,0x72,0x65,0x67,0x69,
    0x73,0x74,0x65,0x72,0x28,0x74,0x30,0x29,0x3b,0x0a,0x53,0x61,0x6d,0x70,0x6c,0x65,
    0x72,0x53,0x74,0x61,0x74,0x65,0x20,0x5f,0x64,0x69,0x66,0x66,0x75,0x73,0x65,0x5f,
    0x73,0x61,0x6d,0x70,0x6c,0x65,0x72,0x20,0x3a,0x20,0x72,0x65,0x67,0x69,0x73,0x74,
    0x65,0x72,0x28,0x73,0x30,0x29,0x3b,0x0a,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,
    0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,0x72,
    0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,
    0x75,0x76,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,
    0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x75,0x76,0x20,0x3a,0x20,
    0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x30,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x73,
    0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,
    0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,
    0x6c,0x6f,0x61,0x74,0x34,0x20,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,0x72,0x20,
    0x3a,0x20,0x53,0x56,0x5f,0x54,0x61,0x72,0x67,0x65,0x74,0x30,0x3b,0x0a,0x7d,0x3b,
    0x0a,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x33,0x20,0x22,0x43,0x3a,0x5c,0x55,
    0x73,0x65,0x72,0x73,0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,
    0x69,0x76,0x65,0x5c,0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,
    0x67,0x75,0x65,0x43,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,
    0x6d,0x65,0x4f,0x76,0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,
    0x72,0x73,0x2f,0x69,0x6d,0x61,0x67,0x65,0x2e,0x67,0x6c,0x73,0x6c,0x22,0x0a,0x76,
    0x6f,0x69,0x64,0x20,0x66,0x72,0x61,0x67,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,
    0x7b,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x33,0x20,0x22,0x43,0x3a,0x5c,0x55,
    0x73,0x65,0x72,0x73,0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,
    0x69,0x76,0x65,0x5c,0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,
    0x67,0x75,0x65,0x43,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,
    0x6d,0x65,0x4f,0x76,0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,
    0x72,0x73,0x2f,0x69,0x6d,0x61,0x67,0x65,0x2e,0x67,0x6c,0x73,0x6c,0x22,0x0a,0x20,
    0x20,0x20,0x20,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,0x64,
    0x69,0x66,0x66,0x75,0x73,0x65,0x2e,0x53,0x61,0x6d,0x70,0x6c,0x65,0x28,0x5f,0x64,
    0x69,0x66,0x66,0x75,0x73,0x65,0x5f,0x73,0x61,0x6d,0x70,0x6c,0x65,0x72,0x2c,0x20,
    0x75,0x76,0x29,0x3b,0x0a,0x7d,0x0a,0x0a,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,
    0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x6d,0x61,0x69,0x6e,0x28,
    0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,
    0x74,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x29,0x0a,0x7b,
    0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,
    0x69,0x6e,0x70,0x75,0x74,0x2e,0x75,0x76,0x3b,0x0a,0x20,0x20,0x20,0x20,0x66,0x72,
    0x61,0x67,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x3b,0x0a,0x20,0x20,0x20,0x20,0x53,
    0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,
    0x74,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,0x0a,
    0x20,0x20,0x20,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,
    0x2e,0x66,0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,0x66,0x72,0x61,
    0x67,0x43,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x20,0x20,0x20,0x20,0x72,0x65,0x74,0x75,
    0x72,0x6e,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,
    0x0a,0x7d,0x0a,0x00,
};
#if !defined(SOKOL_GFX_INCLUDED)
  #error "Please include sokol_gfx.h before image.hpp"
#endif
static inline const sg_shader_desc* image_shader_desc(sg_backend backend) {
  if (backend == SG_BACKEND_GLCORE33) {
    static sg_shader_desc desc;
    static bool valid;
    if (!valid) {
      valid = true;
      desc.attrs[0].name = "vertexPosition";
      desc.vs.source = vs_source_glsl330;
      desc.vs.entry = "main";
      desc.vs.uniform_blocks[0].size = 16;
      desc.vs.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140;
      desc.vs.uniform_blocks[0].uniforms[0].name = "ImageVertexParams";
      desc.vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT4;
      desc.vs.uniform_blocks[0].uniforms[0].array_count = 1;
      desc.fs.source = fs_source_glsl330;
      desc.fs.entry = "main";
      desc.fs.images[0].name = "diffuse";
      desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
      desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
      desc.label = "image_shader";
    }
    return &desc;
  }
  if (backend == SG_BACKEND_GLES2) {
    static sg_shader_desc desc;
    static bool valid;
    if (!valid) {
      valid = true;
      desc.attrs[0].name = "vertexPosition";
      desc.vs.source = vs_source_glsl100;
      desc.vs.entry = "main";
      desc.vs.uniform_blocks[0].size = 16;
      desc.vs.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140;
      desc.vs.uniform_blocks[0].uniforms[0].name = "ImageVertexParams";
      desc.vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT4;
      desc.vs.uniform_blocks[0].uniforms[0].array_count = 1;
      desc.fs.source = fs_source_glsl100;
      desc.fs.entry = "main";
      desc.fs.images[0].name = "diffuse";
      desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
      desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
      desc.label = "image_shader";
    }
    return &desc;
  }
  if (backend == SG_BACKEND_GLES3) {
    static sg_shader_desc desc;
    static bool valid;
    if (!valid) {
      valid = true;
      desc.attrs[0].name = "vertexPosition";
      desc.vs.source = vs_source_glsl300es;
      desc.vs.entry = "main";
      desc.vs.uniform_blocks[0].size = 16;
      desc.vs.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140;
      desc.vs.uniform_blocks[0].uniforms[0].name = "ImageVertexParams";
      desc.vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT4;
      desc.vs.uniform_blocks[0].uniforms[0].array_count = 1;
      desc.fs.source = fs_source_glsl300es;
      desc.fs.entry = "main";
      desc.fs.images[0].name = "diffuse";
      desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
      desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
      desc.label = "image_shader";
    }
    return &desc;
  }
  if (backend == SG_BACKEND_D3D11) {
    static sg_shader_desc desc;
    static bool valid;
    if (!valid) {
      valid = true;
      desc.attrs[0].sem_name = "TEXCOORD";
      desc.attrs[0].sem_index = 0;
      desc.vs.source = vs_source_hlsl5;
      desc.vs.d3d11_target = "vs_5_0";
      desc.vs.entry = "main";
      desc.vs.uniform_blocks[0].size = 16;
      desc.vs.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_STD140;
      desc.fs.source = fs_source_hlsl5;
      desc.fs.d3d11_target = "ps_5_0";
      desc.fs.entry = "main";
      desc.fs.images[0].name = "diffuse";
      desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
      desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
      desc.label = "image_shader";
    }
    return &desc;
  }
  return 0;
}
