@include types.glsl
@vs vs
in vec4 vertexPosition;
out vec2 uv;

uniform SkermImageVertexParams
{
	uniform mat4 mvp;
	uniform vec4 spriteExtents;
	uniform vec2 pivot;
};

void main()
{
	uv = spriteExtents.xy + vertexPosition.xy * spriteExtents.zw;
	gl_Position = mvp * vec4(vertexPosition.xy - pivot.xy, 0.0, 1.0);
}
@end

@fs fs

uniform SkermImageFragmentParams
{
	uniform vec4 color;
};
uniform sampler2D diffuse;

in vec2 uv;
out vec4 fragColor;

void main()
{
	fragColor = texture(diffuse, uv) * color;
}
@end

@program SkermImage vs fs

