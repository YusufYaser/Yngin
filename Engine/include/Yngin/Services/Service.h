#pragma once
#include <Yngin/Core/Context.h>
#include <memory>

namespace Yngin {
	namespace Services {
		class Service {
		public:
			Service(Context* ctx);
			~Service();

			Context* getContext();

		protected:
			struct Impl;
			std::unique_ptr<Impl> impl;

		private:
			virtual void onUpdate();

			friend class Context;
			friend struct std::default_delete<Service>;
		};
	}
}
