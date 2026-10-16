#include "Scene.hpp"

std::shared_ptr<Scene> SceneManager::activeScene;

EngineState SceneManager::state = EngineState::Editor;