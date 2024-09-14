#pragma once
/*
    #version:1# (machine generated, don't edit!)

    Generated by sokol-shdc (https://github.com/floooh/sokol-tools)

    Cmdline: sokol-shdc --input C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/debug_renderer.glsl --output C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/src/shaders/debug_renderer.hpp --slang glsl330:glsl300es:hlsl5:glsl100

    Overview:

        Shader program 'DebugRenderer':
            Get shader desc: DebugRenderer_shader_desc(sg_query_backend());
            Vertex shader: vs
                Attribute slots:
                    ATTR_vs_position = 0
            Fragment shader: fs
                Image 'tex':
                    Type: SG_IMAGETYPE_2D
                    Component Type: SG_SAMPLERTYPE_FLOAT
                    Bind slot: SLOT_tex = 0


    Shader descriptor structs:

        sg_shader DebugRenderer = sg_make_shader(DebugRenderer_shader_desc(sg_query_backend()));

    Vertex attribute locations for vertex shader 'vs':

        sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
            .layout = {
                .attrs = {
                    [ATTR_vs_position] = { ... },
                },
            },
            ...});

    Image bind slots, use as index in sg_bindings.vs_images[] or .fs_images[]

        SLOT_tex = 0;

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
#define ATTR_vs_position (0)
#define SLOT_tex (0)
/*
    #version 330
    
    out vec2 uv;
    layout(location = 0) in vec4 position;
    
    void main()
    {
        uv = (position.xy + vec2(1.0)) * 0.5;
        uv.y = 1.0 - uv.y;
        gl_Position = position;
    }
    
*/
static const char vs_source_glsl330[178] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x33,0x33,0x30,0x0a,0x0a,0x6f,0x75,
    0x74,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,0x0a,0x6c,0x61,0x79,0x6f,0x75,
    0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,0x29,0x20,
    0x69,0x6e,0x20,0x76,0x65,0x63,0x34,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,
    0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,
    0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,0x28,0x70,0x6f,0x73,0x69,0x74,
    0x69,0x6f,0x6e,0x2e,0x78,0x79,0x20,0x2b,0x20,0x76,0x65,0x63,0x32,0x28,0x31,0x2e,
    0x30,0x29,0x29,0x20,0x2a,0x20,0x30,0x2e,0x35,0x3b,0x0a,0x20,0x20,0x20,0x20,0x75,
    0x76,0x2e,0x79,0x20,0x3d,0x20,0x31,0x2e,0x30,0x20,0x2d,0x20,0x75,0x76,0x2e,0x79,
    0x3b,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,
    0x6e,0x20,0x3d,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x7d,0x0a,
    0x0a,0x00,
};
/*
    #version 330
    
    uniform sampler2D tex;
    
    layout(location = 0) out vec4 frag_color;
    in vec2 uv;
    
    void main()
    {
        frag_color = texture(tex, uv);
    }
    
*/
static const char fs_source_glsl330[146] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x33,0x33,0x30,0x0a,0x0a,0x75,0x6e,
    0x69,0x66,0x6f,0x72,0x6d,0x20,0x73,0x61,0x6d,0x70,0x6c,0x65,0x72,0x32,0x44,0x20,
    0x74,0x65,0x78,0x3b,0x0a,0x0a,0x6c,0x61,0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,
    0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,0x29,0x20,0x6f,0x75,0x74,0x20,0x76,
    0x65,0x63,0x34,0x20,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,
    0x69,0x6e,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,0x0a,0x0a,0x76,0x6f,0x69,
    0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,
    0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,0x74,0x65,0x78,0x74,
    0x75,0x72,0x65,0x28,0x74,0x65,0x78,0x2c,0x20,0x75,0x76,0x29,0x3b,0x0a,0x7d,0x0a,
    0x0a,0x00,
};
/*
    #version 100
    
    varying vec2 uv;
    attribute vec4 position;
    
    void main()
    {
        uv = (position.xy + vec2(1.0)) * 0.5;
        uv.y = 1.0 - uv.y;
        gl_Position = position;
    }
    
*/
static const char vs_source_glsl100[168] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x31,0x30,0x30,0x0a,0x0a,0x76,0x61,
    0x72,0x79,0x69,0x6e,0x67,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,0x0a,0x61,
    0x74,0x74,0x72,0x69,0x62,0x75,0x74,0x65,0x20,0x76,0x65,0x63,0x34,0x20,0x70,0x6f,
    0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,
    0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,
    0x28,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x2e,0x78,0x79,0x20,0x2b,0x20,0x76,
    0x65,0x63,0x32,0x28,0x31,0x2e,0x30,0x29,0x29,0x20,0x2a,0x20,0x30,0x2e,0x35,0x3b,
    0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x2e,0x79,0x20,0x3d,0x20,0x31,0x2e,0x30,0x20,
    0x2d,0x20,0x75,0x76,0x2e,0x79,0x3b,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x50,
    0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x3b,0x0a,0x7d,0x0a,0x0a,0x00,
};
/*
    #version 100
    precision mediump float;
    precision highp int;
    
    uniform highp sampler2D tex;
    
    varying highp vec2 uv;
    
    void main()
    {
        gl_FragData[0] = texture2D(tex, uv);
    }
    
*/
static const char fs_source_glsl100[173] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x31,0x30,0x30,0x0a,0x70,0x72,0x65,
    0x63,0x69,0x73,0x69,0x6f,0x6e,0x20,0x6d,0x65,0x64,0x69,0x75,0x6d,0x70,0x20,0x66,
    0x6c,0x6f,0x61,0x74,0x3b,0x0a,0x70,0x72,0x65,0x63,0x69,0x73,0x69,0x6f,0x6e,0x20,
    0x68,0x69,0x67,0x68,0x70,0x20,0x69,0x6e,0x74,0x3b,0x0a,0x0a,0x75,0x6e,0x69,0x66,
    0x6f,0x72,0x6d,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x73,0x61,0x6d,0x70,0x6c,0x65,
    0x72,0x32,0x44,0x20,0x74,0x65,0x78,0x3b,0x0a,0x0a,0x76,0x61,0x72,0x79,0x69,0x6e,
    0x67,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,
    0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,
    0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x46,0x72,0x61,0x67,0x44,0x61,0x74,0x61,0x5b,
    0x30,0x5d,0x20,0x3d,0x20,0x74,0x65,0x78,0x74,0x75,0x72,0x65,0x32,0x44,0x28,0x74,
    0x65,0x78,0x2c,0x20,0x75,0x76,0x29,0x3b,0x0a,0x7d,0x0a,0x0a,0x00,
};
/*
    #version 300 es
    
    out vec2 uv;
    layout(location = 0) in vec4 position;
    
    void main()
    {
        uv = (position.xy + vec2(1.0)) * 0.5;
        uv.y = 1.0 - uv.y;
        gl_Position = position;
    }
    
*/
static const char vs_source_glsl300es[181] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x33,0x30,0x30,0x20,0x65,0x73,0x0a,
    0x0a,0x6f,0x75,0x74,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,0x0a,0x6c,0x61,
    0x79,0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,
    0x30,0x29,0x20,0x69,0x6e,0x20,0x76,0x65,0x63,0x34,0x20,0x70,0x6f,0x73,0x69,0x74,
    0x69,0x6f,0x6e,0x3b,0x0a,0x0a,0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,
    0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,0x28,0x70,0x6f,
    0x73,0x69,0x74,0x69,0x6f,0x6e,0x2e,0x78,0x79,0x20,0x2b,0x20,0x76,0x65,0x63,0x32,
    0x28,0x31,0x2e,0x30,0x29,0x29,0x20,0x2a,0x20,0x30,0x2e,0x35,0x3b,0x0a,0x20,0x20,
    0x20,0x20,0x75,0x76,0x2e,0x79,0x20,0x3d,0x20,0x31,0x2e,0x30,0x20,0x2d,0x20,0x75,
    0x76,0x2e,0x79,0x3b,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,
    0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,
    0x0a,0x7d,0x0a,0x0a,0x00,
};
/*
    #version 300 es
    precision mediump float;
    precision highp int;
    
    uniform highp sampler2D tex;
    
    layout(location = 0) out highp vec4 frag_color;
    in highp vec2 uv;
    
    void main()
    {
        frag_color = texture(tex, uv);
    }
    
*/
static const char fs_source_glsl300es[213] = {
    0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,0x33,0x30,0x30,0x20,0x65,0x73,0x0a,
    0x70,0x72,0x65,0x63,0x69,0x73,0x69,0x6f,0x6e,0x20,0x6d,0x65,0x64,0x69,0x75,0x6d,
    0x70,0x20,0x66,0x6c,0x6f,0x61,0x74,0x3b,0x0a,0x70,0x72,0x65,0x63,0x69,0x73,0x69,
    0x6f,0x6e,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x69,0x6e,0x74,0x3b,0x0a,0x0a,0x75,
    0x6e,0x69,0x66,0x6f,0x72,0x6d,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x73,0x61,0x6d,
    0x70,0x6c,0x65,0x72,0x32,0x44,0x20,0x74,0x65,0x78,0x3b,0x0a,0x0a,0x6c,0x61,0x79,
    0x6f,0x75,0x74,0x28,0x6c,0x6f,0x63,0x61,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x30,
    0x29,0x20,0x6f,0x75,0x74,0x20,0x68,0x69,0x67,0x68,0x70,0x20,0x76,0x65,0x63,0x34,
    0x20,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x69,0x6e,0x20,
    0x68,0x69,0x67,0x68,0x70,0x20,0x76,0x65,0x63,0x32,0x20,0x75,0x76,0x3b,0x0a,0x0a,
    0x76,0x6f,0x69,0x64,0x20,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,0x20,0x20,
    0x20,0x20,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,0x74,
    0x65,0x78,0x74,0x75,0x72,0x65,0x28,0x74,0x65,0x78,0x2c,0x20,0x75,0x76,0x29,0x3b,
    0x0a,0x7d,0x0a,0x0a,0x00,
};
/*
    static float4 gl_Position;
    static float2 uv;
    static float4 position;
    
    struct SPIRV_Cross_Input
    {
        float4 position : TEXCOORD0;
    };
    
    struct SPIRV_Cross_Output
    {
        float2 uv : TEXCOORD0;
        float4 gl_Position : SV_Position;
    };
    
    #line 12 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/debug_renderer.glsl"
    void vert_main()
    {
    #line 12 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/debug_renderer.glsl"
        uv = (position.xy + 1.0f.xx) * 0.5f;
    #line 13 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/debug_renderer.glsl"
        uv.y = 1.0f - uv.y;
    #line 14 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/debug_renderer.glsl"
        gl_Position = position;
    }
    
    SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
    {
        position = stage_input.position;
        vert_main();
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.uv = uv;
        return stage_output;
    }
*/
static const char vs_source_hlsl5[1000] = {
    0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x67,0x6c,
    0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,
    0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x75,0x76,0x3b,0x0a,0x73,0x74,0x61,
    0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x70,0x6f,0x73,0x69,0x74,
    0x69,0x6f,0x6e,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,
    0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x0a,0x7b,
    0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x70,0x6f,0x73,0x69,
    0x74,0x69,0x6f,0x6e,0x20,0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x30,
    0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,
    0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x0a,
    0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x75,0x76,0x20,
    0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,0x44,0x30,0x3b,0x0a,0x20,0x20,0x20,
    0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,
    0x69,0x6f,0x6e,0x20,0x3a,0x20,0x53,0x56,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,
    0x6e,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x32,0x20,
    0x22,0x43,0x3a,0x5c,0x55,0x73,0x65,0x72,0x73,0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,
    0x4f,0x6e,0x65,0x44,0x72,0x69,0x76,0x65,0x5c,0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,
    0x73,0x5c,0x4c,0x65,0x61,0x67,0x75,0x65,0x43,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,
    0x65,0x72,0x5c,0x47,0x61,0x6d,0x65,0x4f,0x76,0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,
    0x73,0x68,0x61,0x64,0x65,0x72,0x73,0x2f,0x64,0x65,0x62,0x75,0x67,0x5f,0x72,0x65,
    0x6e,0x64,0x65,0x72,0x65,0x72,0x2e,0x67,0x6c,0x73,0x6c,0x22,0x0a,0x76,0x6f,0x69,
    0x64,0x20,0x76,0x65,0x72,0x74,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x0a,0x7b,0x0a,
    0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x32,0x20,0x22,0x43,0x3a,0x5c,0x55,0x73,0x65,
    0x72,0x73,0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,0x69,0x76,
    0x65,0x5c,0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,0x67,0x75,
    0x65,0x43,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,0x6d,0x65,
    0x4f,0x76,0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,0x72,0x73,
    0x2f,0x64,0x65,0x62,0x75,0x67,0x5f,0x72,0x65,0x6e,0x64,0x65,0x72,0x65,0x72,0x2e,
    0x67,0x6c,0x73,0x6c,0x22,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,0x28,
    0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x2e,0x78,0x79,0x20,0x2b,0x20,0x31,0x2e,
    0x30,0x66,0x2e,0x78,0x78,0x29,0x20,0x2a,0x20,0x30,0x2e,0x35,0x66,0x3b,0x0a,0x23,
    0x6c,0x69,0x6e,0x65,0x20,0x31,0x33,0x20,0x22,0x43,0x3a,0x5c,0x55,0x73,0x65,0x72,
    0x73,0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,0x69,0x76,0x65,
    0x5c,0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,0x67,0x75,0x65,
    0x43,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,0x6d,0x65,0x4f,
    0x76,0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,0x72,0x73,0x2f,
    0x64,0x65,0x62,0x75,0x67,0x5f,0x72,0x65,0x6e,0x64,0x65,0x72,0x65,0x72,0x2e,0x67,
    0x6c,0x73,0x6c,0x22,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x2e,0x79,0x20,0x3d,0x20,
    0x31,0x2e,0x30,0x66,0x20,0x2d,0x20,0x75,0x76,0x2e,0x79,0x3b,0x0a,0x23,0x6c,0x69,
    0x6e,0x65,0x20,0x31,0x34,0x20,0x22,0x43,0x3a,0x5c,0x55,0x73,0x65,0x72,0x73,0x5c,
    0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,0x69,0x76,0x65,0x5c,0x50,
    0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,0x67,0x75,0x65,0x43,0x6f,
    0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,0x6d,0x65,0x4f,0x76,0x65,
    0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,0x72,0x73,0x2f,0x64,0x65,
    0x62,0x75,0x67,0x5f,0x72,0x65,0x6e,0x64,0x65,0x72,0x65,0x72,0x2e,0x67,0x6c,0x73,
    0x6c,0x22,0x0a,0x20,0x20,0x20,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x20,0x3d,0x20,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,0x7d,
    0x0a,0x0a,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,
    0x74,0x70,0x75,0x74,0x20,0x6d,0x61,0x69,0x6e,0x28,0x53,0x50,0x49,0x52,0x56,0x5f,
    0x43,0x72,0x6f,0x73,0x73,0x5f,0x49,0x6e,0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,
    0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x70,
    0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x20,0x3d,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,
    0x69,0x6e,0x70,0x75,0x74,0x2e,0x70,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x3b,0x0a,
    0x20,0x20,0x20,0x20,0x76,0x65,0x72,0x74,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x3b,
    0x0a,0x20,0x20,0x20,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,
    0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,
    0x74,0x70,0x75,0x74,0x3b,0x0a,0x20,0x20,0x20,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,
    0x6f,0x75,0x74,0x70,0x75,0x74,0x2e,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,
    0x6f,0x6e,0x20,0x3d,0x20,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,
    0x3b,0x0a,0x20,0x20,0x20,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,0x70,
    0x75,0x74,0x2e,0x75,0x76,0x20,0x3d,0x20,0x75,0x76,0x3b,0x0a,0x20,0x20,0x20,0x20,
    0x72,0x65,0x74,0x75,0x72,0x6e,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,
    0x70,0x75,0x74,0x3b,0x0a,0x7d,0x0a,0x00,
};
/*
    Texture2D<float4> tex : register(t0);
    SamplerState _tex_sampler : register(s0);
    
    static float4 frag_color;
    static float2 uv;
    
    struct SPIRV_Cross_Input
    {
        float2 uv : TEXCOORD0;
    };
    
    struct SPIRV_Cross_Output
    {
        float4 frag_color : SV_Target0;
    };
    
    #line 13 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/debug_renderer.glsl"
    void frag_main()
    {
    #line 13 "C:\Users\queri\OneDrive\Projects\LeagueController\GameOverlay\/shaders/debug_renderer.glsl"
        frag_color = tex.Sample(_tex_sampler, uv);
    }
    
    SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
    {
        uv = stage_input.uv;
        frag_main();
        SPIRV_Cross_Output stage_output;
        stage_output.frag_color = frag_color;
        return stage_output;
    }
*/
static const char fs_source_hlsl5[731] = {
    0x54,0x65,0x78,0x74,0x75,0x72,0x65,0x32,0x44,0x3c,0x66,0x6c,0x6f,0x61,0x74,0x34,
    0x3e,0x20,0x74,0x65,0x78,0x20,0x3a,0x20,0x72,0x65,0x67,0x69,0x73,0x74,0x65,0x72,
    0x28,0x74,0x30,0x29,0x3b,0x0a,0x53,0x61,0x6d,0x70,0x6c,0x65,0x72,0x53,0x74,0x61,
    0x74,0x65,0x20,0x5f,0x74,0x65,0x78,0x5f,0x73,0x61,0x6d,0x70,0x6c,0x65,0x72,0x20,
    0x3a,0x20,0x72,0x65,0x67,0x69,0x73,0x74,0x65,0x72,0x28,0x73,0x30,0x29,0x3b,0x0a,
    0x0a,0x73,0x74,0x61,0x74,0x69,0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x66,
    0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x73,0x74,0x61,0x74,0x69,
    0x63,0x20,0x66,0x6c,0x6f,0x61,0x74,0x32,0x20,0x75,0x76,0x3b,0x0a,0x0a,0x73,0x74,
    0x72,0x75,0x63,0x74,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,
    0x5f,0x49,0x6e,0x70,0x75,0x74,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,
    0x61,0x74,0x32,0x20,0x75,0x76,0x20,0x3a,0x20,0x54,0x45,0x58,0x43,0x4f,0x4f,0x52,
    0x44,0x30,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x73,0x74,0x72,0x75,0x63,0x74,0x20,0x53,
    0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,
    0x74,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x66,0x6c,0x6f,0x61,0x74,0x34,0x20,0x66,
    0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x20,0x3a,0x20,0x53,0x56,0x5f,0x54,
    0x61,0x72,0x67,0x65,0x74,0x30,0x3b,0x0a,0x7d,0x3b,0x0a,0x0a,0x23,0x6c,0x69,0x6e,
    0x65,0x20,0x31,0x33,0x20,0x22,0x43,0x3a,0x5c,0x55,0x73,0x65,0x72,0x73,0x5c,0x71,
    0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,0x65,0x44,0x72,0x69,0x76,0x65,0x5c,0x50,0x72,
    0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,0x4c,0x65,0x61,0x67,0x75,0x65,0x43,0x6f,0x6e,
    0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,0x5c,0x47,0x61,0x6d,0x65,0x4f,0x76,0x65,0x72,
    0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,0x61,0x64,0x65,0x72,0x73,0x2f,0x64,0x65,0x62,
    0x75,0x67,0x5f,0x72,0x65,0x6e,0x64,0x65,0x72,0x65,0x72,0x2e,0x67,0x6c,0x73,0x6c,
    0x22,0x0a,0x76,0x6f,0x69,0x64,0x20,0x66,0x72,0x61,0x67,0x5f,0x6d,0x61,0x69,0x6e,
    0x28,0x29,0x0a,0x7b,0x0a,0x23,0x6c,0x69,0x6e,0x65,0x20,0x31,0x33,0x20,0x22,0x43,
    0x3a,0x5c,0x55,0x73,0x65,0x72,0x73,0x5c,0x71,0x75,0x65,0x72,0x69,0x5c,0x4f,0x6e,
    0x65,0x44,0x72,0x69,0x76,0x65,0x5c,0x50,0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x5c,
    0x4c,0x65,0x61,0x67,0x75,0x65,0x43,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x6c,0x65,0x72,
    0x5c,0x47,0x61,0x6d,0x65,0x4f,0x76,0x65,0x72,0x6c,0x61,0x79,0x5c,0x2f,0x73,0x68,
    0x61,0x64,0x65,0x72,0x73,0x2f,0x64,0x65,0x62,0x75,0x67,0x5f,0x72,0x65,0x6e,0x64,
    0x65,0x72,0x65,0x72,0x2e,0x67,0x6c,0x73,0x6c,0x22,0x0a,0x20,0x20,0x20,0x20,0x66,
    0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x20,0x3d,0x20,0x74,0x65,0x78,0x2e,
    0x53,0x61,0x6d,0x70,0x6c,0x65,0x28,0x5f,0x74,0x65,0x78,0x5f,0x73,0x61,0x6d,0x70,
    0x6c,0x65,0x72,0x2c,0x20,0x75,0x76,0x29,0x3b,0x0a,0x7d,0x0a,0x0a,0x53,0x50,0x49,
    0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,0x4f,0x75,0x74,0x70,0x75,0x74,0x20,
    0x6d,0x61,0x69,0x6e,0x28,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,
    0x5f,0x49,0x6e,0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,
    0x75,0x74,0x29,0x0a,0x7b,0x0a,0x20,0x20,0x20,0x20,0x75,0x76,0x20,0x3d,0x20,0x73,
    0x74,0x61,0x67,0x65,0x5f,0x69,0x6e,0x70,0x75,0x74,0x2e,0x75,0x76,0x3b,0x0a,0x20,
    0x20,0x20,0x20,0x66,0x72,0x61,0x67,0x5f,0x6d,0x61,0x69,0x6e,0x28,0x29,0x3b,0x0a,
    0x20,0x20,0x20,0x20,0x53,0x50,0x49,0x52,0x56,0x5f,0x43,0x72,0x6f,0x73,0x73,0x5f,
    0x4f,0x75,0x74,0x70,0x75,0x74,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,0x75,0x74,
    0x70,0x75,0x74,0x3b,0x0a,0x20,0x20,0x20,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,0x6f,
    0x75,0x74,0x70,0x75,0x74,0x2e,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,
    0x20,0x3d,0x20,0x66,0x72,0x61,0x67,0x5f,0x63,0x6f,0x6c,0x6f,0x72,0x3b,0x0a,0x20,
    0x20,0x20,0x20,0x72,0x65,0x74,0x75,0x72,0x6e,0x20,0x73,0x74,0x61,0x67,0x65,0x5f,
    0x6f,0x75,0x74,0x70,0x75,0x74,0x3b,0x0a,0x7d,0x0a,0x00,
};
#if !defined(SOKOL_GFX_INCLUDED)
  #error "Please include sokol_gfx.h before debug_renderer.hpp"
#endif
static inline const sg_shader_desc* DebugRenderer_shader_desc(sg_backend backend) {
  if (backend == SG_BACKEND_GLCORE33) {
    static sg_shader_desc desc;
    static bool valid;
    if (!valid) {
      valid = true;
      desc.attrs[0].name = "position";
      desc.vs.source = vs_source_glsl330;
      desc.vs.entry = "main";
      desc.fs.source = fs_source_glsl330;
      desc.fs.entry = "main";
      desc.fs.images[0].name = "tex";
      desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
      desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
      desc.label = "DebugRenderer_shader";
    }
    return &desc;
  }
  if (backend == SG_BACKEND_GLES2) {
    static sg_shader_desc desc;
    static bool valid;
    if (!valid) {
      valid = true;
      desc.attrs[0].name = "position";
      desc.vs.source = vs_source_glsl100;
      desc.vs.entry = "main";
      desc.fs.source = fs_source_glsl100;
      desc.fs.entry = "main";
      desc.fs.images[0].name = "tex";
      desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
      desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
      desc.label = "DebugRenderer_shader";
    }
    return &desc;
  }
  if (backend == SG_BACKEND_GLES3) {
    static sg_shader_desc desc;
    static bool valid;
    if (!valid) {
      valid = true;
      desc.attrs[0].name = "position";
      desc.vs.source = vs_source_glsl300es;
      desc.vs.entry = "main";
      desc.fs.source = fs_source_glsl300es;
      desc.fs.entry = "main";
      desc.fs.images[0].name = "tex";
      desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
      desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
      desc.label = "DebugRenderer_shader";
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
      desc.fs.source = fs_source_hlsl5;
      desc.fs.d3d11_target = "ps_5_0";
      desc.fs.entry = "main";
      desc.fs.images[0].name = "tex";
      desc.fs.images[0].image_type = SG_IMAGETYPE_2D;
      desc.fs.images[0].sampler_type = SG_SAMPLERTYPE_FLOAT;
      desc.label = "DebugRenderer_shader";
    }
    return &desc;
  }
  return 0;
}
