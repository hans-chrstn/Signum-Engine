#pragma once

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes device support and enablement for a renderer capability.
     *
     * Separates functionality supported by the selected physical device
     * from functionality enabled during logical-device creation.
     *
     * A supported capability is not necessarily enabled. An enabled
     * capability must also be supported by the selected device.
     *
     * This state describes low-level device functionality rather than
     * the availability of a complete engine rendering feature.
     */
    struct RendererCapabilityState {
        /**
         * @brief Whether the selected device supports the capability.
         *
         * Support includes the hardware features and API extensions
         * required by the backend to expose the capability.
         */
        bool device_supported{false};

        /**
         * @brief Whether the capability is enabled on the active device.
         *
         * Represents functionality selected during device initialization,
         * including required API extensions where applicable.
         *
         * An enabled capability is not necessarily used by the active
         * rendering path.
         */
        bool device_enabled{false};
    };

    /**
     * @brief Provides a backend-independent snapshot of renderer capabilities.
     *
     * Describes optional device functionality discovered and negotiated
     * during renderer initialization.
     *
     * Does not contain native graphics handles, allocate resources, or
     * indicate whether higher-level rendering effects are implemented.
     *
     * Unsupported capabilities remain disabled without preventing
     * unrelated rendering functionality from operating.
     *
     * @note Capability support and enablement are fixed for the
     * lifetime of the currently initialized rendering device.
     */
    struct RendererCapabilities {
        /** @brief Acceleration-structure support and enablement. */
        RendererCapabilityState acceleration_structures;

        /** @brief Hardware ray-tracing pipeline support and enablement. */
        RendererCapabilityState ray_tracing_pipelines;

        /** @brief Hardware ray-query support and enablement. */
        RendererCapabilityState ray_queries;

        /** @brief GPU memory-budget reporting support and enablement. */
        RendererCapabilityState gpu_memory_budget;

        /**
         * @brief Latest-ready presentation feature support and enablement.
         *
         * Enablement does not guarantee that the active swapchain
         * currently uses the corresponding presentation mode.
         */
        RendererCapabilityState latest_ready_presentation;
    };
} // namespace SNE::Engine::Renderer
