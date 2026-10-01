#include <Yngin/Services/SerializationService.h>
#include "SerializationService_Internal.h"

namespace Yngin::Services {
	SerializationService::SerializationService(Context* ctx) : Service(ctx) {
		impl = std::make_unique<Impl>();

		pushSkipMetaKeys({ "#NoExport" });
		pushIgnoredMetaPrefixes({ "#" });
	}

	SerializationService::~SerializationService() = default;

	void SerializationService::pushSkipMetaKeys(const std::vector<std::string>& values) {
		impl->skipMetaKeys.push_back(values);
		impl->currentSkipMetaKeys = impl->skipMetaKeys.back();
	}

	std::vector<std::string> SerializationService::popSkipMetaKeys() {
		if (impl->skipMetaKeys.empty()) {
			return {};
		}

		auto val = impl->skipMetaKeys.back();
		impl->skipMetaKeys.pop_back();

		if (!impl->skipMetaKeys.empty()) {
			impl->currentSkipMetaKeys = impl->skipMetaKeys.back();
		} else {
			impl->currentSkipMetaKeys.clear();
		}

		return val;
	}

	void SerializationService::pushIgnoredMetaPrefixes(const std::vector<std::string>& values) {
		impl->ignoredMetaPrefixes.push_back(values);
		impl->currentIgnoredMetaPrefixes = impl->ignoredMetaPrefixes.back();
	}

	std::vector<std::string> SerializationService::popIgnoredMetaPrefixes() {
		if (impl->ignoredMetaPrefixes.empty()) {
			return {};
		}

		auto val = impl->ignoredMetaPrefixes.back();
		impl->ignoredMetaPrefixes.pop_back();

		if (!impl->ignoredMetaPrefixes.empty()) {
			impl->currentIgnoredMetaPrefixes = impl->ignoredMetaPrefixes.back();
		} else {
			impl->currentIgnoredMetaPrefixes.clear();
		}

		return val;
	}

	bool SerializationService::Impl::shouldSkip(const Meta& meta) {
		if (meta.getMetasCount() == 0) {
			return false;
		}

		static const MetaValue empty{};

		for (auto& key : currentSkipMetaKeys) {
			if (meta.getMeta(key, empty) != empty) {
				return true;
			}
		}
		return false;
	}
}
