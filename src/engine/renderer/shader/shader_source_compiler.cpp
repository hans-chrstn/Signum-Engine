#include "shader_source_compiler.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/renderer/shader/shader_artifact.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <slang-com-ptr.h>
#include <slang.h>
#include <string>
#include <utility>

namespace Core = SNE::Engine::Core;
namespace Shader = SNE::Engine::Renderer::Shader;

namespace {
    struct ShaderFailureDetails {
        const char *message;
        const char *operation;
    };

    struct SlangFailureDetails {
        const char *message;
        const char *native_description;
        const char *operation;
    };

    auto validateShaderSource(const Shader::ShaderSourceDescription &source)
        -> void {

        if (source.path.empty()) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceInvalid,
                "Shader source path must not be empty",
                "Validate Shader Source");
        }

        if (source.entry_point.empty()) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceInvalid,
                "Shader source entry point must not be empty",
                "Validate Shader Source");
        }

        if (source.entry_point.contains('\0')) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceInvalid,
                "Shader source entry point contains an embedded null character",
                "Validate Shader Source");
        }

        if (source.language != Shader::ShaderSourceLanguage::Slang) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceInvalid,
                "Shader source language is not supported by the current "
                "compiler",
                "Validate Shader Source Language");
        }
    }

    [[nodiscard]] auto loadShaderSource(const std::filesystem::path &path)
        -> std::string {

        std::ifstream file{path, std::ios::binary | std::ios::ate};

        if (!file.is_open()) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceLoadFailed,
                "Failed to open shader source file", "Load Shader Source");
        }

        const std::streampos end_position = file.tellg();

        if (end_position == std::streampos{-1}) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceLoadFailed,
                "Failed to determine shader source file size",
                "Determine Shader Source Size");
        }

        const auto byte_size = static_cast<std::streamoff>(end_position);

        if (byte_size < 0) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceLoadFailed,
                "Shader source file has an invalid size",
                "Validate Shader Source Size");
        }

        if (byte_size > std::numeric_limits<std::streamsize>::max()) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceLoadFailed,
                "Shader source file exceeds supported size",
                "Validate Shader Source Size");
        }

        std::string source_text;

        if (std::cmp_greater(byte_size, source_text.max_size())) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceLoadFailed,
                "Shader source file exceeds supported size",
                "Validate Shader Source Size");
        }

        source_text.resize(static_cast<std::size_t>(byte_size));

        file.seekg(0, std::ios::beg);

        if (!file) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceLoadFailed,
                "Failed to seek to beginning of shader source file",
                "Seek Shader Source");
        }

        file.read(source_text.data(), static_cast<std::streamsize>(byte_size));

        if (!file) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceLoadFailed,
                "Failed to read shader source file completely",
                "Read Shader Source");
        }

        if (source_text.contains('\0')) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceInvalid,
                "Shader source contains an embedded null character",
                "Validate Shader Source Text");
        }

        return source_text;
    }

    auto appendSlangDiagnostics(std::string &diagnostics, slang::IBlob *blob)
        -> void {

        if (blob == nullptr) {
            return;
        }

        const auto *text = static_cast<const char *>(blob->getBufferPointer());

        const std::size_t byte_count = blob->getBufferSize();

        if (text == nullptr || byte_count == 0) {
            return;
        }

        const std::size_t text_length =
            text[byte_count - 1] == '\0' ? byte_count - 1 : byte_count;

        if (text_length == 0) {
            return;
        }

        if (!diagnostics.empty()) {
            diagnostics.push_back('\n');
        }

        diagnostics.append(text, text_length);
    }

    [[nodiscard]] auto makeDiagnosticMessage(const char *message,
                                             const std::string &diagnostics)
        -> std::string {

        std::string error_message{message};

        if (!diagnostics.empty()) {
            error_message.append(":\n");
            error_message.append(diagnostics);
        }

        return error_message;
    }

    [[noreturn]] auto
    throwShaderCompilationFailure(ShaderFailureDetails details,
                                  const std::string &diagnostics) -> void {

        const std::string error_message =
            makeDiagnosticMessage(details.message, diagnostics);

        throw Core::Error::EngineError(
            Core::Error::Code::ShaderSourceCompilationFailed,
            error_message.c_str(), details.operation);
    }

    [[noreturn]] auto
    throwSlangCompilationFailure(SlangFailureDetails details,
                                 SlangResult native_result,
                                 const std::string &diagnostics) -> void {

        const std::string error_message =
            makeDiagnosticMessage(details.message, diagnostics);

        throw Core::Error::EngineError(
            Core::Error::Code::ShaderSourceCompilationFailed,
            error_message.c_str(),
            Core::Error::NativeError(static_cast<int>(native_result),
                                     details.native_description),
            details.operation);
    }

    [[nodiscard]] auto createSlangGlobalSession()
        -> Slang::ComPtr<slang::IGlobalSession> {

        Slang::ComPtr<slang::IGlobalSession> global_session;

        const SlangResult result =
            slang::createGlobalSession(global_session.writeRef());

        if (SLANG_FAILED(result) || global_session == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderCompilerInitializationFailed,
                "Failed to initialize Slang compiler global session",
                Core::Error::NativeError(
                    static_cast<int>(result),
                    "Slang compiler global session initialization failed"),
                "Initialize Slang Compiler");
        }

        return global_session;
    }

    [[nodiscard]] auto createSlangSession(slang::IGlobalSession *global_session)
        -> Slang::ComPtr<slang::ISession> {

        slang::TargetDesc target_desc{};
        target_desc.format = SLANG_SPIRV;
        target_desc.profile = global_session->findProfile("spirv_1_5");

        slang::CompilerOptionEntry entry_point_name_option{};
        entry_point_name_option.name =
            slang::CompilerOptionName::VulkanUseEntryPointName;
        entry_point_name_option.value.kind =
            slang::CompilerOptionValueKind::Int;
        entry_point_name_option.value.intValue0 = 1;

        target_desc.compilerOptionEntries = &entry_point_name_option;
        target_desc.compilerOptionEntryCount = 1U;

        slang::SessionDesc session_desc{};
        session_desc.targets = &target_desc;
        session_desc.targetCount = 1;
        session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

        Slang::ComPtr<slang::ISession> session;

        const SlangResult result =
            global_session->createSession(session_desc, session.writeRef());

        if (SLANG_FAILED(result) || session == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderCompilerSessionCreationFailed,
                "Failed to create Slang compiler session",
                Core::Error::NativeError(
                    static_cast<int>(result),
                    "Slang compiler session initialization failed"),
                "Create Slang Compiler Session");
        }

        return session;
    }

    [[nodiscard]] auto
    loadSlangModule(slang::ISession *session,
                    const Shader::ShaderSourceDescription &source,
                    const std::string &source_text, std::string &diagnostics)
        -> Slang::ComPtr<slang::IModule> {

        const std::string module_name = source.path.stem().string();
        const std::string source_path = source.path.string();

        Slang::ComPtr<slang::IBlob> diagnostics_blob;

        Slang::ComPtr<slang::IModule> module;

        module = session->loadModuleFromSourceString(
            module_name.c_str(), source_path.c_str(), source_text.c_str(),
            diagnostics_blob.writeRef());

        appendSlangDiagnostics(diagnostics, diagnostics_blob.get());

        if (module == nullptr) {
            throwShaderCompilationFailure(
                {
                    .message = "Failed to load Slang shader module",
                    .operation = "Load Slang Shader Module",
                },
                diagnostics);
        }

        return module;
    }

    [[nodiscard]] auto findSlangEntryPoint(slang::IModule *module,
                                           const std::string &entry_point_name)
        -> Slang::ComPtr<slang::IEntryPoint> {

        Slang::ComPtr<slang::IEntryPoint> entry_point;

        const SlangResult result = module->findEntryPointByName(
            entry_point_name.c_str(), entry_point.writeRef());

        if (SLANG_FAILED(result)) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceCompilationFailed,
                "Failed to find Slang shader entry point",
                Core::Error::NativeError(
                    static_cast<int>(result),
                    "Slang shader entry-point lookup failed"),
                "Find Slang Shader Entry Point");
        }

        if (entry_point == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceCompilationFailed,
                "Slang compiler returned a null shader entry point",
                "Find Slang Shader Entry Point");
        }

        return entry_point;
    }

    [[nodiscard]] auto composeSlangProgram(slang::ISession *session,
                                           slang::IModule *module,
                                           slang::IEntryPoint *entry_point,
                                           std::string &diagnostics)
        -> Slang::ComPtr<slang::IComponentType> {

        std::array<slang::IComponentType *, 2> components{
            module,
            entry_point,
        };

        Slang::ComPtr<slang::IComponentType> composed_program;
        Slang::ComPtr<slang::IBlob> composition_diagnostics;

        const SlangResult result = session->createCompositeComponentType(
            components.data(), static_cast<SlangInt>(components.size()),
            composed_program.writeRef(), composition_diagnostics.writeRef());

        appendSlangDiagnostics(diagnostics, composition_diagnostics.get());

        if (SLANG_FAILED(result)) {
            throwSlangCompilationFailure(
                {
                    .message = "Failed to compose Slang shader program",
                    .native_description =
                        "Slang shader program composition failed",
                    .operation = "Compose Slang Shader Program",
                },
                result, diagnostics);
        }

        if (composed_program == nullptr) {
            throwShaderCompilationFailure(
                {
                    .message = "Failed to compose Slang shader program",
                    .operation = "Compose Slang Shader Program",
                },
                diagnostics);
        }

        return composed_program;
    }

    [[nodiscard]] auto linkSlangProgram(slang::IComponentType *composed_program,
                                        std::string &diagnostics)
        -> Slang::ComPtr<slang::IComponentType> {

        Slang::ComPtr<slang::IComponentType> linked_program;
        Slang::ComPtr<slang::IBlob> link_diagnostics;

        const SlangResult result = composed_program->link(
            linked_program.writeRef(), link_diagnostics.writeRef());

        appendSlangDiagnostics(diagnostics, link_diagnostics.get());

        if (SLANG_FAILED(result)) {
            throwSlangCompilationFailure(
                {
                    .message = "Failed to link Slang shader program",
                    .native_description = "Slang shader program linking failed",
                    .operation = "Link Slang Shader Program",
                },
                result, diagnostics);
        }

        if (linked_program == nullptr) {
            throwShaderCompilationFailure(
                {
                    .message = "Failed to link Slang shader program",
                    .operation = "Link Slang Shader Program",
                },
                diagnostics);
        }

        return linked_program;
    }

    [[nodiscard]] auto toSlangStage(Shader::ShaderStage stage) -> SlangStage {

        switch (stage) {
        case Shader::ShaderStage::Vertex:
            return SLANG_STAGE_VERTEX;

        case Shader::ShaderStage::Fragment:
            return SLANG_STAGE_FRAGMENT;

        case Shader::ShaderStage::Compute:
            return SLANG_STAGE_COMPUTE;

        default:
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceInvalid,
                "Unsupported shader execution stage", "Validate Shader Stage");
        }
    }

    auto validateSlangEntryPointStage(slang::IComponentType *linked_program,
                                      Shader::ShaderStage stage) -> void {

        slang::ProgramLayout *layout = linked_program->getLayout(0);

        if (layout == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceCompilationFailed,
                "Failed to retrieve Slang shader program reflection",
                "Reflect Slang Shader Program");
        }

        if (layout->getEntryPointCount() != 1) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceCompilationFailed,
                "Expected exactly one Slang shader entry point",
                "Validate Slang Shader Entry Points");
        }

        slang::EntryPointReflection *entry_layout =
            layout->getEntryPointByIndex(0);

        if (entry_layout == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceCompilationFailed,
                "Failed to retrieve Slang shader entry-point reflection",
                "Reflect Slang Shader Entry Point");
        }

        const SlangStage actual_stage = entry_layout->getStage();
        const SlangStage expected_stage = toSlangStage(stage);

        if (actual_stage != expected_stage) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceInvalid,
                "Requested shader stage does not match Slang "
                "entry-point stage",
                "Validate Shader Stage");
        }
    }

    [[nodiscard]] auto copySpirvArtifact(slang::IBlob *spirv_blob)
        -> Shader::ShaderArtifact {

        const std::size_t byte_count = spirv_blob->getBufferSize();
        const void *bytecode = spirv_blob->getBufferPointer();

        if (bytecode == nullptr || byte_count == 0 ||
            byte_count % sizeof(std::uint32_t) != 0) {
            throw Core::Error::EngineError(
                Core::Error::Code::ShaderSourceCompilationFailed,
                "Slang generated invalid SPIR-V bytecode",
                "Validate Generated SPIR-V");
        }

        const std::size_t word_count = byte_count / sizeof(std::uint32_t);

        Shader::ShaderArtifact artifact{};
        artifact.words.resize(word_count);

        std::memcpy(artifact.words.data(), bytecode, byte_count);

        return artifact;
    }

    [[nodiscard]] auto generateSlangSpirv(slang::IComponentType *linked_program,
                                          std::string &diagnostics)
        -> Shader::ShaderArtifact {

        Slang::ComPtr<slang::IBlob> spirv_blob;
        Slang::ComPtr<slang::IBlob> codegen_diagnostics;

        const SlangResult result = linked_program->getEntryPointCode(
            0, 0, spirv_blob.writeRef(), codegen_diagnostics.writeRef());

        appendSlangDiagnostics(diagnostics, codegen_diagnostics.get());

        if (SLANG_FAILED(result)) {
            throwSlangCompilationFailure(
                {
                    .message =
                        "Failed to generate Slang SPIR-V shader bytecode",
                    .native_description = "Slang SPIR-V code generation failed",
                    .operation = "Generate Slang SPIR-V",
                },
                result, diagnostics);
        }

        if (spirv_blob == nullptr) {
            throwShaderCompilationFailure(
                {
                    .message =
                        "Slang compiler returned a null SPIR-V bytecode blob",
                    .operation = "Generate Slang SPIR-V",
                },
                diagnostics);
        }

        return copySpirvArtifact(spirv_blob.get());
    }
} // namespace

namespace SNE::Engine::Renderer::Shader {

    auto compileShaderSource(const ShaderSourceDescription &source)
        -> ShaderCompilationResult {

        validateShaderSource(source);

        const std::string source_text = loadShaderSource(source.path);

        Slang::ComPtr<slang::IGlobalSession> global_session =
            createSlangGlobalSession();

        Slang::ComPtr<slang::ISession> session =
            createSlangSession(global_session.get());

        std::string diagnostics;

        Slang::ComPtr<slang::IModule> module =
            loadSlangModule(session.get(), source, source_text, diagnostics);

        Slang::ComPtr<slang::IEntryPoint> entry_point =
            findSlangEntryPoint(module.get(), source.entry_point);

        Slang::ComPtr<slang::IComponentType> composed_program =
            composeSlangProgram(session.get(), module.get(), entry_point.get(),
                                diagnostics);

        Slang::ComPtr<slang::IComponentType> linked_program =
            linkSlangProgram(composed_program.get(), diagnostics);

        validateSlangEntryPointStage(linked_program.get(), source.stage);

        ShaderArtifact artifact =
            generateSlangSpirv(linked_program.get(), diagnostics);

        return ShaderCompilationResult{
            .stage_input =
                {
                    .artifact = std::move(artifact),
                    .selection =
                        {
                            .stage = source.stage,
                            .entry_point = source.entry_point,
                        },
                },
            .diagnostics = std::move(diagnostics),
        };
    }
} // namespace SNE::Engine::Renderer::Shader
