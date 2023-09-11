#version 300 es

precision mediump float;

uniform sampler2D textureDiff;

uniform vec3 matSpec, matAmbi, matEmit;
uniform float matSh;
uniform vec3 srcDiff, srcSpec, srcAmbi;

in vec3 v_normal, v_view, v_lightDir;
in vec2 v_texCoord;

layout(location = 0) out vec4 fragColor;

void main() {

    float alpha = 1.0;
    vec3 color = texture(textureDiff, v_texCoord).rgb;

    //////////////////////////////
    /* TODO: Problem 3 : Implement Phong lighting
     *  Fill in the lines below.
     */

     //normalization
     vec3 normal = normalize(v_normal);
     vec3 view = normalize(v_view);
     vec3 light = normalize(v_lightDir);

     // diffuse term
     vec3 matDiff = color;
     vec3 diff = max(dot(normal, light), 0.0) * srcDiff * matDiff;

     // specular term
     vec3 refl = 2.0 * normal * dot(normal, light) - light;
     vec3 spec = pow(max(dot(refl, view), 0.0), matSh) * srcSpec * matSpec;

     // ambient term
     vec3 ambi = srcAmbi * matAmbi;
     color = diff + spec + ambi + matEmit;

    //////////////////////////////

    //////////////////////////////
    /* TODO: Problem 4 : Implement alpha blending
     *  Fill in the lines below.
     */
    alpha = sqrt(pow(color.r, 2.0) + pow(color.g, 2.0) + pow(color.b, 2.0));
    //////////////////////////////

    fragColor = vec4(color, alpha);
}