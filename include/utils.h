#ifndef UTILS_H
#define UTILS_H

#include <type_traits>
#include <string>
#include <sstream>
#include <iostream>
#include <cxxabi.h>
#include <memory>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include "json.h"

namespace pge
{

/* FS UTILS (I know that I will need them):
============================================================================= */

/* TIME UTILS:
============================================================================= */

inline void wait_ms(uint64_t ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline uint64_t get_time_ms() {
	using namespace std::chrono;
	return duration_cast<milliseconds >(
		system_clock::now().time_since_epoch()
	).count();
}

struct TimePointMs {
	uint64_t start;
	TimePointMs() {
		start = get_time_ms();
	}

	uint64_t elapsed() {
		return get_time_ms() - start;
	}
};

/* DEMANGLE:
============================================================================= */

inline std::string demangle(std::string sname) {
	const char *name = sname.c_str();
	int status = -4; // some arbitrary value to eliminate the compiler warn

	// enable c++11 by passing the flag -std=c++11 to g++
	std::unique_ptr<char, void(*)(void*)> res {
	    abi::__cxa_demangle(name, NULL, NULL, &status),
	    std::free
	};

	return (status==0) ? res.get() : name;
}

template <class T>
inline std::string demangle() {
	return demangle(typeid(T).name());
}

/* SFORMAT:
============================================================================= */

template <typename T>
concept FundamentalPrintf = std::is_fundamental_v<T> ||
		std::is_same_v<std::decay_t<T>, std::decay_t<const char *>> ||
		std::is_same_v<std::decay_t<T>, std::decay_t<char *>>;

template <typename T>
concept CanPrintf = !FundamentalPrintf<T> && (
	requires(T a) { std::string() = a.to_string(); } ||
	requires(T a) { std::string() = a; } ||
	requires(T a) { std::string() = to_string(a); } ||
	requires(T a) { std::string() = std::to_string(a); } ||
	requires(T a) { std::string() = a.c_str(); } ||
	requires(T a) { std::stringstream() << a; }
);

template <CanPrintf T>
		requires requires(T a) { std::string() = a.c_str(); }
inline std::string sformat_get_str(const T& obj, char, char, char, char, char, char) {
	return obj.c_str();
} 

template <CanPrintf T>
		requires requires(T a) { std::string() = a; }
inline std::string sformat_get_str(const T& obj, char, char, char, char, char, float) {
	return obj;
}

template <CanPrintf T>
		requires requires(T a) { std::string() = to_string(a); }
inline std::string sformat_get_str(const T& obj, char, char, char, char, float, float) {
	return to_string(obj);
}

template <CanPrintf T>
		requires requires(T a) { std::string() = std::to_string(a); }
inline std::string sformat_get_str(const T& obj, char, char, char, float, float, float) {
	return to_string(obj);
}

template <CanPrintf T>
		requires requires(T a) { std::string() = a.to_string(); }
inline std::string sformat_get_str(const T& obj, char, char, float, float, float, float) {
	return obj.to_string();
} 

template <CanPrintf T>
		requires requires(T a) { std::stringstream() << a; }
inline std::string sformat_get_str(const T& obj, char, float, float, float, float, float) {
	std::stringstream ss;
	ss << obj;
	return ss.str();
}

template <typename T> requires FundamentalPrintf<T> || CanPrintf<T>
struct sformat_arg_t {
	template <typename U>
	sformat_arg_t(U&& obj) {}
};

template <FundamentalPrintf T>
struct sformat_arg_t<T> {
	T obj;

	template <typename U>
	sformat_arg_t(U&& obj) : obj(obj) {}
	T held_type() { return obj; }
};

template <CanPrintf T>
struct sformat_arg_t<T> {
	std::string str;

	template <typename U>
	sformat_arg_t(U&& obj) {
		str = sformat_get_str(obj, 'c', 'c', 'c', 'c', 'c', 'c');
	}

	const char *held_type() { return str.c_str(); }
};

template <typename ...Args>
inline std::string sformat(const char *fmt, Args&&...args) {
	char buff[1024] = {0};
	snprintf(buff, sizeof(buff), fmt,
			sformat_arg_t<std::decay_t<Args>>(args).held_type()...);
	return buff;
}

/* LOGGING: DBG and EXCEPTIONS:
============================================================================= */

// need 2 of them for the ocasions where we use EXCEPTION inside a macro
#define DBG2(fmt, ...) printf("%s", pge::sformat("[%s:%d] %s() --> " fmt "\n",\
		__FILE__, __LINE__, __func__, ##__VA_ARGS__).c_str());
#define DBG(fmt, ...) printf("%s", pge::sformat("[%s:%d] %s() --> " fmt "\n",\
		__FILE__, __LINE__, __func__, ##__VA_ARGS__).c_str());


#define EXCEPTION(fmt, ...) \
	throw (std::runtime_error([&](const char *file, int line, const char *fn){\
		DBG2(fmt, ##__VA_ARGS__);\
		std::string str;\
		str = pge::sformat("[file: %s][line: %d] %s() -> " fmt "\nbt:\n",\
				file, line, fn, ##__VA_ARGS__);\
		return str;\
	}(__FILE__, __LINE__, __func__).c_str()))

/* JSON UTILS:
============================================================================= */

#define JSON_HELPER(cfg, name, x) ([&] {\
	if (cfg.find(name) == cfg.end())\
		EXCEPTION("\"%s\" not found in json", name);\
	try {\
		return x;\
	}\
	catch(std::exception &e) {\
		EXCEPTION("ewhat: %s", e.what());\
	}\
}())

#define JSON_GET(cfg, name) JSON_HELPER(cfg, name, (cfg)[(name)])

#define JSON_STR(cfg, n) JSON_HELPER((cfg), (n), \
		(cfg)[(n)].get_ref<const std::string&>().c_str())
#define JSON_INT(cfg, n) JSON_HELPER((cfg), (n), (cfg)[(n)].get<int>())
#define JSON_BOOL(cfg, n) JSON_HELPER((cfg), (n), (cfg)[(n)].get<bool>())
#define JSON_FLOAT(cfg, n) JSON_HELPER((cfg), (n), (cfg)[(n)].get<float>())
#define JSON_CFG(cfg, n) JSON_HELPER((cfg), (n), (cfg)[(n)])
#define JSON_SSTR(cfg, n) JSON_HELPER((cfg), (n), (cfg)[(n)].get<std::string>())

nlohmann::json load_config(const std::string& path) {
	nlohmann::json jret;
	std::ifstream in(path);
	if (!in.good()) {
		EXCEPTION("failed to load config file: %s", path.c_str());
	}
	try {
		in >> jret;
		jret["base_path"] =
				std::filesystem::canonical(path).parent_path().string() + "/";
		return jret;
	} catch (std::exception &e) {
		EXCEPTION("load config failed: %s", e.what());
	}
}

/* SHARED OBJECT UTILS:
============================================================================= */

#define EXTERN_FN extern "C" __attribute__ ((visibility ("default")))

#define SO_DECLARE_FN(fn) decltype(fn) *fn ## _fn;
#define SO_LOAD_FN(handle, fn)\
	fn ## _fn = (decltype(fn) *)dlsym(handle, #fn);\
	if (!fn ## _fn) {\
		DBG("Couldn't load function %s", #fn);\
		break;\
	}


}

#endif
