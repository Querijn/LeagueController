@include types.glsl

@vs vs
uniform vs_params
{
    mat4 mvp;
    float hoverTime;
    float blend;
};

in vec2 pos;
out float alpha;
out float colour;

void main()
{
    gl_Position = mvp * vec4(pos, 0, 1);
    alpha = blend;
    colour = hoverTime * 0.3;
}
@end

@fs fs
uniform sampler2D tex;

in float alpha;
in float colour;
out vec4 frag_color;

void main()
{
    frag_color = vec4(colour, colour, colour, alpha);
}
@end

@program texcube vs fs