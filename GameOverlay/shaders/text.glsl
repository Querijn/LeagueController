@include types.glsl

@vs TextVertexShader
in vec2 Positions;
in vec2 UVs;

out vec2 UV;

uniform TextVertexParams
{
	mat4 MVP;
};

void main()
{
	gl_Position = MVP * vec4(Positions, 0.0, 1.0);
	UV = UVs;
}
@end

@fs TextFragmentShader
out vec4 fragColor;

in vec2 UV;

uniform sampler2D Diffuse;
uniform TextFragmentParams
{
	vec4 Color;
};

void main()
{
	float alpha = texture(Diffuse, UV).r;
	fragColor = vec4(Color.xyz, alpha);
}
@end

@program text TextVertexShader TextFragmentShader

