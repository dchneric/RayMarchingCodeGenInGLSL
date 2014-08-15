#include "windowinfo.hpp"

#include "block.hpp"
#include "appstate.hpp"
#include "codegen.hpp"

DiagramWindowUserInputManager::UserInput DiagramWindowUserInputManager::currentUserInputState = DiagramWindowUserInputManager::CANCEL;
Vec2 DiagramWindowUserInputManager::pivotPos;
Block *DiagramWindowUserInputManager::pivotBlock;
Connection *DiagramWindowUserInputManager::pivotConnection;
bool DiagramWindowUserInputManager::pivotConnectionIsInputPort;

void DiagramWindowUserInputManager::key_callback(GLFWwindow* DisplayWindow, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(DisplayWindow, GL_TRUE);
	else if (key == GLFW_KEY_D && mods == GLFW_MOD_CONTROL && action == GLFW_PRESS) {
		processInput(COMPILE, DisplayWindow);
		updateInput(COMPILE, DisplayWindow);
		processInput(CANCEL, DisplayWindow);
	}
}

// 回调会在主线程中进行，此时不应做gl相关操作
void DiagramWindowUserInputManager::resize_callback(GLFWwindow *DisplayWindow, int width, int height)
{
	DiagramWindowInfo::getInstance().Height = height; DiagramWindowInfo::getInstance().Width = width;

	// pivot: top-left corner (no change)
	DiagramWindowInfo::getInstance().needUpdateMVP = true;
}

// State Machine: Switching from op1 to op2 must go through a null state (i.e. rolling back op1)
void DiagramWindowUserInputManager::processInput(DiagramWindowUserInputManager::UserInput operation, GLFWwindow *DisplayWindow)
{
	// 打断当前操作
	if (currentUserInputState != CANCEL && currentUserInputState != operation) {
		clearInput(DisplayWindow);
	}

	switch (operation)
	{
	case DiagramWindowUserInputManager::DRAG:
		startDragging(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::SCALE:
		startScaling(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::BLOCKDRAG:
		startBlockDragging(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::COMPILE:
		startCompiling(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::CANCEL:
		clearInput(DisplayWindow);
		break;
	default:
		break;
	}
}

void DiagramWindowUserInputManager::updateInput(DiagramWindowUserInputManager::UserInput operation, GLFWwindow *DisplayWindow, double arg1, double arg2)
{
	// 送进来的update信号，只有和currentState match时才响应
	if (currentUserInputState == operation) {
		switch (operation)
		{
		case DiagramWindowUserInputManager::DRAG:
			doDragging(DisplayWindow, arg1, arg2);
			break;
		case DiagramWindowUserInputManager::SCALE:
			doScaling(DisplayWindow, arg1, arg2);
			break;
		case DiagramWindowUserInputManager::BLOCKDRAG:
			doBlockDragging(DisplayWindow, arg1, arg2);
			break;
		case DiagramWindowUserInputManager::PORTDRAG:
			doPortDragging(DisplayWindow, arg1, arg2);
			break;
		case DiagramWindowUserInputManager::COMPILE:
			doCompiling(DisplayWindow);
			break;
		case DiagramWindowUserInputManager::CANCEL:
			// NO-OP
			break;
		default:
			break;
		}
	}
}


void DiagramWindowUserInputManager::clearInput(GLFWwindow *DisplayWindow)
{
	switch (currentUserInputState)
	{
	case DiagramWindowUserInputManager::DRAG:
		stopDragging(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::SCALE:
		stopScaling(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::BLOCKDRAG:
		stopBlockDragging(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::PORTDRAG:
		stopPortDragging(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::COMPILE:
		stopCompiling(DisplayWindow);
		break;
	case DiagramWindowUserInputManager::CANCEL:
		// NO-OP
		break;
	default:
		break;
	}
}

void DiagramWindowUserInputManager::mousebutton_callback(GLFWwindow *DisplayWindow, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {

		if (action == GLFW_PRESS) {
			processInput(DRAG, DisplayWindow);
		}
		else if (action == GLFW_RELEASE) {
			processInput(CANCEL, DisplayWindow); //这里有bug：如果current state不是drag，不应cancel
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT) {

		if (action == GLFW_PRESS) {
			processInput(BLOCKDRAG, DisplayWindow);
		}
		else if (action == GLFW_RELEASE) {
			processInput(CANCEL, DisplayWindow); //这里有bug：如果current state不是drag，不应cancel
		}
	}
}

void DiagramWindowUserInputManager::mousemove_callback(GLFWwindow *DisplayWindow, double xPos, double yPos)
{
	updateInput(DRAG, DisplayWindow, xPos, yPos);
	updateInput(BLOCKDRAG, DisplayWindow, xPos, yPos);
	updateInput(PORTDRAG, DisplayWindow, xPos, yPos);
}

void DiagramWindowUserInputManager::mousescroll_callback(GLFWwindow* DisplayWindow, double xoffset, double yoffset)
{
	processInput(SCALE, DisplayWindow);
	updateInput(SCALE, DisplayWindow, xoffset, yoffset);
	processInput(CANCEL, DisplayWindow);
}

void DiagramWindowUserInputManager::startDragging(GLFWwindow *DisplayWindow)
{
	// pan the view (pivot: the dragging point)
	double x, y;
	glfwGetCursorPos(DisplayWindow, &x, &y);
	x = floor(x); y = floor(y);

	// record the current pivot point
	pivotPos = Vec2(DiagramWindowInfo::getInstance().viewportTopLeftCorner.x + x / DiagramWindowInfo::getInstance().viewportScaleFactor,
		DiagramWindowInfo::getInstance().viewportTopLeftCorner.y - y / DiagramWindowInfo::getInstance().viewportScaleFactor);

	// start to drag!
	currentUserInputState = DRAG;
}

void DiagramWindowUserInputManager::doDragging(GLFWwindow *DisplayWindow, double x, double y)
{
	x = floor(x); y = floor(y);

	// update top-left w.r.t. pivot point
	DiagramWindowInfo::getInstance().viewportTopLeftCorner =
		Vec2(pivotPos.x - x / DiagramWindowInfo::getInstance().viewportScaleFactor,
		pivotPos.y + y / DiagramWindowInfo::getInstance().viewportScaleFactor);

	DiagramWindowInfo::getInstance().needUpdateMVP = true;
}

void DiagramWindowUserInputManager::stopDragging(GLFWwindow *DisplayWindow)
{
	// stop dragging!
	currentUserInputState = CANCEL;
}

void DiagramWindowUserInputManager::startScaling(GLFWwindow *DisplayWindow)
{
	// start to scale!
	currentUserInputState = SCALE;
}

void DiagramWindowUserInputManager::doScaling(GLFWwindow *DisplayWindow, double xoffset, double yoffset)
{
	// 一格yoffset=1

	// scale the view (pivot: center point)

	double reciprocalFactor = 1.0 / DiagramWindowInfo::getInstance().viewportScaleFactor;
	// update scaleFactor (clamp: [0.1 - 3])
	DiagramWindowInfo::getInstance().viewportScaleFactor += 0.04*yoffset;
	if (DiagramWindowInfo::getInstance().viewportScaleFactor < 0.5)
		DiagramWindowInfo::getInstance().viewportScaleFactor = 0.5f;
	else if (DiagramWindowInfo::getInstance().viewportScaleFactor > 3.0)
		DiagramWindowInfo::getInstance().viewportScaleFactor = 3.0f;
	reciprocalFactor -= 1.0 / DiagramWindowInfo::getInstance().viewportScaleFactor; // f = 1/F - 1/F'

	// update top-left w.r.t. scaleFactor
	DiagramWindowInfo::getInstance().viewportTopLeftCorner = Vec2(
		DiagramWindowInfo::getInstance().viewportTopLeftCorner.x + DiagramWindowInfo::getInstance().Width / 2.0 * reciprocalFactor,
		DiagramWindowInfo::getInstance().viewportTopLeftCorner.y - DiagramWindowInfo::getInstance().Height / 2.0 * reciprocalFactor);

	DiagramWindowInfo::getInstance().needUpdateMVP = true;
}

void DiagramWindowUserInputManager::stopScaling(GLFWwindow *DisplayWindow)
{
	// stop scaling!
	currentUserInputState = CANCEL;
}

void DiagramWindowUserInputManager::startBlockDragging(GLFWwindow *DisplayWindow)
{
	// pan the view (pivot: the dragging point)
	double x, y;
	glfwGetCursorPos(DisplayWindow, &x, &y);
	x = floor(x); y = floor(y);

	Vec2 pos = Vec2(DiagramWindowInfo::getInstance().viewportTopLeftCorner.x + x / DiagramWindowInfo::getInstance().viewportScaleFactor,
		DiagramWindowInfo::getInstance().viewportTopLeftCorner.y - y / DiagramWindowInfo::getInstance().viewportScaleFactor);

	// determine the block to drag (based on rendering order)
	// todo: blockOrderList提到mouse callback里，把connection和block的响应函数分开
	int pickArg = 0;
	pivotBlock = NULL;
	for (auto it = BlockGraph::getInstance().blockOrderList.rbegin(); it != BlockGraph::getInstance().blockOrderList.rend(); ++it) if ((pickArg = (*it)->IsPicked(pos)) != 0) {
		if (dynamic_cast<Connection*>(*it)) { // connections?
			Connection *c = dynamic_cast<Connection*>(*it);

			// record the dragged connection
			pivotConnection = c;
			pivotConnectionIsInputPort = (pickArg == 2);

			// update rendering order (connection)
			BlockGraph::getInstance().blockOrderList.erase(std::next(it).base()); // &*(reverse_iterator(i)) == &*(i - 1)
			BlockGraph::getInstance().blockOrderList.push_back(pivotConnection);

			// rendering order changed; ask redraw
			DiagramWindowInfo::getInstance().needRedrawDiagram = true;

			// start to portdrag!
			currentUserInputState = PORTDRAG;

			return;
		}
		else if (dynamic_cast<Block*>(*it)) { // blocks?

			Block *b = dynamic_cast<Block*>(*it);

			// 1) check if creating connections
			for (int i = 0; i < b->srcBlocks.size(); i++) if (overlaps(pos, b->GetInputPortRenderRec(i)) && b->srcBlocks[i] == NULL) {
				// create new connection
				pivotConnection = BlockGraph::getInstance().AddConnection(NULL, 0, b, i);
				pivotConnectionIsInputPort = false; // dragging the 'from' part

				pivotConnection->fromPos = pos;

				// rendering order changed; ask redraw
				DiagramWindowInfo::getInstance().needRedrawDiagram = true;

				// start to portdrag!
				currentUserInputState = PORTDRAG;

				return;
			}
			for (int i = 0; i < b->dstBlocks.size(); i++) if (overlaps(pos, b->GetOutputPortRenderRec(i)) && b->dstBlocks[i] == NULL) {
				// create new connection
				pivotConnection = BlockGraph::getInstance().AddConnection(b, i, NULL, 0);
				pivotConnectionIsInputPort = true; // dragging the 'to' part

				pivotConnection->toPos = pos;

				// rendering order changed; ask redraw
				DiagramWindowInfo::getInstance().needRedrawDiagram = true;

				// start to portdrag!
				currentUserInputState = PORTDRAG;

				return;
			}


			// record the dragged block
			pivotBlock = b;

			// record the current pivot point ( = drag point - renderRec)		
			pivotPos = { pos.x - pivotBlock->renderRec.pos.x, pos.y - pivotBlock->renderRec.pos.y };

			// update rendering order (block & connections) (concern: performance)
			BlockGraph::getInstance().blockOrderList.erase(std::next(it).base()); // &*(reverse_iterator(i)) == &*(i - 1)
			BlockGraph::getInstance().blockOrderList.push_back(pivotBlock);
			for (int i = 0; i < pivotBlock->srcBlocks.size(); i++) if (pivotBlock->srcBlocks[i]) {
				BlockGraph::getInstance().blockOrderList.remove(pivotBlock->srcBlocks[i]);
				BlockGraph::getInstance().blockOrderList.push_back(pivotBlock->srcBlocks[i]);
			}
			for (int i = 0; i < pivotBlock->dstBlocks.size(); i++) if (pivotBlock->dstBlocks[i]) {
				BlockGraph::getInstance().blockOrderList.remove(pivotBlock->dstBlocks[i]);
				BlockGraph::getInstance().blockOrderList.push_back(pivotBlock->dstBlocks[i]);
			}

			// rendering order changed; ask redraw
			DiagramWindowInfo::getInstance().needRedrawDiagram = true;

			// start to blockdrag!
			currentUserInputState = BLOCKDRAG;

			return;
		}
	}
}

void DiagramWindowUserInputManager::doBlockDragging(GLFWwindow *DisplayWindow, double x, double y)
{
	x = floor(x); y = floor(y);
	Vec2 pos = Vec2(DiagramWindowInfo::getInstance().viewportTopLeftCorner.x + x / DiagramWindowInfo::getInstance().viewportScaleFactor,
		DiagramWindowInfo::getInstance().viewportTopLeftCorner.y - y / DiagramWindowInfo::getInstance().viewportScaleFactor);

	// update block position w.r.t. pivot point
	pivotBlock->setPosition({ pos.x - pivotPos.x, pos.y - pivotPos.y, pivotBlock->renderRec.size.x, pivotBlock->renderRec.size.y });

	DiagramWindowInfo::getInstance().needRedrawDiagram = true;
}

void DiagramWindowUserInputManager::stopBlockDragging(GLFWwindow *DisplayWindow)
{
	// stop blockdragging!
	currentUserInputState = CANCEL;
}

bool DiagramWindowUserInputManager::updateConnectionForValidPort(Vec2 pos) {
	for (auto it = BlockGraph::getInstance().blockOrderList.rbegin(); it != BlockGraph::getInstance().blockOrderList.rend(); ++it)
	if ((*it) != pivotConnection && (*it)->IsPicked(pos) != 0) {
		Block *b = dynamic_cast<Block*>(*it);
		if (b && b != ((pivotConnectionIsInputPort) ? pivotConnection->from : pivotConnection->to)){ // not connecting to the same block

			if (pivotConnectionIsInputPort) {
				// the port should be picked and not occupied by other connection; if not valid try the next (don't skip) coz rendering area can be overlapping
				for (int i = 0; i < b->srcBlocks.size(); i++)
				if (overlaps(pos, b->GetInputPortRenderRec(i)) && (b->srcBlocks[i] == NULL || b->srcBlocks[i] == pivotConnection)) {
					pivotConnection->SetTo(b, i); // update Connection data
					return true;
				}
			}
			else {
				for (int i = 0; i < b->dstBlocks.size(); i++)
				if (overlaps(pos, b->GetOutputPortRenderRec(i)) && (b->dstBlocks[i] == NULL || b->dstBlocks[i] == pivotConnection)) {
					pivotConnection->SetFrom(b, i); // update Connection data
					return true;
				}
			}
		}
		break; // if block is hit and invalid, skip the rest
	}
	return false;
}

void DiagramWindowUserInputManager::doPortDragging(GLFWwindow *DisplayWindow, double x, double y)
{
	x = floor(x); y = floor(y);
	Vec2 pos = Vec2(DiagramWindowInfo::getInstance().viewportTopLeftCorner.x + x / DiagramWindowInfo::getInstance().viewportScaleFactor,
		DiagramWindowInfo::getInstance().viewportTopLeftCorner.y - y / DiagramWindowInfo::getInstance().viewportScaleFactor);

	// if new position is valid port, update connection (磁铁)
	if (!updateConnectionForValidPort(pos)) {
		// failed: update connection position w.r.t. mouse point
		if (pivotConnectionIsInputPort)
			pivotConnection->toPos = pos;
		else
			pivotConnection->fromPos = pos;
	}

	DiagramWindowInfo::getInstance().needRedrawDiagram = true;
}

void DiagramWindowUserInputManager::stopPortDragging(GLFWwindow *DisplayWindow)
{
	double x, y;
	glfwGetCursorPos(DisplayWindow, &x, &y);
	x = floor(x); y = floor(y);
	Vec2 pos = Vec2(DiagramWindowInfo::getInstance().viewportTopLeftCorner.x + x / DiagramWindowInfo::getInstance().viewportScaleFactor,
		DiagramWindowInfo::getInstance().viewportTopLeftCorner.y - y / DiagramWindowInfo::getInstance().viewportScaleFactor);

	// if new position is valid port, update connection (磁铁)
	if (!updateConnectionForValidPort(pos)) {
		// failed: delete connection (todo: wrap up into BlockGraph Method)
		BlockGraph::getInstance().RemoveConnection(pivotConnection);
		pivotConnection = NULL;
	}
	DiagramWindowInfo::getInstance().needRedrawDiagram = true;

	// stop portdragging!
	currentUserInputState = CANCEL;
}



void DiagramWindowUserInputManager::startCompiling(GLFWwindow *DisplayWindow)
{
	std::string fragStr = CodeGenManager::getInstance().GenerateFragShader();

	FILE *file;
	fopen_s(&file, AppState::OutputShaderName.c_str(), "w");
	fprintf(file, "%s", fragStr.c_str());
	fclose(file);
	DisplayWindowInfo::getInstance().needUpdateShader = true;

	// start to compile!
	currentUserInputState = COMPILE;

}
void DiagramWindowUserInputManager::doCompiling(GLFWwindow *DisplayWindow)
{
	// NO-OP 不希望被打断，放start里做了
}
void DiagramWindowUserInputManager::stopCompiling(GLFWwindow *DisplayWindow)
{
	// stop compiling!
	currentUserInputState = CANCEL;
}