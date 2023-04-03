#version 450                          
out vec4 FragColor;

in struct Vertex{
    vec2 UV;
}v_out;


uniform sampler2D _FrameBuffer;
uniform float _PostProcessingMagnitude;
uniform int _PostProcessingEnabled;


void main()
{      
    vec2 uv = v_out.UV;
    uv.y = 1 - uv.y;

    vec3 color = texture(_FrameBuffer, uv).rgb;

    if(_PostProcessingEnabled == 1)
    {
        float avg = (color.r + color.g + color.b) / 3.0f;
        vec3 greyScaleColor = vec3(avg, avg, avg);
    
        color = mix(color, greyScaleColor, _PostProcessingMagnitude);
    }

    FragColor = vec4(color, 1);

    // Debug to show just uv gradient
    //FragColor = vec4(v_out.UV.x, v_out.UV.y, 0.0f, 1.0f);
}