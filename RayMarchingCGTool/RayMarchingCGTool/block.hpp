#pragma once

#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "mathutil.hpp"

#include <vector>
#include <list>
#include <string>

class Renderable {
public:
	virtual void DrawObject() = 0;
	virtual int IsPicked(Vec2 cursorPos) { return 0; } // using return value to pass arg

};

class Connection;

class Block : public Renderable{

public:
	static const int BlockDefaultPortLength = 10;
	static const int BlockDefaultPortActableRadius = 10;
	static const int BlockDefaultSize = 100;

	
	virtual void DrawObject();	
	virtual int IsPicked(Vec2 cursorPos);

	virtual std::string GenerateDefinition() = 0;
	virtual std::string GenerateCallsite() = 0;

	void setPosition(Rec newPos);
	Vec2 GetInputPortPos(int portIdx);
	Vec2 GetOutputPortPos(int portIdx);
	Rec GetInputPortRenderRec(int portIdx);
	Rec GetOutputPortRenderRec(int portIdx);

//protected:
	
	Block(int numIn, int numOut);

	int numInput;
	std::vector<Connection *> srcBlocks;
	int numOutput;
	std::vector<Connection *> dstBlocks;
	Rec renderRec; // = bounding box = actionable area

protected:
	virtual void DrawIcon(/*args*/) = 0;

};


// to consider:  Template?
class SphereBlock : public Block {
public:
	virtual void DrawIcon();
	virtual std::string GenerateDefinition();
	virtual std::string GenerateCallsite();
	SphereBlock() : Block(0, 1) {}
};

class BoxBlock : public Block {
public:
	virtual void DrawIcon();
	virtual std::string GenerateDefinition();
	virtual std::string GenerateCallsite();
	BoxBlock() : Block(0, 1) {}
};

class ScreenBlock : public Block {
public:
	virtual void DrawIcon();
	virtual std::string GenerateDefinition();
	virtual std::string GenerateCallsite();
	ScreenBlock() : Block(1, 0) {}
};

class BoolDifferenceBlock : public Block {
public:

	virtual void DrawIcon();
	virtual std::string GenerateDefinition();
	virtual std::string GenerateCallsite();
	BoolDifferenceBlock() : Block(2, 1) {}
};


class Connection : public Renderable {
public:
	static const int PickingRadius = 10;

	virtual void DrawObject();
	virtual int IsPicked(Vec2 cursorPos);

	void SetFrom(Block *b, int idx);
	void SetTo(Block *b, int idx);

	Block *from, *to;
	int fromIdx, toIdx;

	Vec2 fromPos, toPos; // for rendering

	Connection(Block *bFrom, int iFrom, Block *bTo, int iTo);
	~Connection();
};



class BlockGraph {
public:
	static BlockGraph &getInstance() {
		static BlockGraph instance;
		return instance;
	}

	// memory holder
	std::vector<Block*> blockList;
	std::vector<Connection*> connectionList;

	// graphics data
	std::list<Renderable*> blockOrderList; // front() = backmost object

	// Euler operations
	Connection* AddConnection(Block *bFrom, int iFrom, Block *bTo, int iTo);
	void RemoveConnection(Connection *conn);

private:
	BlockGraph();

	// create runtime rendering info for block diagram
	void setupRenderingInfoCache();
};

#endif
