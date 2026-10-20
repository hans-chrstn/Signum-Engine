#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/material/material_resource_binding_compatibility.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_type.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_updates.hpp"
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

            if (std::cmp_greater_equal(binding.set, descriptor_sets.size()) ||
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
                [&binding](
                    const VulkanDescriptorBindingDescription &description)
                    -> bool { return description.binding == binding.binding; });

            if (binding_it == layout_bindings.end()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer material binding is not declared in "
                    "the descriptor-set layout");
            }

            if (binding_it->descriptor_type !=
                    toVkDescriptorType(binding.type) ||
                binding.array_element >= binding_it->descriptor_count) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer material binding has an incompatible "
                    "descriptor type or array element");
            }

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptor_sets[binding.set];
            write.dstBinding = binding.binding;
            write.dstArrayElement = binding.array_element;
            write.descriptorCount = 1U;
            write.descriptorType = toVkDescriptorType(binding.type);

            switch (binding.type) {
            case Shader::ShaderResourceType::CombinedImageSampler: {
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
                image_info.sampler =
                    texture_resources->m_Sampler.nativeHandle();
                image_info.imageView =
                    texture_resources->m_ImageView.nativeHandle();
                image_info.imageLayout =
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                image_infos.push_back(image_info);

                write.pImageInfo = &image_infos.back();

                break;
            }
            case Shader::ShaderResourceType::UniformBuffer: {
                const VulkanBuffer *buffer =
                    m_BufferRegistry.find(binding.resource_handle);

                if (buffer == nullptr) {
                    Core::Assertion::failAssertion(
                        Core::Assertion::AssertionType::Precondition,
                        Core::Error::Subsystem::Vulkan,
                        "VulkanRenderer material descriptor update requires a "
                        "registered buffer handle");
                }

                if (buffer->size() > m_PhysicalDevice.capabilities.properties
                                         .limits.maxUniformBufferRange) {
                    Core::Assertion::failAssertion(
                        Core::Assertion::AssertionType::Precondition,
                        Core::Error::Subsystem::Vulkan,
                        "VulkanRenderer material uniform-buffer descriptor "
                        "range exceeds "
                        "the device's maximum uniform-buffer range");
                }

                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = buffer->nativeHandle();
                buffer_info.offset = 0U;
                buffer_info.range = buffer->size();

                buffer_infos.push_back(buffer_info);

                write.pBufferInfo = &buffer_infos.back();

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
        if (!m_MaterialDescriptorResources.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material descriptor updates require allocated "
                "descriptor sets");
        }

        writeMaterialDescriptorSets(m_MaterialDescriptorResources.value(),
                                    m_MaterialBindings.bindings);
    }

    auto VulkanRenderer::updateMaterialBuffer(std::uint32_t set,
                                              std::uint32_t binding,
                                              std::uint32_t array_element,
                                              std::span<const std::byte> data)
        -> void {

        if (!m_MaterialDescriptorResources.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::updateMaterialBuffer requires allocated "
                "material descriptor resources");
        }

        MaterialDescriptorResources &active_descriptor_resources =
            m_MaterialDescriptorResources.value();

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

        if (binding_it->type != Shader::ShaderResourceType::UniformBuffer) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::updateMaterialBuffer requires a "
                "uniform-buffer binding");
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
                "VulkanRenderer material uniform-buffer binding does not "
                "resolve to a registered buffer");
        }

        if (data.size_bytes() != old_buffer->size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::updateMaterialBuffer requires data matching "
                "the existing buffer size");
        }

        std::vector<MaterialResourceBinding> bindings =
            m_MaterialBindings.bindings;

        GpuResourceHandle buffer_handle = registerUniformBuffer(data);

        bindings[binding_index].resource_handle = buffer_handle;

        std::optional<MaterialDescriptorResources>
            new_material_descriptor_resources;

        try {
            new_material_descriptor_resources.emplace(
                createMaterialDescriptorResources(
                    m_MaterialDescriptorPoolSizes));

            writeMaterialDescriptorSets(
                new_material_descriptor_resources.value(), bindings);
        } catch (...) {
            new_material_descriptor_resources.reset();
            static_cast<void>(m_BufferRegistry.remove(buffer_handle));
            throw;
        }

        VulkanBuffer *old_buffer_to_retire =
            m_BufferRegistry.find(old_buffer_handle);

        if (old_buffer_to_retire == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active material buffer handle does not resolve "
                "to a registered buffer");
        }

        try {
            const std::uint64_t submission_id =
                m_GraphicsSubmissionState.lastSubmitted();

            m_GraphicsResourceRetirementQueue.retire(
                submission_id, *old_buffer_to_retire,
                active_descriptor_resources);
        } catch (...) {
            new_material_descriptor_resources.reset();
            static_cast<void>(m_BufferRegistry.remove(buffer_handle));
            throw;
        }

        static_cast<void>(m_BufferRegistry.remove(old_buffer_handle));

        m_MaterialDescriptorResources.emplace(
            std::move(new_material_descriptor_resources.value()));

        m_MaterialBindings.bindings[binding_index] = bindings[binding_index];
    }

    auto VulkanRenderer::registerUniformBuffer(std::span<const std::byte> data)
        -> GpuResourceHandle {
        if (data.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer uniform buffer registration requires nonempty "
                "data");
        }

        VulkanBufferCreateInfo buffer_create_info{
            .size = data.size_bytes(),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .memory_usage = GpuMemoryUsage::Upload,
        };

        VulkanBuffer buffer{m_MemoryAllocator, buffer_create_info};

        buffer.write(data);

        GpuResourceHandle buffer_handle =
            m_BufferRegistry.insert(std::move(buffer));

        return buffer_handle;
    }

    auto VulkanRenderer::registerMaterialBufferBinding(
        const MaterialBufferBindingInput &input) -> MaterialResourceBinding {
        if (input.type != Shader::ShaderResourceType::UniformBuffer) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer material buffer binding requires a uniform "
                "buffer resource type");
        }

        GpuResourceHandle buffer_handle = registerUniformBuffer(input.data);

        return MaterialResourceBinding{
            .set = input.set,
            .binding = input.binding,
            .array_element = input.array_element,
            .type = input.type,
            .resource_handle = buffer_handle,
        };
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

        if (!m_DescriptorSetLayouts.empty()) {
            m_MaterialDescriptorResources.emplace(
                createMaterialDescriptorResources(
                    m_MaterialDescriptorPoolSizes));
        }

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
