#include "UI.h"
#include <ImGui/imgui.h>
#include <Yngin/Yngin.h>
#include "../Editor.h"
#ifdef _WIN32
#include <Windows.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

using namespace Yngin;

bool EditorUI::textureSelector(std::string id, uint32_t* v) {
	bool changed = false;

	Texture* tex = editor->ctx->getTexturesManager()->getTexture(*v);

	std::string name = "(Invalid)";

	if (tex) {
		name = tex->meta.getMetaString("Editor.Name", "no_name");
		if (name == "no_name") {
			name = "Texture #" + std::to_string(tex->getId());
		} else {
			name += " (" + std::to_string(tex->getId()) + ")";
		}
	}

	ImGui::BeginGroup();
	if (ImGui::Button((name + "##TextureSelector" + id).c_str(), { -1, 0 })) {
		ImGui::OpenPopup(("TextureSelectorPopup" + id).c_str());
	}
	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_ID")) {
			*v = *(uint32_t*)payload->Data;
			changed = true;
		}

		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopup(("TextureSelectorPopup" + id).c_str())) {
		auto textures = editor->ctx->getTexturesManager()->getTextures();

		if (ImGui::BeginChild("TextureSelectorPopupContent", ImVec2(144 * 5, 192))) {
			int i = 0;

			for (auto& t : textures) {
				unsigned int GLid = t->getGLid();

				if (i % 5 != 0) ImGui::SameLine();

				if (t->getId() == *v) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, .5f, 0, 1));

				if (ImGui::ImageButton(std::string("TextureSelectorPopup #" + std::to_string(t->getId())).c_str(), (void*)(intptr_t)GLid, ImVec2(128, 128))) {
					if (t->getId() == *v) ImGui::PopStyleColor();
					*v = t->getId();
					changed = true;
				} else if (t->getId() == *v) {
					ImGui::PopStyleColor();
				}

				i++;
			}
			ImGui::EndChild();
		}
		ImGui::EndPopup();
	}

	return changed;
}

bool EditorUI::modelSelector(std::string id, uint32_t* v) {
	bool changed = false;

	Model* model = editor->ctx->getModelsManager()->getModel(*v);

	std::string name = "(Invalid)";

	if (model) {
		name = model->meta.getMetaString("Editor.Name", "no_name");
		if (name == "no_name") {
			name = "Model #" + std::to_string(model->getId());
		} else {
			name += " (" + std::to_string(model->getId()) + ")";
		}
	}

	ImGui::BeginGroup();
	if (ImGui::Button((name + "##ModelSelector" + id).c_str(), { -1, 0 })) {
		ImGui::OpenPopup(("ModelSelectorPopup" + id).c_str());
	}
	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_ID")) {
			*v = *(uint32_t*)payload->Data;
			changed = true;
		}

		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopup(("ModelSelectorPopup" + id).c_str())) {
		auto models = editor->ctx->getModelsManager()->getModels();

		if (ImGui::BeginChild("ModelSelectorPopupContent", ImVec2(128, 192))) {
			int i = 0;

			for (auto& t : models) {
				if (ImGui::Selectable(t->meta.getMetaString("Editor.Name", std::string("Model #" + std::to_string(t->getId())).c_str()).c_str())) {
					*v = t->getId();
					changed = true;
				}

				i++;
			}
			ImGui::EndChild();
		}
		ImGui::EndPopup();
	}

	return changed;
}

bool EditorUI::materialSelector(std::string id, uint32_t* v) {
	bool changed = false;

	Material* mat = editor->ctx->getMaterialsManager()->getMaterial(*v);

	std::string name = "(Invalid)";

	if (mat) {
		name = mat->meta.getMetaString("Editor.Name", "no_name");
		if (name == "no_name") {
			name = "Material #" + std::to_string(mat->getId());
		} else {
			name += " (" + std::to_string(mat->getId()) + ")";
		}
	}

	ImGui::BeginGroup();
	if (ImGui::Button((name + "##MaterialSelector" + id).c_str(), { -1, 0 })) {
		ImGui::OpenPopup(("MaterialSelectorPopup" + id).c_str());
	}
	ImGui::EndGroup();

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_ID")) {
			*v = *(uint32_t*)payload->Data;
			changed = true;
		}

		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopup(("MaterialSelectorPopup" + id).c_str())) {
		auto materials = editor->ctx->getMaterialsManager()->getMaterials();

		if (ImGui::BeginChild("MaterialSelectorPopupContent", ImVec2(128, 192))) {
			int i = 0;

			for (auto& t : materials) {
				if (ImGui::Selectable(t->meta.getMetaString("Editor.Name", std::string("Material #" + std::to_string(t->getId())).c_str()).c_str())) {
					*v = t->getId();
					changed = true;
				}

				i++;
			}
			ImGui::EndChild();
		}
		ImGui::EndPopup();
	}

	return changed;
}

bool EditorUI::fileSelector(std::string id, std::map<std::string, std::string> filters, char* path, size_t pathSize) {
#ifdef _WIN32
	{
		if (ImGui::Button(("Select File##" + id).c_str(), ImVec2(-1, 40))) {
			GLFWwindow* window = editor->ctx->getWindow()->getGLFWwindow();
			HWND hwnd = glfwGetWin32Window(window);

			OPENFILENAMEA ofn = {};

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFile = path;
			ofn.nMaxFile = pathSize;
			ofn.lpstrTitle = "Select File";
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
			ofn.lpstrInitialDir = ".";

			std::string lpstrFilter;
			for (auto& [filterName, filterPattern] : filters) {
				lpstrFilter += filterName + " (" + filterPattern + ")";
				lpstrFilter.push_back('\0');
				lpstrFilter += filterPattern;
				lpstrFilter.push_back('\0');
			}
			lpstrFilter.push_back('\0');
			ofn.lpstrFilter = lpstrFilter.c_str();

			ofn.nFilterIndex = 1;

			if (GetOpenFileNameA(&ofn)) {
				return true;
			}
		}
	}
#else
	{
		ImGui::Text("Path");
		ImGui::SameLine(50);
		ImGui::PushItemWidth(-1);
		ImGui::InputText(("##New " + id + " Path").c_str(), path, pathSize);
		ImGui::PopItemWidth();

		if (ImGui::Button("Open File", ImVec2(-1, 0))) {
			return true;
		}
	}
#endif

	return false;
}
