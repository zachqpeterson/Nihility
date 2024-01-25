#pragma once

#include "Defines.hpp"

#include "Containers\TypeList.hpp"
#include "Component.hpp"

#include "Mesh.hpp"
#include "Rendering\UI.hpp"

using RegisteredComponents = TypeList<MeshComponent, ModelComponent, UIComponent>;