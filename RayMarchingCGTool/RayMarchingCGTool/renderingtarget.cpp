#include "renderingtarget.hpp"

// Include GLFW
#include <GLFW/glfw3.h>

#include "windowinfo.hpp"
#include <cassert>
#include <cmath>

const double PI = 3.141592653589793238463;

const float rtConstants::backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const float rtConstants::normalLineColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float rtConstants::normalFillColor[4] = { 0.2f, 0.4f, 0.8f, 0.5f };
const std::string rtConstants::IconFontFile = std::string("msyhbd.ttf");

void rtUtil::setColor(const float color[4]) {
	glUniform4fv(DiagramWindowInfo::getInstance().drawcolorID, 1, color);
}


rtBox &rtBox::getInstance() {
	// singleton: only initiated in the diagramWindow context
	assert(glfwGetCurrentContext() == DiagramWindowInfo::getInstance().window);
	static rtBox instance;
	return instance;
}

void rtBox::Draw(Rec position, bool isFill, float zVal) {
	if (isFill) {
		const GLfloat box_data[] = {
			// FILL
			position.pos.x, position.pos.y, zVal,
			position.pos.x + position.size.x, position.pos.y, zVal,
			position.pos.x, position.pos.y + position.size.y, zVal,
			position.pos.x + position.size.x, position.pos.y + position.size.y, zVal,
		};

		// VBO
		glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(box_data), box_data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// draw box
		glBindVertexArray(boxVertexArrayObject);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw quad (2 triangles)
		glBindVertexArray(0);
	}
	else {
		const GLfloat box_data[] = {
			// LINE_LOOP
			position.pos.x, position.pos.y, zVal,
			position.pos.x + position.size.x, position.pos.y, zVal,
			position.pos.x + position.size.x, position.pos.y + position.size.y, zVal,
			position.pos.x, position.pos.y + position.size.y, zVal,
		};

		// VBO
		glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(box_data), box_data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// draw box
		glBindVertexArray(boxVertexArrayObject);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_LINE_LOOP, 0, 4); // draw line loop (box)
		glBindVertexArray(0);
	}

}

rtBox::rtBox() {
	// VBO
	glGenBuffers(1, &boxVertexBuffer);

	// VAO
	glGenVertexArrays(1, &boxVertexArrayObject);
	glBindVertexArray(boxVertexArrayObject);
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

rtBox::~rtBox() {
	glDeleteBuffers(1, &boxVertexBuffer);
	glDeleteVertexArrays(1, &boxVertexArrayObject);
}



rtLine &rtLine::getInstance() {
	// singleton: only initiated in the diagramWindow context
	assert(glfwGetCurrentContext() == DiagramWindowInfo::getInstance().window);
	static rtLine instance;
	return instance;
}

void rtLine::Draw(Vec2 pos1, Vec2 pos2, float zVal) {
	const GLfloat line_data[] = {
		// LINE
		pos1.x, pos1.y, zVal,
		pos2.x, pos2.y, zVal,
	};

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_data), line_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// draw line
	glBindVertexArray(lineVertexArrayObject);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_LINES, 0, 2); // draw line
	glBindVertexArray(0);
}

rtLine::rtLine() {
	// VBO
	glGenBuffers(1, &lineVertexBuffer);

	// VAO
	glGenVertexArrays(1, &lineVertexArrayObject);
	glBindVertexArray(lineVertexArrayObject);
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

rtLine::~rtLine() {
	glDeleteBuffers(1, &lineVertexBuffer);
	glDeleteVertexArrays(1, &lineVertexArrayObject);
}



rtArrow &rtArrow::getInstance() {
	// singleton: only initiated in the diagramWindow context
	assert(glfwGetCurrentContext() == DiagramWindowInfo::getInstance().window);
	static rtArrow instance;
	return instance;
}

void rtArrow::Draw(Vec2 fromPos, Vec2 toPos, float zVal) {
	// Prepare a line segment & a triangle
	float r = std::sqrt((toPos.x - fromPos.x)*(toPos.x - fromPos.x) + (toPos.y - fromPos.y)*(toPos.y - fromPos.y));
	Vec2 p = Vec2(r - rtConstants::ArrowSize, rtConstants::ArrowSize * std::tan(rtConstants::ArrowAngle /180.0 * PI)); // 旋转前底角
	Vec2 cs = Vec2((toPos.x - fromPos.x) / r, (toPos.y - fromPos.y) / r); // cos(phi), sin(phi)
	GLfloat arrow_data[] = {
		// LINE
		fromPos.x, fromPos.y, zVal,
		toPos.x, toPos.y, zVal,

		// TRIANGLE
		fromPos.x + cs.x * p.x - cs.y * p.y, fromPos.y + cs.y * p.x + cs.x * p.y, zVal,
		fromPos.x + cs.x * p.x + cs.y * p.y, fromPos.y + cs.y * p.x - cs.x * p.y, zVal,
		toPos.x, toPos.y, zVal,
	};

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, arrowVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(arrow_data), arrow_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// draw arrow
	glBindVertexArray(arrowVertexArrayObject);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_LINES, 0, 2); // draw line segment
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 2, 3); // draw triangle
	glBindVertexArray(0);

}

rtArrow::rtArrow() {
	// VBO
	glGenBuffers(1, &arrowVertexBuffer);

	// VAO
	glGenVertexArrays(1, &arrowVertexArrayObject);
	glBindVertexArray(arrowVertexArrayObject);
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, arrowVertexBuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

rtArrow::~rtArrow() {
	glDeleteBuffers(1, &arrowVertexBuffer);
	glDeleteVertexArrays(1, &arrowVertexArrayObject);
}



rtText &rtText::getInstance() {
	// singleton: only initiated in the diagramWindow context
	assert(glfwGetCurrentContext() == DiagramWindowInfo::getInstance().window);
	static rtText instance;
	return instance;
}

void rtText::Draw(Vec2 pos, int align, std::wstring str, int size, float zVal) {

	unsigned char *tex_buffer_memory = NULL;
	Rec tex_bounding_box;
	auto it = glyphCache.find(std::make_pair(str, size));
	if (it != glyphCache.end()) {
		// use cached data!
		tex_buffer_memory = it->second.first.get();
		tex_bounding_box = it->second.second;
	} else {
		// cache failed, create new glyph
		SetFontSize(size);
		ConvertStringToTex(str, tex_buffer_memory, tex_bounding_box);
		// add to cache
		glyphCache[std::make_pair(str, size)] = std::make_pair(std::unique_ptr<unsigned char>(tex_buffer_memory), tex_bounding_box);
	}

	// Alginment: 1 means pos = left-bottom corner; 5 means pos = center
	//    789
	//    456
	//    123
	Vec2 leftbottom;
	if (align % 3 == 1) leftbottom.x = pos.x;
	else if (align % 3 == 2) leftbottom.x = pos.x - tex_bounding_box.size.x / 2;
	else if (align % 3 == 0) leftbottom.x = pos.x - tex_bounding_box.size.x;
	if ((align - 1) / 3 == 0) leftbottom.y = pos.y;
	else if ((align - 1) / 3 == 1) leftbottom.y = pos.y - tex_bounding_box.size.y / 2;
	else if ((align - 1) / 3 == 2) leftbottom.y = pos.y - tex_bounding_box.size.y;

	const GLfloat text_data[] = {
		// FILL
		leftbottom.x, leftbottom.y, zVal,
		leftbottom.x + tex_bounding_box.size.x, leftbottom.y, zVal,
		leftbottom.x, leftbottom.y + tex_bounding_box.size.y, zVal,
		leftbottom.x + tex_bounding_box.size.x, leftbottom.y + tex_bounding_box.size.y, zVal,
	};

	//debugging bounding box
	//rtBox::getInstance().Draw(Rec(leftbottom.x, leftbottom.y, tex_bounding_box.size.x, tex_bounding_box.size.y), false);

	const GLfloat text_texcoord_data[] = {
		0, 0,
		1, 0,
		0, 1,
		1, 1,
	};

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, textVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(text_data), text_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, textTexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(text_texcoord_data), text_texcoord_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// change texture
	glActiveTexture(GL_TEXTURE0); // select 0
	glBindTexture(GL_TEXTURE_2D, fTextureText);
	{
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			(int)tex_bounding_box.size.x,
			(int)tex_bounding_box.size.y,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			tex_buffer_memory
			);

		// draw text
		glBindVertexArray(textVertexArrayObject);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUniform1i(DiagramWindowInfo::getInstance().shadertypeID, 1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw quad (2 triangles)
		glUniform1i(DiagramWindowInfo::getInstance().shadertypeID, 0);
		glBindVertexArray(0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

rtText::rtText() {


	// VBO
	glGenBuffers(1, &textVertexBuffer);
	glGenBuffers(1, &textTexCoordBuffer);

	// VAO
	glGenVertexArrays(1, &textVertexArrayObject);
	glBindVertexArray(textVertexArrayObject);
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, textVertexBuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(1); // UV
		glBindBuffer(GL_ARRAY_BUFFER, textTexCoordBuffer);
		glVertexAttribPointer(
			1,                   // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                   // size : U+V => 2
			GL_FLOAT,            // type
			GL_FALSE,            // normalized?
			0,                   // stride
			(void*)0             // array buffer offset
			);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

	// Load TTF File
	fFileBuffer = LoadFontFile(rtConstants::IconFontFile.c_str(), fFontFileSize);
	InitFontLibrary();

	// Default font size
	SetFontSize(rtConstants::TextFontSize);

	// create texture
	glActiveTexture(GL_TEXTURE0); // select 0
	glGenTextures(1, &fTextureText);
	glBindTexture(GL_TEXTURE_2D, fTextureText);
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

rtText::~rtText() {
	glDeleteBuffers(1, &textVertexBuffer);
	glDeleteBuffers(1, &textTexCoordBuffer);
	glDeleteVertexArrays(1, &textVertexArrayObject);

	// Unload TTF File
	DeleteFontLibrary();
	delete fFileBuffer;

	// delete texture
	glDeleteTextures(1, &fTextureText);
}

void rtText::InitFontLibrary() {
	int error = FT_Init_FreeType(&fFontLibrary);
	if (error) {}

	error = FT_New_Memory_Face(fFontLibrary,
		(const FT_Byte*)fFileBuffer,    // file base
		fFontFileSize,            // file size
		0,                     // default face
		&fFontFace);

	if (error == FT_Err_Unknown_File_Format) {
		//... the font file could be opened and read, but it appears
		//	... that its font format is unsupported
	}
	else if (error)	{
		//... another error code means that the font file could not
		//	... be opened or read, or simply that it is broken...
	}

	// reset glyph cache
	glyphCache.clear();
}

void rtText::DeleteFontLibrary() {
	FT_Done_Face(fFontFace);
	FT_Done_FreeType(fFontLibrary);

	// reset glyph cache
	glyphCache.clear();
}

unsigned char *rtText::LoadFontFile(const char * fileName, unsigned int &fileSize)
{
	FILE *inFile = NULL;
	fopen_s(&inFile, fileName, "rb");
	fseek(inFile, 0, SEEK_END); // seek to end of file
	fileSize = ftell(inFile); // get current file pointer
	fseek(inFile, 0, SEEK_SET); // seek back to beginning of file

	unsigned char *buf = new unsigned char[fileSize];

	fileSize = (unsigned int)fread(buf, sizeof(unsigned char), fileSize, inFile);
	fclose(inFile);
	return buf;
}

void rtText::SetFontSize(int size) {
	bool error = false;
	error = FT_Set_Pixel_Sizes(
		fFontFace,   /* handle to face object */
		size,      /* pixel_width           */
		size);     /* pixel_height          */
}

void rtText::ConvertStringToTex(std::wstring str, unsigned char *&texMemory, Rec &texBoundingBox) {

	FT_GlyphSlot  slot = fFontFace->glyph;  /* a small shortcut */
	FT_UInt       glyph_index;
	FT_Bool       use_kerning = FT_HAS_KERNING(fFontFace);
	FT_UInt       previous;
	int           pen_x, pen_y;
	FT_Vector     delta;
	int           baseY;
	bool          error = false;

	// todo: 加入转义解析

	// 第一趟：确定包围盒大小
	pen_x = pen_y = 0; //任意基点
	previous = 0;
	int xMin, xMax, yMin, yMax;
	xMin = xMax = yMin = yMax = 0;
	for (int n = 0; n < str.length(); n++) {
		/* convert character code to glyph index */
		glyph_index = FT_Get_Char_Index(fFontFace, str[n]);
		/* retrieve kerning distance and move pen position */
		if (use_kerning && previous && glyph_index) {
			FT_Get_Kerning(fFontFace, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
			pen_x += delta.x >> 6;
		}
		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Glyph(fFontFace, glyph_index, FT_LOAD_RENDER);
		if (error) continue;  /* ignore errors */

		// a single glyph bouncing box is given by the following:
		// xMin = pen_x + slot->bitmap_left, yMax = pen_y + slot->bitmap_top;
		// xMax = xMin + slot->bitmap.width, yMin = yMax - slot->bitmap.rows;

		// grow overall string bouncing box
		xMin = std::min(pen_x + slot->bitmap_left, xMin); xMax = std::max(pen_x + slot->bitmap_left + slot->bitmap.width, xMax);
		yMin = std::min(pen_y + slot->bitmap_top - slot->bitmap.rows, yMin); yMax = std::max(pen_y + slot->bitmap_top, yMax);

		/* increment pen position */
		pen_x += slot->advance.x >> 6;
		/* record current glyph index */
		previous = glyph_index;
	}

	// allocate memory for texture (顺便把greyscale转成rgba，更flexible; alternative的话: subteximage2d)
	unsigned char *fStringBMP = new unsigned char[(xMax - xMin) * (yMax - yMin)]; // r, aligned with 4, upperbound exclusive
	memset(fStringBMP, 0, sizeof (unsigned char)* (xMax - xMin) * (yMax - yMin));
	// 第二趟：搬运纹理
	pen_x = pen_y = 0; //任意基点
	previous = 0;
	for (int n = 0; n < str.length(); n++) {
		/* convert character code to glyph index */
		glyph_index = FT_Get_Char_Index(fFontFace, str[n]);
		/* retrieve kerning distance and move pen position */
		if (use_kerning && previous && glyph_index){
			FT_Get_Kerning(fFontFace, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
			pen_x += delta.x >> 6;
		}
		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Glyph(fFontFace, glyph_index, FT_LOAD_RENDER);
		if (error) continue;  /* ignore errors */

		// copy bitmap
		for (int i = 0; i < slot->bitmap.rows; i++) {
			baseY = pen_y + slot->bitmap_top - 1 - yMin - i; // freeType bitmap为图像格式 需要颠倒
			memcpy(fStringBMP + baseY * (xMax - xMin) + pen_x + slot->bitmap_left - xMin, slot->bitmap.buffer + i * slot->bitmap.width, sizeof(unsigned char)* slot->bitmap.width);
		}

		/* increment pen position */
		pen_x += slot->advance.x >> 6;
		/* record current glyph index */
		previous = glyph_index;
	}

	// return
	texMemory = fStringBMP;
	texBoundingBox = Rec(xMin, yMin, (float)xMax - xMin, (float)yMax - yMin);

	//// immediately free memory
	//if (fStringBMP) delete[] fStringBMP;
}
