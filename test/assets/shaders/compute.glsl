//#type compute
#version 430 compatibility

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
layout(rgba8) uniform image2D imgOutput;
uniform int objectBufferCount;
layout(std430, binding = 1) buffer objectBuffer 
{ 
    vec2 data_SSBO[]; 
};

void main()
{
    // base pixel colour for image
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);

    // get index in global work group i.e x,y position
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims         = imageSize(imgOutput);
    ivec2 pos          = pixelCoords - dims / 2;
    vec2 uv            = vec2(pixelCoords.xy / dims);
    uv                 = uv * 2.0 - 1.0;

    for (int i = 0; i < objectBufferCount; i++)
    {
        if (length(pos - data_SSBO[i]) < 20.0) { pixel.r += 0.2; }
    }

    imageStore(imgOutput, pixelCoords, pixel);
}
