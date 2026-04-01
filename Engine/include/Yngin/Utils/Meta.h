#pragma once
#include <string>
#include <memory>

namespace Yngin {
	class Meta {
	public:
		Meta();
		~Meta();

		void reset();
		void clearMeta(const std::string& key);

		void setMeta(const std::string& key, const std::string& value);
		void setMeta(const std::string& key, const int& value);
		void setMeta(const std::string& key, const float& value);
		void setMeta(const std::string& key, void* value);

		std::string getMetaString(const std::string& key, const std::string& defaultValue = "") const;
		int getMetaInt(const std::string& key, const int& defaultValue = 0) const;
		float getMetaFloat(const std::string& key, const float& defaultValue = 0) const;
		void* getMetaPtr(const std::string& key, void* defaultValue = nullptr) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
