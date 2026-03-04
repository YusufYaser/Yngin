#include <Yngin/Core/Context.h>
#include <Yngin/Services/Service.h>
#include "Services_Internal.h"

namespace Yngin {
	namespace Services {
		Service::Service(Context* ctx) {
			impl = std::make_unique<Impl>();
			impl->ctx = ctx;
		}

		Service::~Service() = default;

		Context* Service::getContext() {
			return impl->ctx;
		}

		void Service::onUpdate() {}
	}
}