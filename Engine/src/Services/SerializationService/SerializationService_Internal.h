#pragma once
#include <Yngin/Services/SerializationService.h>
#include <Yngin/Utils/Meta.h>

namespace Yngin::Services {
	struct SerializationService::Impl {
		std::vector<std::vector<std::string>> skipMetaKeys;
		std::vector<std::string> currentSkipMetaKeys;
		std::vector<std::vector<std::string>> ignoredMetaPrefixes;
		std::vector<std::string> currentIgnoredMetaPrefixes;

		bool shouldSkip(const Meta& meta);
	};
}
