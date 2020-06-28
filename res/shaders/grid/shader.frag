#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform GridParam {
    mat4 invMVP;
    mat4 model;
    vec4 color1;
    vec4 color2;
    vec2 offset;
    vec2 tileSize;
    vec2 screenSize;
    float extended;
} gridParam;

// Convenient function to check if a vector is inside a box
float insideBox(vec2 v, vec2 bottomLeft, vec2 topRight) {
    vec2 s = step(bottomLeft, v) - step(topRight, v);
    return s.x * s.y;   
}

void main() {

    // Get the xyw coordinates on screen
    vec4 z = vec4(gl_FragCoord.xy * 2.0 / gridParam.screenSize - 1.0, 1.0, 1.0);
    // Determine the z-coordinate in NDS that correspond to the z-coordinates on the grid (z=0, model-wise)
    z[2] = -(gridParam.invMVP[0][2] * z[0] + gridParam.invMVP[1][2] * z[1] + gridParam.invMVP[3][2] * z[3]) / gridParam.invMVP[2][2];
    
    vec4 hpos = gridParam.invMVP * z;
    vec2 pos = hpos.xy / hpos.w;

    if (gridParam.extended == 0.0) {
        if (insideBox(pos, vec2(-1.0, -1.0), vec2(1.0, 1.0)) == 0.0) {
            discard;
        }
    }

    // Position on the grid
    pos = (vec4(pos, 1.0, 1.0) * gridParam.model).xy - gridParam.offset - (vec4(-1.0, -1.0, 1.0, 1.0) * gridParam.model).xy;
    
    // Tile number
    float s = dot(floor(pos / gridParam.tileSize), vec2(1.0));

    if (mod(mod(s, 2) + 2, 2) == 0) {
        outColor = gridParam.color1;
    } else {
        outColor = gridParam.color2;
    }

}
