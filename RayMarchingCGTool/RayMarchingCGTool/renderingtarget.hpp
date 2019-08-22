#pragma once

#ifndef RENDERINGTARGET_HPP
#define RENDERINGTARGET_HPP

// Include GLEW
#include <GL/glew.h>

// FreeType 2
#include <ft2build.h>
#include FT_FREETYPE_H

#include "mathutil.hpp"
#include <string>
#include <unordered_map>
#include <memory>

using namespace std;
namespace std {
	template < typename T, typename U >
	struct hash<pair<T, U> > { 
		size_t operator()(const pair<T, U> key)  {
			return hash<T>()(key.first) ^ hash<U>()(key.second); 
		}
	};
};


class rtConstants
{
public:
	static const int ArrowSize = 20; // the Height of the triangle
	static const int ArrowAngle = 15; // half of the triangle apex angle
	static const float normalLineColor[4]; // block border
	static const float normalFillColor[4]; // block fill
	static const float backgroundColor[4]; // diagram window background
	static const int TextFontSize = 40;
	static const std::string IconFontFile;
};


class rtUtil
{
public:
	static void setColor(const float color[4]);
};

class rtBox
{
public:
	static rtBox &getInstance();
	void Draw(Rec pos, bool isFill, float zVal = 0.0f);

private:
	rtBox(); // setup VAO
	~rtBox(); // destroy VAO

	GLuint boxVertexBuffer;
	GLuint boxVertexArrayObject;
};

class rtLine
{
public:
	static rtLine &getInstance();
	void Draw(Vec2 pos1, Vec2 pos2, float zVal = 0.0f);

private:
	rtLine(); // setup VAO
	~rtLine(); // destroy VAO

	GLuint lineVertexBuffer;
	GLuint lineVertexArrayObject;
};

class rtArrow
{
public:
	static rtArrow &getInstance();
	void Draw(Vec2 fromPos, Vec2 toPos, float zVal = 0.0f);

private:
	rtArrow(); // setup VAO
	~rtArrow(); // destroy VAO

	GLuint arrowVertexBuffer;
	GLuint arrowVertexArrayObject;
};

class rtText
{
public:
	static rtText &getInstance();
	void Draw(Vec2 pos, int align, std::wstring str, int size = rtConstants::TextFontSize, float zVal = 0.0f);

private:
	rtText(); // setup VAO / ft library
	~rtText(); // destroy VAO / ft library
	void InitFontLibrary();
	void DeleteFontLibrary();
	unsigned char *LoadFontFile(const char * fileName, unsigned int &fileSize);

	// utils
	void SetFontSize(int size);
	void ConvertStringToTex(std::wstring str, unsigned char *&texMemory, Rec &texBoundingBox);

	GLuint textVertexBuffer;
	GLuint textTexCoordBuffer;
	GLuint textVertexArrayObject;

	// font handle
	FT_Library fFontLibrary;
	FT_Face fFontFace;
	unsigned char *fFileBuffer; // TTF
	unsigned int fFontFileSize;

	// texture handle
	GLuint fTextureText;

	// 用String和FontSize做索引，todo:和字体绑定
    struct pairhash {
    public:
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T, U> &x) const
        {
            return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
        }
    };

	std::unordered_map<std::pair<std::wstring, int>, std::pair<std::unique_ptr<unsigned char>, Rec>, pairhash > glyphCache;
};


#endif
