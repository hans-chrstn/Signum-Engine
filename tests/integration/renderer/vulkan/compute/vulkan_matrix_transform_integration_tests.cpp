
#include "../vulkan_integration_test_fixture.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/core/math/vector4.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_readback.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_shader_module.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <numbers>
#include <span>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Math = SNE::Engine::Core::Math;
namespace Renderer = SNE::Engine::Renderer;

namespace {
    struct ComputeTestResources {
        VkDevice device{VK_NULL_HANDLE};
        VkDescriptorSetLayout descriptor_set_layout{VK_NULL_HANDLE};
        VkDescriptorPool descriptor_pool{VK_NULL_HANDLE};
        VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
        VkPipeline pipeline{VK_NULL_HANDLE};

        explicit ComputeTestResources(VkDevice logical_device)
            : device(logical_device) {}

        ComputeTestResources(const ComputeTestResources &) = delete;
        auto operator=(const ComputeTestResources &)
            -> ComputeTestResources & = delete;

        ~ComputeTestResources() noexcept {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, pipeline, nullptr);
            }

            if (pipeline_layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
            }

            if (descriptor_pool != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
            }

            if (descriptor_set_layout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device, descriptor_set_layout,
                                             nullptr);
            }
        }
    };
} // namespace

TEST_F(VulkanIntegrationTest, CpuMatrixMatchesComputeShader) {
    constexpr std::uint32_t kOutputBinding = 0U;
    constexpr std::uint32_t kDescriptorCount = 1U;
    constexpr std::uint32_t kDescriptorSetCount = 1U;
    constexpr std::uint32_t kDescriptorSetIndex = 0U;
    constexpr std::uint32_t kPushConstantOffset = 0U;
    constexpr std::uint32_t kDispatchCount = 1U;
    constexpr std::size_t kOutputElementCount = 1U;
    constexpr std::size_t kVectorComponentCount = 4U;

    constexpr float kRightAngleRadians = std::numbers::pi_v<float> / 2.0F;
    constexpr float kTolerance = 1.0e-5F;

    constexpr Math::Vector3f kTranslation{
        .x = 5.0F,
        .y = -3.0F,
        .z = 0.0F,
    };

    constexpr Math::Vector3f kRotation{
        .x = 0.0F,
        .y = 0.0F,
        .z = kRightAngleRadians,
    };

    constexpr Math::Vector3f kScale{
        .x = 2.0F,
        .y = 3.0F,
        .z = 1.0F,
    };

    constexpr Math::Vector4f kExpected{
        .x = -1.0F,
        .y = -1.0F,
        .z = 0.0F,
        .w = 1.0F,
    };

    static_assert(std::is_trivially_copyable_v<Math::Vector4f>);
    static_assert(sizeof(Math::Vector4f) ==
                  kVectorComponentCount * sizeof(float));

    constexpr VkDeviceSize kOutputBufferSize = sizeof(Math::Vector4f);

    const std::vector<VkQueueFamilyProperties> queue_families =
        Vulkan::queryQueueFamilyProperties(m_PhysicalDevice);

    const std::uint32_t family_index = m_SelectedQueueFamily.family_index;

    ASSERT_LT(family_index, queue_families.size());

    if ((queue_families[family_index].queueFlags & VK_QUEUE_COMPUTE_BIT) ==
        0U) {
        GTEST_SKIP() << "Selected queue family does not support compute";
    }

    const Vulkan::VulkanBufferCreateInfo output_info{
        .size = kOutputBufferSize,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memory_usage = Renderer::GpuMemoryUsage::Device,
    };

    Vulkan::VulkanBuffer output_buffer{memoryAllocator(), output_info};

    ASSERT_NE(output_buffer.nativeHandle(), VK_NULL_HANDLE);

    ComputeTestResources resources{m_Device};

    VkDescriptorSetLayoutBinding output_binding{};
    output_binding.binding = kOutputBinding;
    output_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    output_binding.descriptorCount = kDescriptorCount;
    output_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo descriptor_layout_info{};
    descriptor_layout_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout_info.bindingCount = kDescriptorCount;
    descriptor_layout_info.pBindings = &output_binding;

    ASSERT_EQ(vkCreateDescriptorSetLayout(m_Device, &descriptor_layout_info,
                                          nullptr,
                                          &resources.descriptor_set_layout),
              VK_SUCCESS);

    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_size.descriptorCount = kDescriptorCount;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = kDescriptorCount;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = kDescriptorSetCount;

    ASSERT_EQ(vkCreateDescriptorPool(m_Device, &pool_info, nullptr,
                                     &resources.descriptor_pool),
              VK_SUCCESS);

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = resources.descriptor_pool;
    allocate_info.descriptorSetCount = kDescriptorSetCount;
    allocate_info.pSetLayouts = &resources.descriptor_set_layout;

    VkDescriptorSet descriptor_set{VK_NULL_HANDLE};

    ASSERT_EQ(
        vkAllocateDescriptorSets(m_Device, &allocate_info, &descriptor_set),
        VK_SUCCESS);

    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = output_buffer.nativeHandle();
    buffer_info.offset = 0U;
    buffer_info.range = kOutputBufferSize;

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = kOutputBinding;
    descriptor_write.descriptorCount = kDescriptorCount;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(m_Device, kDescriptorCount, &descriptor_write, 0U,
                           nullptr);

    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant_range.offset = kPushConstantOffset;
    push_constant_range.size =
        static_cast<std::uint32_t>(sizeof(Math::Matrix4f));

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = kDescriptorSetCount;
    pipeline_layout_info.pSetLayouts = &resources.descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 1U;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    ASSERT_EQ(vkCreatePipelineLayout(m_Device, &pipeline_layout_info, nullptr,
                                     &resources.pipeline_layout),
              VK_SUCCESS);

    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
        "matrix_transform.comp.spv";

    const std::vector<std::uint32_t> spirv = Vulkan::loadSpirv(shader_path);

    const Vulkan::VulkanShaderModule shader_module{m_Device, spirv};

    VkPipelineShaderStageCreateInfo shader_stage{};
    shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_stage.module = shader_module.nativeHandle();
    shader_stage.pName = "main";

    VkComputePipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage = shader_stage;
    pipeline_info.layout = resources.pipeline_layout;

    ASSERT_EQ(vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1U,
                                       &pipeline_info, nullptr,
                                       &resources.pipeline),
              VK_SUCCESS);

    const Math::Matrix4f model =
        Math::composeTRS(kTranslation, kRotation, kScale);

    immediateSubmission().execute([&](VkCommandBuffer command_buffer) -> void {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          resources.pipeline);

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                resources.pipeline_layout, kDescriptorSetIndex,
                                kDescriptorSetCount, &descriptor_set, 0U,
                                nullptr);

        vkCmdPushConstants(command_buffer, resources.pipeline_layout,
                           VK_SHADER_STAGE_COMPUTE_BIT, kPushConstantOffset,
                           static_cast<std::uint32_t>(sizeof(model)), &model);

        vkCmdDispatch(command_buffer, kDispatchCount, kDispatchCount,
                      kDispatchCount);

        VkBufferMemoryBarrier2 buffer_barrier{};
        buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        buffer_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        buffer_barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        buffer_barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        buffer_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        buffer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buffer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buffer_barrier.buffer = output_buffer.nativeHandle();
        buffer_barrier.offset = 0U;
        buffer_barrier.size = kOutputBufferSize;

        VkDependencyInfo dependency_info{};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.bufferMemoryBarrierCount = 1U;
        dependency_info.pBufferMemoryBarriers = &buffer_barrier;

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);
    });

    Math::Vector4f gpu_result{};

    const auto output_bytes = std::as_writable_bytes(
        std::span<Math::Vector4f>{&gpu_result, kOutputElementCount});

    Vulkan::readBufferData(memoryAllocator(), immediateSubmission(),
                           output_buffer, output_bytes);

    EXPECT_NEAR(gpu_result.x, kExpected.x, kTolerance);
    EXPECT_NEAR(gpu_result.y, kExpected.y, kTolerance);
    EXPECT_NEAR(gpu_result.z, kExpected.z, kTolerance);
    EXPECT_NEAR(gpu_result.w, kExpected.w, kTolerance);
}
