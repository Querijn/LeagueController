@vs vs
in vec4 position;

out vec2 uv;

void main()
{
    uv = (position.xy + 1) * 0.5;
    uv.y = 1.0 - uv.y;
    gl_Position = position;
}
@end

//--- fragment shader
@fs fs
uniform sampler2D tex;

in vec2 uv;
out vec4 frag_color;

void main()
{
    frag_color = texture(tex, uv);
}
@end

@program DebugRenderer vs fs
