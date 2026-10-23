#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/material/material_buffer_update_frequency.hpp"
#include "engine/renderer/material/material_resource_binding.hpp"
#include "engine/renderer/material/material_resource_binding_compatibility.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_type.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_updates.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_reflection.hpp"
#include "vulkan_renderer.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    auto VulkanRenderer::createTextureBindingResources(
        const MaterialResourceBinding &resource_binding,
        const VulkanDescriptorSetLayout &layout) -> TextureBindingResources {
        if (resource_binding.set != 0U || resource_binding.binding != 0U ||
            resource_binding.array_element != 0U ||
            resource_binding.type !=
                Shader::ShaderResourceType::CombinedImageSampler) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::createTextureBindingResources requires a "
                "supported sampled-texture material binding");
        }

        const GpuResourceHandle &texture_handle =
            resource_binding.resource_handle;

        const TextureResources *texture_resources =
            m_TextureRegistry.find(texture_handle);

        if (texture_resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::createTextureBindingResources requires a "
                "registered texture handle");
        }

        VkDescriptorPoolSize descriptor_pool_size{};
        descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_pool_size.descriptorCount = 1U;

        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.maxSets = 1U;
        descriptor_pool_create_info.poolSizeCount = 1U;
        descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;

        VulkanDescriptorPool descriptor_pool{m_Device.nativeHandle(),
                                             descriptor_pool_create_info};

        std::array<VkDescriptorSetLayout, 1> layouts{
            layout.nativeHandle(),
        };

        std::vector<VkDescriptorSet> descriptor_sets =
            descriptor_pool.allocateDescriptorSets(layouts);

        VkDescriptorImageInfo descriptor_image_info{};
        descriptor_image_info.sampler =
            texture_resources->m_Sampler.nativeHandle();
        descriptor_image_info.imageView =
            texture_resources->m_ImageView.nativeHandle();
        descriptor_image_info.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet = descriptor_sets.front();
        write_descriptor_set.dstBinding = 0U;
        write_descriptor_set.dstArrayElement = 0U;
        write_descriptor_set.descriptorCount = 1U;
        write_descriptor_set.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set.pImageInfo = &descriptor_image_info;

        std::array<VkWriteDescriptorSet, 1> writes{
            write_descriptor_set,
        };

        updateDescriptorSets(m_Device.nativeHandle(), writes);

        return TextureBindingResources{
            .m_DescriptorPool = std::move(descriptor_pool),
            .m_DescriptorSet = descriptor_sets.front(),
            .m_TextureHandle = texture_handle,
        };
    }

    auto VulkanRenderer::validateMaterialDescriptorBinding(
        const MaterialResourceBinding &binding,
        std::size_t descriptor_set_count) const -> void {
        if (std::cmp_greater_equal(binding.set, descriptor_set_count) ||
            std::cmp_greater_equal(binding.set,
                                   m_DescriptorSetLayouts.size())) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material binding references an invalid "
                "descriptor-set index");
        }

        const std::span<const VulkanDescriptorBindingDescription>
            layout_bindings =
                m_DescriptorSetLayouts[binding.set].bindingDescriptions();

        const auto binding_it = std::ranges::find_if(
            layout_bindings,
            [&binding](const VulkanDescriptorBindingDescription &description)
                -> bool { return description.binding == binding.binding; });

        if (binding_it == layout_bindings.end()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material binding is not declared in "
                "the descriptor-set layout");
        }

        if (binding_it->descriptor_type != toVkDescriptorType(binding.type) ||
            binding.array_element >= binding_it->descriptor_count) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material binding has an incompatible "
                "descriptor type or array element");
        }
    }

    auto VulkanRenderer::writeMaterialTextureDescriptor(
        const MaterialResourceBinding &binding, VkWriteDescriptorSet &write,
        std::vector<VkDescriptorImageInfo> &image_infos) const -> void {

        const TextureResources *texture_resources =
            m_TextureRegistry.find(binding.resource_handle);

        if (texture_resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material descriptor update requires a "
                "registered texture handle");
        }

        VkDescriptorImageInfo image_info{};
        image_info.sampler = texture_resources->m_Sampler.nativeHandle();
        image_info.imageView = texture_resources->m_ImageView.nativeHandle();
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        image_infos.push_back(image_info);

        write.pImageInfo = &image_infos.back();
    }

    auto VulkanRenderer::writeMaterialBufferDescriptor(
        const MaterialResourceBinding &binding, VkWriteDescriptorSet &write,
        std::vector<VkDescriptorBufferInfo> &buffer_infos) const -> void {

        const VulkanBuffer *buffer =
            m_BufferRegistry.find(binding.resource_handle);

        if (buffer == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material buffer descriptor update requires a "
                "registered buffer handle");
        }

        switch (binding.type) {
        case Shader::ShaderResourceType::StorageBuffer: {
            if (buffer->size() > m_PhysicalDevice.capabilities.properties.limits
                                     .maxStorageBufferRange) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer material storage-buffer descriptor range "
                    "exceeds the device's maximum storage-buffer range");
            }

            break;
        }
        case Shader::ShaderResourceType::UniformBuffer: {
            if (buffer->size() > m_PhysicalDevice.capabilities.properties.limits
                                     .maxUniformBufferRange) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer material uniform-buffer descriptor range "
                    "exceeds the device's maximum uniform-buffer range");
            }

            break;
        }
        default:
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material buffer descriptor update does not "
                "support the requested resource type");
        }

        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = buffer->nativeHandle();
        buffer_info.offset = 0U;
        buffer_info.range = buffer->size();

        buffer_infos.push_back(buffer_info);

        write.pBufferInfo = &buffer_infos.back();
    }

    auto VulkanRenderer::writeMaterialDescriptorSets(
        MaterialDescriptorResources &destination,
        std::span<const MaterialResourceBinding> bindings) -> void {

        const std::vector<VkDescriptorSet> &descriptor_sets =
            destination.m_DescriptorSets;

        std::vector<VkDescriptorImageInfo> image_infos;
        image_infos.reserve(bindings.size());

        std::vector<VkDescriptorBufferInfo> buffer_infos;
        buffer_infos.reserve(bindings.size());

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(bindings.size());

        for (const MaterialResourceBinding &binding : bindings) {
            validateMaterialDescriptorBinding(binding, descriptor_sets.size());
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptor_sets[binding.set];
            write.dstBinding = binding.binding;
            write.dstArrayElement = binding.array_element;
            write.descriptorCount = 1U;
            write.descriptorType = toVkDescriptorType(binding.type);

            switch (binding.type) {
            case Shader::ShaderResourceType::CombinedImageSampler: {
                writeMaterialTextureDescriptor(binding, write, image_infos);
                break;
            }
            case Shader::ShaderResourceType::StorageBuffer:
            case Shader::ShaderResourceType::UniformBuffer: {
                writeMaterialBufferDescriptor(binding, write, buffer_infos);
                break;
            }
            default:
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer material descriptor update does not "
                    "support "
                    "the requested resource type");
            }

            writes.push_back(write);
        }

        updateDescriptorSets(m_Device.nativeHandle(), writes);
    }

    auto VulkanRenderer::updateMaterialDescriptorSets() -> void {

        MaterialDescriptorResources *active_resources =
            findActiveMaterialDescriptorResources();

        if (active_resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material descriptor updates require allocated "
                "descriptor sets");
        }

        writeMaterialDescriptorSets(*active_resources,
                                    m_MaterialBindings.bindings);
    }

    auto VulkanRenderer::findActiveMaterialDescriptorResources() noexcept
        -> MaterialDescriptorResources * {
        if (!m_ActiveMaterialDescriptorHandle.has_value()) {
            return nullptr;
        }

        MaterialDescriptorResources *resources =
            m_MaterialDescriptorRegistry.find(
                *m_ActiveMaterialDescriptorHandle);

        if (resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active material descriptor handle does not "
                "resolve to registered resources");
        }

        return resources;
    }

    auto VulkanRenderer::findActiveMaterialDescriptorResources() const noexcept
        -> const MaterialDescriptorResources * {
        if (!m_ActiveMaterialDescriptorHandle.has_value()) {
            return nullptr;
        }

        const MaterialDescriptorResources *resources =
            m_MaterialDescriptorRegistry.find(
                *m_ActiveMaterialDescriptorHandle);

        if (resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active material descriptor handle does not "
                "resolve to registered resources");
        }

        return resources;
    }

    auto VulkanRenderer::findMaterialDescriptorResourcesForFrame(
        std::size_t frame_index) const noexcept
        -> const MaterialDescriptorResources * {
        if (frame_index >= m_FrameResources.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material descriptor selection requires a valid "
                "frame index");
        }

        if (m_FrameMaterialDescriptorHandles.empty()) {
            return findActiveMaterialDescriptorResources();
        }

        if (m_FrameMaterialDescriptorHandles.size() !=
            m_FrameResources.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires one material descriptor handle per "
                "frame slot");
        }

        const MaterialDescriptorResources *resources =
            m_MaterialDescriptorRegistry.find(
                m_FrameMaterialDescriptorHandles[frame_index]);

        if (resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frame material descriptor handle does not "
                "resolve to registered resources");
        }

        return resources;
    }

    auto VulkanRenderer::collectActiveMaterialDescriptorHandles() const
        -> std::vector<GpuResourceHandle> {

        std::vector<GpuResourceHandle> handles;

        if (!m_FrameMaterialDescriptorHandles.empty()) {
            if (m_FrameMaterialDescriptorHandles.size() !=
                m_FrameResources.size()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer requires one material descriptor handle "
                    "per frame slot");
            }

            handles = m_FrameMaterialDescriptorHandles;
        } else {
            if (!m_ActiveMaterialDescriptorHandle.has_value()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer::updateMaterialBuffer requires allocated "
                    "material descriptor resources");
            }

            handles.push_back(m_ActiveMaterialDescriptorHandle.value());
        }

        for (const GpuResourceHandle &handle : handles) {
            const MaterialDescriptorResources *resources =
                m_MaterialDescriptorRegistry.find(handle);

            if (resources == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer previous material descriptor handle does "
                    "not resolve to registered resources");
            }
        }

        return handles;
    }

    auto VulkanRenderer::validateMaterialBufferReplacement(
        std::uint32_t set, std::uint32_t binding, std::uint32_t array_element,
        std::span<const std::byte> data) const -> std::size_t {

        if (data.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::updateMaterialBuffer requires nonempty data");
        }

        const auto binding_it = std::ranges::find_if(
            m_MaterialBindings.bindings,
            [set, binding,
             array_element](const MaterialResourceBinding &candidate) -> bool {
                return candidate.set == set && candidate.binding == binding &&
                       candidate.array_element == array_element;
            });

        if (binding_it == m_MaterialBindings.bindings.end()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::updateMaterialBuffer requires an existing "
                "material binding");
        }

        if (binding_it->type != Shader::ShaderResourceType::UniformBuffer &&
            binding_it->type != Shader::ShaderResourceType::StorageBuffer) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::updateMaterialBuffer requires a "
                "uniform-buffer or storage-buffer binding");
        }

        const std::size_t binding_index = static_cast<std::size_t>(
            binding_it - m_MaterialBindings.bindings.begin());

        const GpuResourceHandle old_buffer_handle = binding_it->resource_handle;

        const VulkanBuffer *old_buffer =
            m_BufferRegistry.find(old_buffer_handle);

        if (old_buffer == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material buffer binding does not "
                "resolve to a registered buffer");
        }

        if (data.size_bytes() != old_buffer->size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::updateMaterialBuffer requires data matching "
                "the existing buffer size");
        }

        return binding_index;
    }

    auto VulkanRenderer::retirePreviousMaterialResources(
        GpuResourceHandle old_buffer_handle,
        std::span<const GpuResourceHandle> previous_descriptor_handles,
        bool retire_old_buffer) -> void {
        VulkanBuffer *old_buffer_to_retire =
            m_BufferRegistry.find(old_buffer_handle);

        if (old_buffer_to_retire == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active material buffer handle does not resolve "
                "to a registered buffer");
        }

        const std::uint64_t submission_id =
            m_GraphicsSubmissionState.lastSubmitted();

        std::vector<MaterialDescriptorResources *>
            descriptor_resources_to_retire;

        descriptor_resources_to_retire.reserve(
            previous_descriptor_handles.size());

        for (const GpuResourceHandle &handle : previous_descriptor_handles) {
            MaterialDescriptorResources *resources =
                m_MaterialDescriptorRegistry.find(handle);

            if (resources == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer previous material descriptor handle "
                    "does not resolve to registered resources during "
                    "retirement");
            }

            descriptor_resources_to_retire.push_back(resources);
        }

        if (retire_old_buffer) {
            m_GraphicsResourceRetirementQueue
                .retireMany<VulkanBuffer, MaterialDescriptorResources>(
                    submission_id, *old_buffer_to_retire,
                    std::span<MaterialDescriptorResources *const>{
                        descriptor_resources_to_retire});
        } else {
            m_GraphicsResourceRetirementQueue
                .retireMany<MaterialDescriptorResources>(
                    submission_id,
                    std::span<MaterialDescriptorResources *const>{
                        descriptor_resources_to_retire});
        }
    }

    auto VulkanRenderer::updateMaterialBuffer(std::uint32_t set,
                                              std::uint32_t binding,
                                              std::uint32_t array_element,
                                              std::span<const std::byte> data)
        -> void {

        const auto buffers_it = std::ranges::find_if(
            m_FrequentMaterialBuffers,
            [set, binding,
             array_element](const FrequentMaterialBufferState &state) -> bool {
                return state.m_Set == set && state.m_Binding == binding &&
                       state.m_ArrayElement == array_element;
            });

        if (buffers_it != m_FrequentMaterialBuffers.end()) {
            stageFrequentMaterialBufferUpdate(*buffers_it, data);
            return;
        }

        const std::vector<GpuResourceHandle> previous_descriptor_handles =
            collectActiveMaterialDescriptorHandles();

        const std::size_t binding_index = validateMaterialBufferReplacement(
            set, binding, array_element, data);

        const MaterialResourceBinding &existing_binding =
            m_MaterialBindings.bindings[binding_index];

        const GpuResourceHandle old_buffer_handle =
            existing_binding.resource_handle;

        std::vector<MaterialResourceBinding> bindings =
            m_MaterialBindings.bindings;

        GpuResourceHandle buffer_handle =
            registerMaterialBuffer(data, existing_binding.type);

        bindings[binding_index].resource_handle = buffer_handle;

        const bool old_buffer_still_referenced = std::ranges::any_of(
            bindings,
            [old_buffer_handle](
                const MaterialResourceBinding &candidate) -> bool {
                return old_buffer_handle == candidate.resource_handle;
            });

        std::vector<GpuResourceHandle> replacement_descriptor_handles;

        try {
            replacement_descriptor_handles.reserve(
                previous_descriptor_handles.size());

            if (m_FrameMaterialDescriptorHandles.empty()) {
                replacement_descriptor_handles.push_back(
                    registerMaterialDescriptorResources(bindings));
            } else {
                replacement_descriptor_handles =
                    registerFrameMaterialDescriptorResources(bindings);
            }
        } catch (...) {
            static_cast<void>(m_BufferRegistry.remove(buffer_handle));
            throw;
        }

        const bool retire_old_buffer =
            !old_buffer_still_referenced &&
            !m_ResourceDependencies.hasDependentsOutside(
                old_buffer_handle, previous_descriptor_handles);

        try {
            retirePreviousMaterialResources(old_buffer_handle,
                                            previous_descriptor_handles,
                                            retire_old_buffer);
        } catch (...) {
            rollbackRegisteredFrameMaterialDescriptorResources(
                replacement_descriptor_handles);

            static_cast<void>(m_BufferRegistry.remove(buffer_handle));
            throw;
        }

        for (const GpuResourceHandle &handle : previous_descriptor_handles) {
            static_cast<void>(
                m_ResourceDependencies.removeRelationshipsInvolving(handle));

            static_cast<void>(m_MaterialDescriptorRegistry.remove(handle));
        }

        if (retire_old_buffer) {
            static_cast<void>(m_BufferRegistry.remove(old_buffer_handle));
        }

        if (m_FrameMaterialDescriptorHandles.empty()) {
            m_ActiveMaterialDescriptorHandle.emplace(
                replacement_descriptor_handles.front());
        } else {
            m_FrameMaterialDescriptorHandles =
                std::move(replacement_descriptor_handles);
        }

        m_MaterialBindings.bindings[binding_index] = bindings[binding_index];
    }

    auto VulkanRenderer::registerMaterialBuffer(std::span<const std::byte> data,
                                                Shader::ShaderResourceType type)
        -> GpuResourceHandle {
        if (data.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material buffer registration requires nonempty "
                "data");
        }

        VulkanBufferCreateInfo buffer_create_info{
            .size = data.size_bytes(),
            .memory_usage = GpuMemoryUsage::Upload,
        };

        switch (type) {
        case Shader::ShaderResourceType::UniformBuffer:
            buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case Shader::ShaderResourceType::StorageBuffer:
            buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        default:
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material buffer registration requires a "
                "uniform or storage buffer resource type");
        }

        VulkanBuffer buffer{m_MemoryAllocator, buffer_create_info};

        buffer.write(data);

        GpuResourceHandle buffer_handle =
            m_BufferRegistry.insert(std::move(buffer));

        return buffer_handle;
    }

    auto VulkanRenderer::createFrequentMaterialBufferState(
        const MaterialBufferBindingInput &input)
        -> FrequentMaterialBufferState {
        if (input.update_frequency != MaterialBufferUpdateFrequency::Frequent) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frequent material buffer creation requires "
                "Frequent update frequency");
        }

        if (input.type != Shader::ShaderResourceType::UniformBuffer &&
            input.type != Shader::ShaderResourceType::StorageBuffer) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frequent material buffer creation requires a "
                "uniform or storage buffer");
        }

        if (input.data.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frequent material buffer creation requires "
                "nonempty data");
        }

        if (m_FrameResources.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frequent material buffer creation requires "
                "initialized frame resources");
        }

        FrequentMaterialBufferState state{};
        state.m_Set = input.set;
        state.m_Binding = input.binding;
        state.m_ArrayElement = input.array_element;
        state.m_Type = input.type;
        state.m_LatestData.assign(input.data.begin(), input.data.end());
        state.m_Frames.resize(m_FrameResources.size());

        std::size_t registered_count{};
        try {
            for (FrequentMaterialBufferState::FrameState &frame :
                 state.m_Frames) {
                frame.m_BufferHandle =
                    registerMaterialBuffer(state.m_LatestData, state.m_Type);

                ++registered_count;
            }
        } catch (...) {
            for (std::size_t i{}; i < registered_count; ++i) {
                static_cast<void>(
                    m_BufferRegistry.remove(state.m_Frames[i].m_BufferHandle));
            }
            throw;
        }
        return state;
    }

    auto VulkanRenderer::rollbackFrequentMaterialBufferState(
        const FrequentMaterialBufferState &state) -> void {
        for (const FrequentMaterialBufferState::FrameState &frame_state :
             state.m_Frames) {
            static_cast<void>(
                m_BufferRegistry.remove(frame_state.m_BufferHandle));
        }
    }

    auto VulkanRenderer::stageFrequentMaterialBufferUpdate(
        FrequentMaterialBufferState &state, std::span<const std::byte> data)
        -> void {
        if (data.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frequent material buffer update requires "
                "nonempty data");
        }

        if (data.size_bytes() != state.m_LatestData.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frequent material buffer update requires "
                "matching data size");
        }

        std::vector<std::byte> replacement_data;
        replacement_data.assign(data.begin(), data.end());

        state.m_LatestData.swap(replacement_data);

        for (FrequentMaterialBufferState::FrameState &frame_state :
             state.m_Frames) {
            frame_state.m_NeedsUpload = true;
        }
    }

    auto VulkanRenderer::uploadFrequentMaterialBuffersForFrame(
        std::size_t frame_index) -> void {
        if (frame_index >= m_FrameResources.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frequent material buffer upload requires a "
                "valid frame index");
        }

        for (FrequentMaterialBufferState &state : m_FrequentMaterialBuffers) {
            if (frame_index >= state.m_Frames.size()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer frequent material buffer state has an "
                    "invalid frame index");
            }

            FrequentMaterialBufferState::FrameState &frame_state =
                state.m_Frames[frame_index];

            if (!frame_state.m_NeedsUpload) {
                continue;
            }

            VulkanBuffer *buffer =
                m_BufferRegistry.find(frame_state.m_BufferHandle);

            if (buffer == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer frequent material buffer handle does not "
                    "resolve to a registered buffer");
            }

            if (buffer->size() != state.m_LatestData.size()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer frequent material buffer upload requires "
                    "matching buffer size");
            }

            buffer->write(std::span<const std::byte>{state.m_LatestData});

            frame_state.m_NeedsUpload = false;
        }
    }

    auto VulkanRenderer::registerMaterialBufferBinding(
        const MaterialBufferBindingInput &input) -> MaterialResourceBinding {
        if (input.type != Shader::ShaderResourceType::UniformBuffer &&
            input.type != Shader::ShaderResourceType::StorageBuffer) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material buffer binding requires a "
                "uniform or storage buffer resource type");
        }

        if (input.update_frequency == MaterialBufferUpdateFrequency::Frequent) {
            return registerFrequentMaterialBufferBinding(input);
        }

        GpuResourceHandle buffer_handle =
            registerMaterialBuffer(input.data, input.type);

        return MaterialResourceBinding{
            .set = input.set,
            .binding = input.binding,
            .array_element = input.array_element,
            .type = input.type,
            .resource_handle = buffer_handle,
        };
    }

    auto VulkanRenderer::registerFrequentMaterialBufferBinding(
        const MaterialBufferBindingInput &input) -> MaterialResourceBinding {
        FrequentMaterialBufferState state =
            createFrequentMaterialBufferState(input);

        MaterialResourceBinding binding{
            .set = input.set,
            .binding = input.binding,
            .array_element = input.array_element,
            .type = input.type,
            .resource_handle = state.m_Frames.front().m_BufferHandle,
        };

        try {
            m_FrequentMaterialBuffers.push_back(std::move(state));
        } catch (...) {
            rollbackFrequentMaterialBufferState(state);
            throw;
        }

        return binding;
    }

    auto VulkanRenderer::createMaterialDescriptorResources(
        std::span<const VkDescriptorPoolSize> pool_sizes)
        -> MaterialDescriptorResources {
        const std::optional<std::uint32_t> layout_count =
            Core::Numeric::tryConvertToUint32(m_DescriptorSetLayouts.size());

        if (!layout_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material descriptor-set count exceeds "
                "uint32_t");
        }

        const std::optional<std::uint32_t> pool_count =
            Core::Numeric::tryConvertToUint32(pool_sizes.size());

        if (!pool_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material descriptor-pool size count "
                "exceeds uint32_t");
        }

        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.maxSets = layout_count.value();
        pool_create_info.poolSizeCount = pool_count.value();
        pool_create_info.pPoolSizes = pool_sizes.data();

        VulkanDescriptorPool descriptor_pool{
            m_Device.nativeHandle(),
            pool_create_info,
        };

        std::vector<VkDescriptorSetLayout> layout_handles;
        layout_handles.reserve(m_DescriptorSetLayouts.size());
        for (const VulkanDescriptorSetLayout &layout : m_DescriptorSetLayouts) {
            layout_handles.push_back(layout.nativeHandle());
        }

        std::vector<VkDescriptorSet> descriptor_sets =
            descriptor_pool.allocateDescriptorSets(layout_handles);

        return MaterialDescriptorResources{
            .m_DescriptorPool = std::move(descriptor_pool),
            .m_DescriptorSets = std::move(descriptor_sets),
        };
    }

    auto VulkanRenderer::buildFrameMaterialBindings(
        std::size_t frame_index,
        std::span<const MaterialResourceBinding> base_bindings) const
        -> std::vector<MaterialResourceBinding> {
        if (frame_index >= m_FrameResources.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frame material bindings require a valid frame "
                "index");
        }

        std::vector<MaterialResourceBinding> bindings{base_bindings.begin(),
                                                      base_bindings.end()};

        for (const FrequentMaterialBufferState &state :
             m_FrequentMaterialBuffers) {
            if (frame_index >= state.m_Frames.size()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer frequent material buffer state has an "
                    "invalid frame index");
            }

            const auto binding_it = std::ranges::find_if(
                bindings,
                [&state](const MaterialResourceBinding &binding) -> bool {
                    return binding.set == state.m_Set &&
                           binding.binding == state.m_Binding &&
                           binding.array_element == state.m_ArrayElement;
                });

            if (binding_it == bindings.end()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer frequent material buffer binding was not "
                    "found");
            }

            const GpuResourceHandle &buffer_handle =
                state.m_Frames[frame_index].m_BufferHandle;

            const VulkanBuffer *buffer = m_BufferRegistry.find(buffer_handle);

            if (buffer == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer frequent material buffer handle does not "
                    "resolve to a registered buffer");
            }

            binding_it->resource_handle = buffer_handle;
        }

        return bindings;
    }

    auto VulkanRenderer::registerMaterialDescriptorResources(
        std::span<const MaterialResourceBinding> bindings)
        -> GpuResourceHandle {

        MaterialDescriptorResources resources =
            createMaterialDescriptorResources(m_MaterialDescriptorPoolSizes);

        writeMaterialDescriptorSets(resources, bindings);

        const GpuResourceHandle descriptor_handle =
            m_MaterialDescriptorRegistry.insert(std::move(resources));

        try {
            for (const MaterialResourceBinding &binding : bindings) {
                const GpuResourceDependency relationship{
                    .dependent = descriptor_handle,
                    .dependency = binding.resource_handle,
                };

                const bool relationship_exists =
                    m_ResourceDependencies.contains(relationship);

                if (!relationship_exists) {
                    m_ResourceDependencies.addDependency(relationship);
                }
            }
        } catch (...) {
            static_cast<void>(
                m_ResourceDependencies.removeRelationshipsInvolving(
                    descriptor_handle));

            static_cast<void>(
                m_MaterialDescriptorRegistry.remove(descriptor_handle));
            throw;
        }

        return descriptor_handle;
    }

    auto VulkanRenderer::registerFrameMaterialDescriptorResources(
        std::span<const MaterialResourceBinding> base_bindings)
        -> std::vector<GpuResourceHandle> {
        if (m_FrameResources.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer frame material descriptor registration "
                "requires initialized frame resources");
        }

        std::vector<GpuResourceHandle> handles;

        handles.reserve(m_FrameResources.size());

        try {
            for (std::size_t i{}; i < m_FrameResources.size(); ++i) {
                const std::vector<MaterialResourceBinding> bindings =
                    buildFrameMaterialBindings(i, base_bindings);

                GpuResourceHandle handle =
                    registerMaterialDescriptorResources(bindings);

                handles.push_back(handle);
            }

        } catch (...) {
            rollbackRegisteredFrameMaterialDescriptorResources(handles);
            throw;
        }

        return handles;
    }

    auto VulkanRenderer::rollbackRegisteredMaterialDescriptorResources(
        GpuResourceHandle descriptor_handle) -> void {
        MaterialDescriptorResources *resources =
            m_MaterialDescriptorRegistry.find(descriptor_handle);

        if (resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer descriptor rollback requires a registered "
                "material descriptor handle");
        }

        static_cast<void>(m_ResourceDependencies.removeRelationshipsInvolving(
            descriptor_handle));

        static_cast<void>(
            m_MaterialDescriptorRegistry.remove(descriptor_handle));
    }

    auto VulkanRenderer::rollbackRegisteredFrameMaterialDescriptorResources(
        std::span<const GpuResourceHandle> descriptor_handles) -> void {
        for (const GpuResourceHandle &handle : descriptor_handles) {
            rollbackRegisteredMaterialDescriptorResources(handle);
        }
    }

    auto VulkanRenderer::initializeInitialMaterial(
        Shader::GraphicsShaderData shaders)
        -> ReflectedGraphicsShaderInterfaces {
        m_Material.emplace(MaterialDescription{
            .shaders = std::move(shaders),
            .pipeline_state = {},
        });

        const Shader::ShaderStageInput &vertex = m_Material->shaders.vertex;
        const Shader::ShaderStageInput &fragment = m_Material->shaders.fragment;

        const ReflectedGraphicsShaderInterfaces reflected_interfaces{
            .vertex =
                reflectSpirvInterface(vertex.artifact.words, vertex.selection),
            .fragment = reflectSpirvInterface(fragment.artifact.words,
                                              fragment.selection),
        };

        const std::array<Shader::ShaderInterface, 2> interfaces{
            reflected_interfaces.vertex,
            reflected_interfaces.fragment,
        };

        const std::vector<VulkanDescriptorSetLayoutRequirements> requirements =
            mergeVkDescriptorSetLayoutRequirements(interfaces);

        m_DescriptorSetLayouts = createVulkanDescriptorSetLayouts(
            m_Device.nativeHandle(), requirements,
            m_PhysicalDevice.capabilities.properties.limits
                .maxBoundDescriptorSets);

        m_MaterialDescriptorPoolSizes =
            calculateVkDescriptorPoolSizes(requirements);

        return reflected_interfaces;
    }

    auto VulkanRenderer::validateInitialMaterialBindings(
        const ReflectedGraphicsShaderInterfaces &interfaces) const -> void {
        const bool fragment_material_compatible =
            areMaterialResourceBindingsCompatible(
                m_MaterialBindings.bindings,
                interfaces.fragment.descriptor_bindings);

        if (!fragment_material_compatible) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer initial material bindings do not satisfy "
                "fragment shader descriptor requirements");
        }

        const bool vertex_material_compatible =
            areMaterialResourceBindingsCompatible(
                m_MaterialBindings.bindings,
                interfaces.vertex.descriptor_bindings);

        if (!vertex_material_compatible) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer initial material bindings do not satisfy "
                "vertex shader descriptor requirements");
        }
    }
} // namespace SNE::Engine::Renderer::Vulkan
