#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec2 UV;
}v_out;


uniform sampler2D _FrameBuffer;


void main()
{      
    vec2 uv = v_out.UV;
    uv.y = 1 - uv.y;

    vec3 color = texture(_FrameBuffer, uv).rgb;

    FragColor = vec4(color, 1);

    // Debug to show just uv gradient
    //FragColor = vec4(v_out.UV.x, v_out.UV.y, 0.0f, 1.0f);
}
