#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// 让法线向量转换为世界空间坐标的一种简便方法，有助于简化 PBR 代码。
vec3 getNormalFromMap()
{
    // 1. 从法线贴图读取并解码法线（切线空间）
    //    纹理值在 [0,1]，乘 2 再减 1 变成 [-1,1]
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    // 2. 使用屏幕空间的偏导数近似计算世界空间下 WorldPos 对屏幕 x,y 的变化量
    //    dFdx/dFdy 是 GLSL 内建函数，分别给出当前片元在屏幕 x 方向和 y 方向上插值变量的偏导数。
    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    //    同理取得 UV 坐标对屏幕 x,y 的变化量
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    // 3. 标准化模型传入的世界空间法线
    vec3 N   = normalize(Normal);
    // 4. 根据微分关系构造切线 T
    //    推导自 ∂P/∂u and ∂P/∂v 与切线/副切线的线性关系
    vec3 T  = normalize( Q1 * st2.t - Q2 * st1.t );
    // 5. 构造副切线 B，-cross 保证和 T,N 构成右手系
    vec3 B  = -normalize( cross(N, T) );
    // 6. 将 T、B、N 放到一个矩阵里：TBN 矩阵
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
// 计算NDF发现分布函数
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// ----------------------------------------------------------------------------
// 计算几何遮蔽函数，Schlick——GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    // k_dierct 直接光照
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// ----------------------------------------------------------------------------
// 计算几何遮蔽函数，Smith，组合视线和光线两个方向的几何遮蔽
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
// 计算菲尼尔方程
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    // clamp函数限制大小在[0,1]之间
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------------------------------------------------------------------
// 计算菲尼尔方程，考虑粗糙度
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
    // material properties
    vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));// gamma correction
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;

    // input lighting data
    // 从法线贴图提取法线，并将其转换为世界空间坐标
    vec3 N = getNormalFromMap();            // normal
    vec3 V = normalize(camPos - WorldPos);  // view direction
    vec3 R = reflect(-V, N);                // reflection vector

    // 计算垂直入射时的反射率；
    // 如果是电介质（比如塑料），则使用 F0 值为 0.04；.3
    // 如果是金属，则使用颜色作为 F0 值（金属工作流程）   
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // 为了实现节能效果，漫射光和镜面反射光的亮度都不能超过 1.0（除非表面本身会发光）；
        // 为了保持这种关系，漫射光部分（kD）应等于 1.0 减去镜面反射光部分（kS）的值。
        vec3 kD = vec3(1.0) - kS;
        // 将 kD 乘以金属度的倒数，这样只有非金属材质才会具有漫反射光照效果，
        // 如果是部分金属材质则采用线性混合效果（纯金属材质则不会有漫反射光照效果）。
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;

    // 对pre-filter map和 BRDF LUT进行采样，
    // 并按照“分割求和近似法”将它们组合在一起，以获取 IBL 镜面反射部分
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color , 1.0);
}