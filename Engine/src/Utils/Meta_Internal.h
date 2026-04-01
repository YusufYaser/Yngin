#pragma once
#include <Yngin/Utils/Meta.h>
#include <map>
#include <variant>

typedef std::variant<std::string, int, float, void*> MetaValue;

namespace Yngin {
	struct Meta::Impl {
		std::map<std::string, MetaValue> metas;
	};
}
