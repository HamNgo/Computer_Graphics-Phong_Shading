#include "lighting.h"

ColorRGB calculatePhongLighting(Vec3 position, Vec3 normal, Vec3 cameraPosition,
    const Light& light, const Material& material) {

    Vec3 N = normalize(normal);
    Vec3 L = normalize(light.position - position);
    Vec3 V = normalize(cameraPosition - position);

    float diffuseAmount = std::fmax(0.0f, dot(N, L));

    // The highlight only exists on surfaces the light actually reaches (N.L > 0).
    float specularAmount = 0.0f;
    if (diffuseAmount > 0.0f) {
        Vec3 R = N * (2.0f * dot(N, L)) - L;   // L mirrored about the normal
        specularAmount = std::pow(std::fmax(0.0f, dot(R, V)), material.shininess);
    }

    float ambientTerm = material.ambient;
    float diffuseTerm = material.diffuse * diffuseAmount;
    float specularTerm = material.specular * specularAmount;

    ColorRGB result = {
        light.color.x * (material.baseColor.r * (ambientTerm + diffuseTerm) + material.specularColor.r * specularTerm),
        light.color.y * (material.baseColor.g * (ambientTerm + diffuseTerm) + material.specularColor.g * specularTerm),
        light.color.z * (material.baseColor.b * (ambientTerm + diffuseTerm) + material.specularColor.b * specularTerm)
    };
    result.r = clampf(result.r, 0.0f, 1.0f);
    result.g = clampf(result.g, 0.0f, 1.0f);
    result.b = clampf(result.b, 0.0f, 1.0f);
    return result;
}