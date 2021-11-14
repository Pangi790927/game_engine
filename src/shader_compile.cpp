// object file to make compilation faster for shaders

#include "utils.h"
#include "shader_compile.h"

#include <shaderc/shaderc.hpp>
#include <string>
#include <fstream>
#include <streambuf>

[[maybe_unused]]
static std::string preprocess_shader(const std::string& source_name,
		shaderc_shader_kind kind, const std::string& source)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	// Like -DMY_DEFINE=1
	options.AddMacroDefinition("MY_DEFINE", "1");

	shaderc::PreprocessedSourceCompilationResult result =
			compiler.PreprocessGlsl(source, kind, source_name.c_str(), options);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		DBG("%s", result.GetErrorMessage());
		return "";
	}

	return {result.cbegin(), result.cend()};
}


// Compiles a shader to SPIR-V assembly. Returns the assembly text
// as a string.
[[maybe_unused]]
static std::string compile_file_to_assembly(const std::string& source_name,
		shaderc_shader_kind kind, const std::string& source,
		bool optimize = false)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	// Like -DMY_DEFINE=1
	options.AddMacroDefinition("MY_DEFINE", "1");
	if (optimize)
		options.SetOptimizationLevel(shaderc_optimization_level_size);

	shaderc::AssemblyCompilationResult result = compiler.CompileGlslToSpvAssembly(
		source, kind, source_name.c_str(), options);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		DBG("%s", result.GetErrorMessage());
		return "";
	}

	return {result.cbegin(), result.cend()};
}

static shaderc_shader_kind get_shader_kind(int shader_type) {
	switch (shader_type) {
		case SHC_VERTEX_SHADER:
			return shaderc_glsl_vertex_shader;
		case SHC_FRAGMENT_SHADER:
			return shaderc_glsl_fragment_shader;
		case SHC_COMPUTE_SHADER:
			return shaderc_glsl_compute_shader;
		case SHC_GEOMETRY_SHADER:
			return shaderc_glsl_geometry_shader;
		case SHC_TESS_CONTROL_SHADER:
			return shaderc_glsl_tess_control_shader;
		case SHC_TESS_EVALUATION_SHADER:
			return shaderc_glsl_tess_evaluation_shader;
		default: return shaderc_glsl_infer_from_source;
	}
}

// Compiles a shader to a SPIR-V binary. Returns the binary as
// a vector of 32-bit words.
static std::vector<uint32_t> compile_shader_src(const std::string& name,
		const std::string& source, int kind,
		bool optimize)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	// Like -DMY_DEFINE=1
	options.AddMacroDefinition("MY_DEFINE", "1");
	if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

	shaderc::SpvCompilationResult module =
		compiler.CompileGlslToSpv(source, get_shader_kind(kind),
		name.c_str(), options);

	if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
		DBG("%s", module.GetErrorMessage());
		return std::vector<uint32_t>{};
	}

	return {module.cbegin(), module.cend()};
}

static std::vector<uint32_t> compile_shader_path(const std::string& path,
		int kind, bool optimize)
{
	std::ifstream t(path);
	std::string src((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
	return compile_shader_src(path, src, kind, optimize);
}

EXTERN_FN uint32_t *shc_compile_path(const char *path, int kind,
		size_t *result_len, bool optimize)
{
	if (!result_len || !path) {
		DBG("name, src, result_len can't be NULL");
		return NULL;
	}
	auto vec = compile_shader_path(path, kind, optimize);
	*result_len = vec.size();
	uint32_t *ret = new uint32_t[vec.size()];
	memcpy(ret, vec.data(), vec.size() * sizeof(uint32_t));
	return ret;
}

EXTERN_FN uint32_t *shc_compile_src(const char *name, const char *src,
		int kind, size_t *result_len, bool optimize)
{
	if (!result_len || !name || !src) {
		DBG("name, src, result_len can't be NULL");
		return NULL;
	}
	auto vec = compile_shader_src(name, src, kind, optimize);
	*result_len = vec.size();
	uint32_t *ret = new uint32_t[vec.size()];
	memcpy(ret, vec.data(), vec.size() * sizeof(uint32_t));
	return ret;
}

EXTERN_FN int shc_free_shader(uint32_t *ptr) {
	delete [] ptr;
	return 0;
}

EXTERN_FN int shc_get_version() {
	return LIB_VERSION;
}