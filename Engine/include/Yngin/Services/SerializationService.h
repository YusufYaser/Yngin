#pragma once
#include <Yngin/Forward.h>
#include "Service.h"

namespace Yngin {
	namespace Services {
		struct DeserializationContext {
			Meta* targetMeta;
			Scene* targetScene;
			GameObject* targetGameObject;
			UI::UIManager* targetUIManager;
			bool allowLoadingTexturesFromDevice;
			bool ignoreErrorMissingContext;
			bool ignoreErrorConflictingId;
			bool overrideConflictingId;
			bool useOriginalConflictingObject;
		};

		enum class DESERIALIZATION_STATUS : uint8_t {
			OK = 0,
			GENERIC_ERROR,
			UNSUPPORTED_SCHEMA_VERSION,
			INVALID_DATA,
			MISSING_CONTEXT,
			STREAM_ERROR,
			CONFLICTING_ID,
			PERMISSION_ERROR,
		};

		struct CreateArchiveSettings {

		};

		class SerializationService : public Service {
		public:
			void pushSkipMetaKeys(const std::vector<std::string>& values);
			std::vector<std::string> popSkipMetaKeys();
			void pushIgnoredMetaPrefixes(const std::vector<std::string>& values);
			std::vector<std::string> popIgnoredMetaPrefixes();

			std::vector<char> createArchive(std::istream& serializedData, const CreateArchiveSettings& settings = {});
			bool loadArchive(std::istream& archive, const DeserializationContext& deserializationContext = {});
			uint32_t streamArchive(std::unique_ptr<std::istream>& archive, const DeserializationContext& deserializationContext = {});

			DESERIALIZATION_STATUS validate(std::istream& in);
			DESERIALIZATION_STATUS load(std::istream& in, const DeserializationContext& deserializationContext = {});

			bool serialize(std::ostream& out, const Meta& input);

			bool serialize(std::ostream& out, ModelsManager* input);
			bool serialize(std::ostream& out, Model* input);

			bool serialize(std::ostream& out, MaterialsManager* input);
			bool serialize(std::ostream& out, Material* input);

			bool serialize(std::ostream& out, TexturesManager* input, bool compressed = true);
			bool serialize(std::ostream& out, Texture* input, bool compressed = true);

			bool serialize(std::ostream& out, ScriptsManager* input);
			bool serialize(std::ostream& out, Script* input);

			bool serialize(std::ostream& out, CamerasManager* input);
			bool serialize(std::ostream& out, Camera* input);

			bool serialize(std::ostream& out, GameObjectsManager* input);
			// childrenDepth = -1 for infinity
			bool serialize(std::ostream& out, GameObject* input, int childrenDepth = -1);
			bool serialize(std::ostream& out, Components::Component* input);

			bool serialize(std::ostream& out, UI::UIManager* input);
			// childrenDepth = -1 for infinity
			bool serialize(std::ostream& out, UI::UIElement* input, int childrenDepth = -1);

		private:
			friend class Context;
			friend struct std::default_delete<SerializationService>;
			friend class Texture;
			friend class Material;
			friend class Model;

			SerializationService(Context* ctx);
			~SerializationService();

			struct Impl;
			std::unique_ptr<Impl> impl;
		};
	}
}
