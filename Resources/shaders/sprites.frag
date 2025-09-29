#version 460 core

struct ShaderLightingInfo {
	float r, g, b;
	float global_light_impact;
	float source_light_impact;
};

layout(std430) buffer SpriteLightsSSBO {
    ShaderLightingInfo lightMap[];
};

out vec4 fragColor;

in vec2 texCoord;
in float texIndex;
in vec2 globalCoord;

uniform sampler2D u_textures[2];
uniform float day_ratio;
uniform float blockSize;
uniform ivec2 worldSize;

vec3 getLightColor(vec2 worldPosition) {
    ivec2 blockCoord = ivec2(worldPosition.xy / blockSize);
    int index = worldSize.x * blockCoord.y + blockCoord.x;
    return clamp(vec3(lightMap[index].r, lightMap[index].g, lightMap[index].b) * lightMap[index].global_light_impact, 0.0, 1.0);
}

ShaderLightingInfo getBilinearCustomLightInfo(vec2 worldPosition) {
    ivec2 blockCoord = ivec2(floor(worldPosition.xy / blockSize));
    vec2 frac = mod(worldPosition.xy, blockSize) / blockSize;
    float reducedPrecision = 0.2;
    frac = vec2(floor(frac.x / reducedPrecision + 0.5) * reducedPrecision, floor(frac.y / reducedPrecision + 0.5) * reducedPrecision);

    ivec2 nearestBlockCoord;
    if (frac.x > 0.5) {
        nearestBlockCoord.x = blockCoord.x + 1;
        frac.x -= 0.5;
    } else {
        nearestBlockCoord.x = blockCoord.x - 1;
        frac.x = 0.5 - frac.x;
    }
    if (frac.y > 0.5) {
        nearestBlockCoord.y = blockCoord.y + 1;
        frac.y -= 0.5;
    } else {
        nearestBlockCoord.y = blockCoord.y - 1;
        frac.y = 0.5 - frac.y;
    }

    int index00 = blockCoord.y * worldSize.x + blockCoord.x;
    int index10 = blockCoord.y * worldSize.x + (nearestBlockCoord.x);
    int index01 = (nearestBlockCoord.y) * worldSize.x + blockCoord.x;
    int index11 = (nearestBlockCoord.y) * worldSize.x + (nearestBlockCoord.x);

    ShaderLightingInfo light00 = lightMap[index00];
    ShaderLightingInfo light10 = (nearestBlockCoord.x >= 0 && nearestBlockCoord.x < worldSize.x) ? lightMap[index10] : light00;
    ShaderLightingInfo light01 = (nearestBlockCoord.y >= 0 && nearestBlockCoord.y < worldSize.y) ? lightMap[index01] : light00;
    ShaderLightingInfo light11 = (nearestBlockCoord.x >= 0 && nearestBlockCoord.x < worldSize.x && nearestBlockCoord.y >= 0 && nearestBlockCoord.y < worldSize.y) ? lightMap[index11] : light01;

    vec4 l00 = vec4(light00.r, light00.g, light00.b, light00.global_light_impact);
    vec4 l10 = vec4(light10.r, light10.g, light10.b, light10.global_light_impact);
    vec4 l01 = vec4(light01.r, light01.g, light01.b, light01.global_light_impact);
    vec4 l11 = vec4(light11.r, light11.g, light11.b, light11.global_light_impact);

    vec4 lightTop = mix(l00, l10, frac.x);
    vec4 lightBottom = mix(l01, l11, frac.x);
    vec4 lightFinal = clamp(mix(lightTop, lightBottom, frac.y), 0.0, 1.0);

    float sourceLightImpactTop = mix(light00.source_light_impact, light10.source_light_impact, frac.x);
    float sourceLightImpactBottom = mix(light01.source_light_impact, light11.source_light_impact, frac.x);
    float sourceLightImpactFinal = clamp(mix(sourceLightImpactTop, sourceLightImpactBottom, frac.y), 0.0, 1.0);

    return ShaderLightingInfo(lightFinal.x, lightFinal.y, lightFinal.z, lightFinal.w, sourceLightImpactFinal);
}

void main()
{
    int index = int(texIndex);
    if(texture(u_textures[index], texCoord).a < 0.1) discard;

    vec4 texColor = texture(u_textures[index], texCoord);
    vec3 color = texColor.rgb;
    float alpha = texColor.a;

    float global_light_level = day_ratio;
    float source_light_level = day_ratio;
    if (global_light_level < 0.02) global_light_level = 0.02;
    if (int(globalCoord.y / blockSize) < worldSize.y * 0.68) {
        global_light_level = 0.0;
        source_light_level = 0.0;
    }

    //default texture color based on ambient lighting
    ShaderLightingInfo bilinearColor = getBilinearCustomLightInfo(globalCoord);
    vec3 result = global_light_level * color * alpha * bilinearColor.global_light_impact;

    //add light color from source lights
    result += vec3(bilinearColor.r, bilinearColor.g, bilinearColor.b) * bilinearColor.source_light_impact * (1.0 - bilinearColor.global_light_impact * source_light_level) * color * alpha;

    fragColor = clamp(vec4(result, alpha), 0.0, 1.0);
}