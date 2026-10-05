#pragma once
#include <Yngin/Forward.h>
#include "Service.h"

namespace Yngin {
	namespace Services {
		class SerializationService : public Service {
		public:
			void pushSkipMetaKeys(const std::vector<std::string>& values);
			std::vector<std::string> popSkipMetaKeys();
			void pushIgnoredMetaPrefixes(const std::vector<std::string>& values);
			std::vector<std::string> popIgnoredMetaPrefixes();

			bool validate(std::istream& in);

			bool serialize(std::ostream& out, const Meta& input);

			bool serialize(std::ostream& out, ModelsManager* input, bool includeDependencies = true);
			bool serialize(std::ostream& out, Model* input, bool includeDependencies = true);

			bool serialize(std::ostream& out, MaterialsManager* input);
			bool serialize(std::ostream& out, Material* input);

			bool serialize(std::ostream& out, TexturesManager* input, bool compressed = true);
			bool serialize(std::ostream& out, Texture* input, bool compressed = true);

			bool serialize(std::ostream& out, ScriptsManager* input);
			bool serialize(std::ostream& out, Script* input);

			bool serialize(std::ostream& out, CamerasManager* input);
			bool serialize(std::ostream& out, Camera* input);

			bool serialize(std::ostream& out, GameObjectsManager* input, bool includeComponentDependencies = true);
			// childrenDepth = -1 for infinity
			bool serialize(std::ostream& out, GameObject* input, int childrenDepth = -1, bool includeComponentDependencies = true);
			bool serialize(std::ostream& out, Components::Component* input, bool includeDependencies = true);

			bool serialize(std::ostream& out, UI::UIManager* input, bool includeDependencies = true);
			// childrenDepth = -1 for infinity
			bool serialize(std::ostream& out, UI::UIElement* input, int childrenDepth = -1, bool includeDependencies = true);

		private:
			friend class Context;
			friend struct std::default_delete<SerializationService>;

			SerializationService(Context* ctx);
			~SerializationService();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
