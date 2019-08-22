#include "block.hpp"

#include "renderingtarget.hpp"
#include <cassert>

Block::Block(int numIn, int numOut) :
	numInput(numIn), srcBlocks(numIn, NULL),
	numOutput(numOut), dstBlocks(numOut, NULL),
	renderRec(0, 0, BlockDefaultSize + 2 * BlockDefaultPortLength, BlockDefaultSize)
	{  }

void Block::DrawObject(){
	// draw block rect
	Rec blockRect(renderRec.pos.x + BlockDefaultPortLength, renderRec.pos.y, renderRec.size.x - 2 * BlockDefaultPortLength, renderRec.size.y);
	rtUtil::setColor(rtConstants::normalFillColor);
	rtBox::getInstance().Draw(blockRect, true);
	rtUtil::setColor(rtConstants::normalLineColor);
	rtBox::getInstance().Draw(blockRect, false);

	// draw block port
	rtUtil::setColor(rtConstants::normalLineColor);
	for (int i = 0; i < numInput; i++) {
		
		Vec2 portPos = GetInputPortPos(i);
		rtLine::getInstance().Draw({ portPos.x, portPos.y }, { portPos.x + 2 * BlockDefaultPortLength, portPos.y });
	}
	for (int i = 0; i < numOutput; i++) {
		Vec2 portPos = GetOutputPortPos(i);
		rtLine::getInstance().Draw({ portPos.x - 2 * BlockDefaultPortLength, portPos.y }, { portPos.x, portPos.y });
	}

	// draw block icon
	DrawIcon();
}

int Block::IsPicked(Vec2 cursorPos) {
	return overlaps(cursorPos, renderRec) ? 1 : 0;
}


void Block::setPosition(Rec newPos) {
	// update block rendering area
	renderRec = newPos;
	// update block port connections position
	for (int i = 0; i < srcBlocks.size(); i++) if (srcBlocks[i])
		srcBlocks[i]->toPos = GetInputPortPos(i);
	for (int i = 0; i < dstBlocks.size(); i++) if (dstBlocks[i])
		dstBlocks[i]->fromPos = GetOutputPortPos(i);

}

Vec2 Block::GetInputPortPos(int portIdx){
	return { renderRec.pos.x, 
		renderRec.pos.y + (renderRec.size.y * (portIdx + 1.0f) / (numInput + 1.0f)) };
}

Vec2 Block::GetOutputPortPos(int portIdx){
	return { renderRec.pos.x + renderRec.size.x,
		renderRec.pos.y + (renderRec.size.y * (portIdx + 1.0f) / (numOutput + 1.0f)) };
}

Rec Block::GetInputPortRenderRec(int portIdx){
	Vec2 pos = GetInputPortPos(portIdx);
	return{ pos.x, pos.y - BlockDefaultPortActableRadius, BlockDefaultPortLength * 2, BlockDefaultPortActableRadius * 2 };
}

Rec Block::GetOutputPortRenderRec(int portIdx){
	Vec2 pos = GetOutputPortPos(portIdx);
	return{ pos.x - BlockDefaultPortLength * 2, pos.y - BlockDefaultPortActableRadius, BlockDefaultPortLength * 2, BlockDefaultPortActableRadius * 2 };
}





BlockGraph::BlockGraph() {
	blockList.push_back(new BoxBlock());
	blockList.push_back(new SphereBlock()); blockList.back()->renderRec = Rec(rand() % 500, rand() % 500, Block::BlockDefaultSize + Block::BlockDefaultPortLength * 2, Block::BlockDefaultSize);
	blockList.push_back(new BoolDifferenceBlock()); blockList.back()->renderRec = Rec(rand() % 500, rand() % 500, Block::BlockDefaultSize + Block::BlockDefaultPortLength * 2, Block::BlockDefaultSize);
	blockList.push_back(new ScreenBlock()); blockList.back()->renderRec = Rec(rand() % 500, rand() % 500, Block::BlockDefaultSize + Block::BlockDefaultPortLength * 2, Block::BlockDefaultSize);


	connectionList.push_back(new Connection(blockList[0], 0, blockList[2], 0));
	connectionList.push_back(new Connection(blockList[1], 0, blockList[2], 1));
	connectionList.push_back(new Connection(blockList[2], 0, blockList[3], 0));


	
	//for (int i = 0; i < 300; i++)
	//{
	//	blockList.push_back(new BoolDifferenceBlock());
	//	blockList.back()->renderRec = Rec(rand() % 5000, rand() % 5000, Block::BlockDefaultSize + Block::BlockDefaultPortLength * 2, Block::BlockDefaultSize);
	//}

	//for (int i = 0; i < 400; i++)
	//{
	//	int from = rand() % blockList.size(); if (blockList[from]->numOutput == 0) continue;  int fromIdx = rand() % blockList[from]->numOutput;
	//	int to = rand() % blockList.size(); if (blockList[to]->numInput == 0) continue;  int toIdx = rand() % blockList[to]->numInput;
	//	if (blockList[from]->dstBlocks[fromIdx]==NULL && blockList[to]->srcBlocks[toIdx]==NULL)
	//		BlockGraph::AddConnection(blockList[from], fromIdx, blockList[to], toIdx);
	//}


	setupRenderingInfoCache();
}

Connection* BlockGraph::AddConnection(Block *bFrom, int iFrom, Block *bTo, int iTo) {
	Connection *conn = new Connection(bFrom, iFrom, bTo, iTo);
	BlockGraph::getInstance().connectionList.push_back(conn);
	BlockGraph::getInstance().blockOrderList.push_back(conn);
	return conn;
}
void BlockGraph::RemoveConnection(Connection *conn) {
	BlockGraph::getInstance().blockOrderList.remove(conn);
	BlockGraph::getInstance().connectionList.erase(std::find(BlockGraph::getInstance().connectionList.begin(), BlockGraph::getInstance().connectionList.end(), conn));
	delete conn; //析构完成才真正切断联系
}




void BlockGraph::setupRenderingInfoCache() {
	blockOrderList.clear();
	for (auto it = blockList.begin(); it != blockList.end(); ++it) {
		blockOrderList.push_back(*it);
	}
	for (auto it = connectionList.begin(); it != connectionList.end(); ++it) {
		blockOrderList.push_back(*it);
	}
}


Connection::Connection(Block *bFrom, int iFrom, Block *bTo, int iTo) :
	from(0), fromIdx(0), to(0), toIdx(0) {
	// Connection is valid only if both From and To exist
	assert(bFrom || bTo);
	SetFrom(bFrom, iFrom);
	SetTo(bTo, iTo);
}

Connection::~Connection() {
	// delete the connection only
	assert(from || to);
	SetFrom(NULL, 0);
	SetTo(NULL, 0);
}

void Connection::DrawObject() {
	// draw arrow from parent to this
	rtUtil::setColor(rtConstants::normalLineColor);
	rtArrow::getInstance().Draw(fromPos, toPos);
}

int Connection::IsPicked(Vec2 cursorPos) {
	// check output/input port
	bool hit1 = overlaps(cursorPos, { fromPos.x - Connection::PickingRadius, fromPos.y - Connection::PickingRadius, Connection::PickingRadius * 2, Connection::PickingRadius * 2 });
	bool hit2 = overlaps(cursorPos, { toPos.x - Connection::PickingRadius, toPos.y - Connection::PickingRadius, Connection::PickingRadius * 2, Connection::PickingRadius * 2 });
	if (hit1)
		return 1; // picked output
	else if (hit2)
		return 2; // picked input
	else 
		return 0; // picked nothing
}

void Connection::SetFrom(Block *b, int idx) {
	// disconnect previous From Block
	if (from) from->dstBlocks[fromIdx] = NULL;

	from = b;
	fromIdx = idx;
	if (from) {
		fromPos = from->GetOutputPortPos(idx);
		from->dstBlocks[fromIdx] = this;
	}
}

void Connection::SetTo(Block *b, int idx) {
	// disconnect previous To Block
	if (to) to->srcBlocks[toIdx] = NULL;

	to = b;
	toIdx = idx;
	if (to) {
		toPos = to->GetInputPortPos(idx);
		to->srcBlocks[toIdx] = this;
	}
}







std::string SphereBlock::GenerateDefinition() {
	return
		R"(
float sdsphere(vec3 p, float r) {
	return length(p) - r;
}
		)";
}
std::string SphereBlock::GenerateCallsite() {
	return R"(sdsphere(p, 1.0))";
}

void SphereBlock::DrawIcon() {
	// draw a character in the center of the block
	rtText::getInstance().Draw(Vec2(renderRec.pos.x + renderRec.size.x * 0.5, renderRec.pos.y + renderRec.size.y * 0.5), 5, L"球", BlockDefaultSize * 0.6);
}


std::string BoxBlock::GenerateDefinition() {
	return
		R"(
float sdBox(vec3 p, vec3 b)
{
	vec3 d = abs(p) - b;
	return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}
		)";
}
std::string BoxBlock::GenerateCallsite() {
	return R"(sdBox(p, vec3(0.7)))";
}

void BoxBlock::DrawIcon() {
	// draw a character in the center of the block
	rtText::getInstance().Draw(Vec2(renderRec.pos.x + renderRec.size.x * 0.5, renderRec.pos.y + renderRec.size.y * 0.5), 5, L"盒", BlockDefaultSize * 0.6);
}



std::string ScreenBlock::GenerateDefinition() {
	return "";
}
std::string ScreenBlock::GenerateCallsite() {
	return srcBlocks[0]->from->GenerateCallsite();
}

void ScreenBlock::DrawIcon() {
	// draw a character in the center of the block
	rtText::getInstance().Draw(Vec2(renderRec.pos.x + renderRec.size.x * 0.5, renderRec.pos.y + renderRec.size.y * 0.5), 5, L"显", BlockDefaultSize * 0.6);
}



std::string BoolDifferenceBlock::GenerateDefinition() {
	return
		R"(
float opS(float d1, float d2){
	return max(-d1,d2);
}
		)";
}
std::string BoolDifferenceBlock::GenerateCallsite() {
	return "opS(" + srcBlocks[0]->from->GenerateCallsite() + "," + srcBlocks[1]->from->GenerateCallsite() + ")";
}

void BoolDifferenceBlock::DrawIcon() {
	// draw a character in the center of the block
	rtText::getInstance().Draw(Vec2(renderRec.pos.x + renderRec.size.x * 0.5, renderRec.pos.y + renderRec.size.y * 0.5), 5, L"差", BlockDefaultSize * 0.6);
}
