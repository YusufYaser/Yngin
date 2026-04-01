#include <Yngin/Utils/Meta.h>
#include "Meta_Internal.h"

namespace Yngin {
	Meta::Meta() {
		impl = std::make_unique<Impl>();
	}

	Meta::~Meta() = default;

	void Meta::reset() {
		impl->metas.clear();
	}

	void Meta::clearMeta(const std::string& key) {
		impl->metas.erase(key);
	}

	void Meta::setMeta(const std::string& key, const std::string& value) {
		impl->metas[key] = value;
	}

	void Meta::setMeta(const std::string& key, const int& value) {
		impl->metas[key] = value;
	}

	void Meta::setMeta(const std::string& key, const float& value) {
		impl->metas[key] = value;
	}

	void Meta::setMeta(const std::string& key, void* value) {
		impl->metas[key] = value;
	}

	size_t Meta::getMetasCount() const {
		return impl->metas.size();
	}

	std::map<std::string, MetaValue> Meta::getMetas() const {
		return impl->metas;
	}

	MetaValue Meta::getMeta(const std::string& key, const MetaValue& defaultValue) const {
		auto it = impl->metas.find(key);
		if (it != impl->metas.end()) {
			return it->second;
		}

		return defaultValue;
	}

	std::string Meta::getMetaString(const std::string& key, const std::string& defaultValue) const {
		auto it = impl->metas.find(key);
		if (it != impl->metas.end()) {
			if (auto val = std::get_if<std::string>(&it->second)) {
				return *val;
			}
		}

		return defaultValue;
	}

	int Meta::getMetaInt(const std::string& key, const int& defaultValue) const {
		auto it = impl->metas.find(key);
		if (it != impl->metas.end()) {
			if (auto val = std::get_if<int>(&it->second)) {
				return *val;
			}
		}

		return defaultValue;
	}

	float Meta::getMetaFloat(const std::string& key, const float& defaultValue) const {
		auto it = impl->metas.find(key);
		if (it != impl->metas.end()) {
			if (auto val = std::get_if<float>(&it->second)) {
				return *val;
			}
		}

		return defaultValue;
	}

	void* Meta::getMetaPtr(const std::string& key, void* defaultValue) const {
		auto it = impl->metas.find(key);
		if (it != impl->metas.end()) {
			if (auto val = std::get_if<void*>(&it->second)) {
				return *val;
			}
		}

		return defaultValue;
	}
}
