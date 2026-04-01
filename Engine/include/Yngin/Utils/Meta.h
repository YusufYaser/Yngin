#pragma once
#include <string>
#include <memory>
#include <vector>
#include <map>
#if (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (__cplusplus >= 201703L)
#define CPP17
#endif

#ifdef CPP17
#include <variant>
#endif

namespace Yngin {
#ifdef CPP17
	typedef std::variant<std::string, int, float, void*> MetaValue;
#endif

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

		size_t getMetasCount() const;

#ifdef CPP17
		std::map<std::string, MetaValue> getMetas() const;
		MetaValue getMeta(const std::string& key, const MetaValue& defaultValue = {}) const;
#endif

		std::string getMetaString(const std::string& key, const std::string& defaultValue = "") const;
		int getMetaInt(const std::string& key, const int& defaultValue = 0) const;
		float getMetaFloat(const std::string& key, const float& defaultValue = 0) const;
		void* getMetaPtr(const std::string& key, void* defaultValue = nullptr) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}

#undef CPP17
