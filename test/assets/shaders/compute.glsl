#type compute
#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba8) uniform image2D img_output;

void main()
{
    // base pixel colour for image
    vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);
    // get index in global work group i.e x,y position
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims         = imageSize(img_output); // fetch image dimensions
    ivec2 pos          = pixel_coords - dims / 2;
    vec2 uv            = vec2(pixel_coords.xy) / dims;
    uv                 = uv * 2 - 1;

    if (length(pos) < 20.0) { pixel.r = 1; }

    imageStore(img_output, pixel_coords, pixel);
}