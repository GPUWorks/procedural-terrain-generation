#version 330

in vec2 uv;

out vec3 color;

uniform sampler2D tmp;
uniform sampler2D tmp2;

uniform float tex_width;
uniform float tex_height;

uniform float kernel_size;
uniform float kernel[800];

void main() {

    float weight_tot = 0;
    vec3 color_tot = vec3(0,0,0);

    for(int i = 0; i < kernel_size; i++) {
        float weight = kernel[i];
        vec2 pos = vec2((i - (kernel_size / 2.0)) / (tex_width) , 0.0);
        color_tot += weight * texture(tmp, uv + pos).rgb;
        weight_tot += weight;
    }

    for(int i = 0; i < kernel_size; i++) {
        float weight = kernel[i];
        vec2 pos = vec2(0.0 , (i - (kernel_size / 2.0)) / (tex_height));
        color_tot += weight * texture(tmp2, uv + pos).rgb;
        weight_tot += weight;
    }

    color = color_tot/weight_tot;

}
