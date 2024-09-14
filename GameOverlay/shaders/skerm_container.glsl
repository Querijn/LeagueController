@include types.glsl
@vs SkermContainerVertex
in vec4 vertexPosition;

uniform SkermContainerVertexParams
{
	uniform mat4 mvp;
};

void main()
{
	gl_Position = mvp * vec4(vertexPosition.xy, 0.0, 1.0);
}
@end

@fs SkermContainerFragment

uniform SkermContainerFragmentParams
{
	uniform vec4 color;
};

out vec4 fragColor;

void main()
{
	fragColor = color;
}
@end

@program SkermContainer SkermContainerVertex SkermContainerFragment

