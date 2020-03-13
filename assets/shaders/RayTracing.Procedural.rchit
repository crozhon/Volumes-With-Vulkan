#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#extension GL_NV_ray_tracing : require
#include "Material.glsl"

layout(binding = 4) readonly buffer VertexArray { float Vertices[]; };
layout(binding = 5) readonly buffer IndexArray { uint Indices[]; };
layout(binding = 6) readonly buffer MaterialArray { Material[] Materials; };
layout(binding = 7) readonly buffer OffsetArray { uvec2[] Offsets; };
layout(binding = 8) uniform sampler2D[] TextureSamplers;
layout(binding = 9) readonly buffer SphereArray { vec4[] Spheres; };
layout(binding = 10) uniform sampler3D[] VolumeSamplers;

#include "Scatter.glsl"
#include "Vertex.glsl"

hitAttributeNV vec4 Sphere;
rayPayloadInNV RayPayload Ray;

vec2 GetSphereTexCoord(const vec3 point)
{
	const float phi = atan(point.x, point.z);
	const float theta = asin(point.y);
	const float pi = 3.1415926535897932384626433832795;

	return vec2
	(
		(phi + pi) / (2* pi),
		1 - (theta + pi /2) / pi
	);
}

bool intersectVolume(vec3 origin, vec3 direction) {
     float tmin = (-2.0 - origin.x) / direction.x;
     float tmax = (2.0 - origin.x) / direction.x;

     if (tmin > tmax) {
         float temp = tmin;
         tmin = tmax;
         tmax = temp;
     }
     float tymin = (-2.0 - origin.y) / direction.y;
     float tymax = (2.0 - origin.y) / direction.y;

     if (tymin > tymax) {
         float temp = tymin;
         tymin = tymax;
         tymax = temp;
     }

     if ((tmin > tymax) || (tymin > tmax)) {
         return false;
     }
     if (tymin > tmin) {
         tmin = tymin;
     }
     if (tymax < tmax) {
         tmax = tymax;
     }

     float tzmin = (-2.0 - origin.z) / direction.z;
     float tzmax = (2.0 - origin.z) / direction.z;

     if (tzmin > tzmax) {
         float temp = tzmin;
         tzmin = tzmax;
         tzmax = temp;
     }
     if (tmin > tzmax || tzmin > tmax) {
         return false;
     }
     if (tzmin > tmin) {
         tmin = tzmin;
     }
     if (tzmax < tmax) {
         tmax = tzmax;
     }
     return true;
}

bool withinVolume(vec3 point) {
    return point.x > -2.0 && point.x < 2.0 && point.y > -2.0 && point.y < 2.0 && point.z > -2.0 && point.z < 2.0;
}

void main()
{
	// Get the material.
	const uvec2 offsets = Offsets[gl_InstanceCustomIndexNV];
	const uint indexOffset = offsets.x;
	const uint vertexOffset = offsets.y;
	const Vertex v0 = UnpackVertex(vertexOffset + Indices[indexOffset]);
	const Material material = Materials[v0.MaterialIndex];

	// Compute the ray hit point properties.
	const vec4 sphere = Spheres[gl_InstanceCustomIndexNV];
	const vec3 center = sphere.xyz;
	const float radius = sphere.w;
	const vec3 point = gl_WorldRayOriginNV + gl_HitTNV * gl_WorldRayDirectionNV;
	const vec3 normal = (point - center) / radius;
	const vec2 texCoord = GetSphereTexCoord(normal);


	Ray = Scatter(material, gl_WorldRayDirectionNV, normal, texCoord, gl_HitTNV, Ray.RandomSeed);
    Ray.ScatterOrigin = point;

if (intersectVolume(gl_WorldRayOriginNV, gl_WorldRayDirectionNV)) {


    float s = 0.0;
    while (true) {
          s += - log(1 - RandomFloat(Ray.RandomSeed)) / 1.0f;
          if (s > Ray.ColorAndDistance.w) {
              break;
          }
          const vec3 sample_point = gl_WorldRayOriginNV + gl_HitTNV * (s / Ray.ColorAndDistance.w) * gl_WorldRayDirectionNV;

          if (withinVolume(sample_point) == false) {
              continue;
          }
          const vec3 volume_coord = (sample_point + 2.0f) / 4.0f;

          const float sigma_t = texture(VolumeSamplers[0], volume_coord).r;
          if (RandomFloat(Ray.RandomSeed) < sigma_t / 1.0f) {
             break;
          }
    }
    if (s < Ray.ColorAndDistance.w) {
       const vec3 sample_point = gl_WorldRayOriginNV + gl_HitTNV * (s / Ray.ColorAndDistance.w) * gl_WorldRayDirectionNV;
       const vec3 volume_coord = (sample_point + 2.0f) / 4.0f;
       if (volume_coord.r > 1.0) {
           Ray.ColorAndDistance.rgb = volume_coord;
           return;
       }
       const float sigma_t = texture(VolumeSamplers[0], volume_coord).r;
	   const float prob_extinction = exp(- s * sigma_t);

	   if (RandomFloat(Ray.RandomSeed) < (1 - prob_extinction)) {
	       //Ray.ScatterDirection.w = 0;
	       Ray.ColorAndDistance.rgb += vec3(1.0);
           Ray.ScatterOrigin = sample_point;
	   }
    }
    }
}
