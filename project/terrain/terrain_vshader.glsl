#version 330

in vec2 position;

out vec2 uv;
out vec4 vpoint_mv;
out float height;
out vec3 light_dir, view_dir;
out vec3 normal_mv;

uniform sampler2D heightmap;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec3 light_pos;

void main() {
    uv = (position + vec2(2.0)) / 4.0;
    height = max(0.0, texture(heightmap, uv).r);

    mat4 MV = view * model;
    vpoint_mv = MV * vec4(position, height, 1.0);
    gl_Position = projection * vpoint_mv;

    vec3 normal = vec3(0.0, 0.0, 1.0);

    if (height > 0.0) {
        float sx0 = textureOffset(heightmap, uv, ivec2(-1 ,0)).r;
        float sx1 = textureOffset(heightmap, uv, ivec2( 1, 0)).r;
        float sy0 = textureOffset(heightmap, uv, ivec2( 0,-1)).r;
        float sy1 = textureOffset(heightmap, uv, ivec2( 0, 1)).r;

        vec3 vx = vec3(2.0 / 512.0, 0, (sx1 - sx0));
        vec3 vy = vec3(0, 2.0 / 512.0, (sy1 - sy0));

        normal = cross(vx,vy);
    }

    vec4 light_pos_mv = MV * vec4(light_pos, 1.0);

    light_dir = light_pos_mv.xyz - vpoint_mv.xyz;
    view_dir = -vpoint_mv.xyz;
    normal_mv = mat3(inverse(transpose(MV))) * normal;
}
