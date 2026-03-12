#pragma once
#include <utility>
#include <Yngin/Core/Scenes.h>

enum class EXPLORER_SELECTION_TYPE {
	NONE,
	GAMEOBJECT,
	UIELEMENT
};

std::pair<EXPLORER_SELECTION_TYPE, int> showExplorer(Yngin::Scene* scene);
