#pragma once

#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <string>



class CodeGenManager {

public:
	static CodeGenManager &getInstance() {
		static CodeGenManager instance;
		return instance;
	}

	std::string GenerateFragShader() {
		return GenerateFragShaderTemplate() +
			GenerateBlockDefinitions() +
			GenerateScene() +
			GenerateRayMarchingTemplate();
	}


	std::string GenerateFragShaderTemplate() {
		return R"(
#version 330 core

// Ouput data
out vec4 color;
uniform vec2 resolution;
uniform float time;


vec2 pt;
		)";
	}

	std::string GenerateBlockDefinitions();

	std::string GenerateScene();

	std::string GenerateRayMarchingTemplate() {
		return R"(
vec3 norm(vec3 p)
{
	// the normal is simply the gradient of the volume
	vec4 dim = vec4(1, 1, 1, 0) * 0.0001;
	vec3 n;
	n.x = scene(p - dim.xww) - scene(p + dim.xww);
	n.y = scene(p - dim.wyw) - scene(p + dim.wyw);
	n.z = scene(p - dim.wwz) - scene(p + dim.wwz);
	return normalize(n);
}

void main(void)
{
	vec2 pos = gl_FragCoord.xy / resolution.xy;
	pt = -1.0 + 2.0 * vec2(pos.x, 1.0-pos.y);

	// camera
	vec3 dir = normalize(vec3(pt * resolution.xy, -0.5 * resolution.y / tan(0.5 * 45.0 / 180.0 * 3.1415926 ))); // looking from zPos
	vec2 rot = vec2(cos(time * 0.09), sin(time * 0.09)); // rotation starting from zPos
	vec3 ray = vec3(0.0, rot * 5.0);
	dir = vec3(dir.x, dot(vec2(dir.z, -dir.y), vec2(rot.x, -rot.y)), dot(vec2(dir.z, -dir.y), rot.yx) );

	// raymarching
	float t = 0.0;
	for (int i = 0; i < 90; i++)
	{
		float k = scene(ray + dir * t);
		t += k;
	}
	vec3 hit = ray + dir * t;

	// fog
	float fogFact = clamp(exp(-distance(ray, hit) * 0.3), 0.0, 1.0);

	if (fogFact < 0.05)
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	// diffuse & specular light
	vec3 sun = normalize(vec3(0.1, 1.0, 0.2));
	vec3 n = norm(hit);
	vec3 ref = reflect(normalize(hit - ray), n);
	float diff = dot(n, sun);
	float spec = pow(max(dot(ref, sun), 0.0), 32.0);
	vec3 col = mix(vec3(0.0, 0.7, 0.9), vec3(0.0, 0.1, 0.2), diff);

	// enviroment map
//	col += textureCube(iChannel0, ref).xyz * 0.2;
	col = fogFact * (col + spec);

	// iq's vignetting
//	col *= 0.1 + 0.8 * pow(16.0 * pos.x * pos.y * (1.0 - pos.x) * (1.0 - pos.y), 0.1);

	color = vec4(col, 1.0);

}
		)";
	}

private:
	CodeGenManager() { }
};


#endif
