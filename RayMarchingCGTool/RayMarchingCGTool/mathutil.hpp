#pragma once

#ifndef MATHUTIL_HPP
#define MATHUTIL_HPP

#include <algorithm>

struct Vec2{
	float x, y;
	Vec2() { x = y = 0; }
	Vec2(float px, float py) { x = px; y = py; }
};

struct Rec{
	Vec2 pos, size;
	Rec() : pos(), size() {}
	Rec(float px, float py, float pw, float ph) : pos(px, py), size(pw, ph) {}
};

static bool overlaps(Rec a, Rec b) {
	using namespace std;
	return !(max(a.pos.x, a.pos.x + a.size.x) < min(b.pos.x, b.pos.x + b.size.x)) &&
		!(max(b.pos.x, b.pos.x + b.size.x) < min(a.pos.x, a.pos.x + a.size.x)) &&
		!(max(a.pos.y, a.pos.y + a.size.y) < min(b.pos.y, b.pos.y + b.size.y)) &&
		!(max(b.pos.y, b.pos.y + b.size.y) < min(a.pos.y, a.pos.y + a.size.y));
}

static bool overlaps(Vec2 a, Rec b) {
	return overlaps(Rec(a.x, a.y, 0, 0), b);
}

static bool overlaps(Rec a, Vec2 b) {
	return overlaps(Rec(b.x, b.y, 0, 0), a);
}

#endif