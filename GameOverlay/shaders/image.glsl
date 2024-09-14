@include types.glsl
@vs vs
in vec4 vertexPosition;
out vec2 uv;

uniform ImageVertexParams
{
	vec2 pos;
	vec2 size;
};

void main()
{
    gl_Position = vec4(pos,0,0) + vertexPosition * vec4(size,1.0,1.0);
	uv = vertexPosition.xy + vec2(0.5);
}
@end

@fs fs

uniform sampler2D diffuse;
in vec2 uv;
out vec4 fragColor;

void main()
{
	fragColor = texture(diffuse, uv);
}
@end

@program image vs fs

