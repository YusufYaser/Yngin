#pragma once
#include <Yngin/Utils/Meta.h>
#include <map>
#include <variant>

namespace Yngin {
	struct Meta::Impl {
		std::map<std::string, MetaValue> metas;
	};
}
