#include "codegen.hpp"

#include "block.hpp"

std::string CodeGenManager::GenerateBlockDefinitions() {
	std::string impl;

	// todo: 跳过重复的block定义
	for (auto it = BlockGraph::getInstance().blockList.begin(); it != BlockGraph::getInstance().blockList.end(); ++it) {
		impl += (*it)->GenerateDefinition();
	}

	return impl;
}


std::string CodeGenManager::GenerateScene() {
	std::string impl;

	for (auto it = BlockGraph::getInstance().blockList.begin(); it != BlockGraph::getInstance().blockList.end(); ++it) if (dynamic_cast<ScreenBlock *>(*it)) {
		impl = (*it)->GenerateCallsite();
	}

	return R"(
float scene(vec3 p)
{
	return )" + (impl.empty() ? "0.0" : impl) + R"(;
}
)";

}