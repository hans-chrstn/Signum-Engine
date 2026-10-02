# Engine Development Checklist

## Phase 1 — Application Foundation

### Implement

- [x] Create application entry point
- [x] Create `GlfwContext`
- [x] Make `Application` own `GlfwContext`
- [x] Create `Window`
- [x] Separate Core and Platform code
- [x] Make `GlfwContext` non-copyable
- [x] Add header guard to `glfw_context.hpp`
- [x] Remove unnecessary GLFW include from `glfw_context.hpp`
- [x] Let GLFW select the platform automatically
- [x] Clean up namespace usage
- [x] Build engine implementation as reusable library target
- [x] Add repeatable clean command
- [x] Verify clean build
- [ ] Verify clean shutdown

### Learn

- [x] RAII
- [x] ownership
- [x] constructor/destructor lifetime relationships
- [x] non-copyable resource owners
- [x] application composition root
- [ ] process lifetime
- [ ] subsystem lifetime

### Test

- [ ] Verify Window destruction before GLFW termination
- [ ] Verify normal window-close path
- [ ] Verify repeated startup and shutdown

### Architecture

- [ ] Keep `Application` as orchestration
- [ ] Avoid turning `Application` into a global subsystem container
- [ ] Keep platform lifecycle ownership explicit

---

## Phase 2 — Error Handling Foundation

### Implement

- [x] Define engine error codes
- [x] Define error subsystems
- [x] Store human-readable messages
- [x] Create `NativeError`
- [x] Support optional native error context
- [x] Capture GLFW native errors
- [x] Add operation context
- [x] Add source location
- [x] Define initial fatal boundary
- [x] Preserve Vulkan native errors
- [x] Define recoverable error representation
- [ ] Define exception policy
- [ ] Define result-value policy
- [ ] Avoid generic catch-all handling that discards context

### Learn

- [x] exception propagation
- [x] `std::optional`
- [x] `std::source_location`
- [x] exception boundaries
- [ ] exceptions versus explicit result types
- [ ] runtime errors versus programmer errors

### Test

- [x] Test error codes
- [x] Test error messages
- [x] Test native error preservation
- [x] Test optional native context
- [x] Test subsystem classification
- [x] Test operation context
- [x] Test source location

### Architecture

- [ ] Keep error storage separate from formatting
- [ ] Keep error formatting separate from reporting
- [ ] Keep recoverability separate from error type
- [ ] Keep native API information intact

---

## Phase 3 — Vulkan Instance Foundation

### Learn

- [x] Vulkan instances
- [x] Vulkan extensions
- [x] Vulkan layers
- [x] debug callbacks
- [x] Vulkan API versioning
- [x] loader-supported version vs application-required version
- [x] surfaces
- [x] `VkResult`

### Implement

- [x] Create Vulkan instance
- [x] Get required GLFW Vulkan extensions
- [x] Validate required extensions
- [x] Enable validation layers
- [x] Create debug messenger
- [x] Query supported Vulkan instance API version
- [x] Define required Vulkan API version
- [x] Reject unsupported Vulkan API versions
- [x] Create window surface
- [x] Preserve `VkResult`
- [x] Add Vulkan error conversion
- [x] Add RAII Vulkan surface ownership

### Test

- [x] Test extension-selection logic
- [x] Test missing-extension behavior
- [x] Verify validation output
- [x] Verify Vulkan lifetime order
- [x] Verify Vulkan shutdown

### Architecture

- [x] Keep Vulkan inside renderer/backend code
- [ ] Avoid exposing Vulkan types to normal game APIs
- [x] Keep Vulkan initialization out of gameplay code
- [x] Keep Vulkan instance and surface ownership in separate RAII components

---

## Phase 4 — GPU Device Foundation

### Learn

- [x] physical devices
- [x] logical devices
- [x] queue families
- [x] queues
- [x] device capabilities
- [ ] feature negotiation
- [x] supported features versus enabled features
- [x] queue-family sharing versus distinct queue families

### Implement

- [x] Enumerate physical devices
- [x] Inspect device capabilities
- [x] Find queue families
- [x] Check required device extensions
- [x] Separate required and optional capabilities
- [x] Discover optional hardware ray-tracing capabilities without requiring them
- [x] Select physical device
- [x] Enforce required Vulkan API version for the loader and physical devices
- [x] Define logical-device feature configuration
- [x] Derive unique queue-family requests
- [x] Create RAII VulkanDevice owner
- [x] Create logical device
- [x] Retrieve graphics queue
- [x] Retrieve presentation queue
- [ ] Report detailed unsupported-device rejection reasons

### Test

- [x] Test device-selection logic
- [x] Test queue-family selection
- [x] Test Vulkan API-version policy and physical-device rejection below the required version
- [x] Test shared graphics/presentation queue-family request planning
- [x] Test distinct graphics/presentation queue-family request planning
- [x] Test unsupported-device behavior

### Architecture

- [x] Separate capability discovery from device-selection policy
- [x] Centralize the required Vulkan API version as shared backend policy
- [x] Separate supported capabilities from enabled device features
- [x] Keep queue-family request planning separate from device creation
- [x] Centralize required device extensions as shared backend policy
- [x] Keep VkDevice ownership explicit and RAII-managed
- [x] Keep hardware policy independent from gameplay
- [x] Keep optional GPU capabilities opt-in
- [x] Do not make renderer startup depend on hardware ray tracing

---

## Phase 5 — Swapchain and Presentation

### Learn

- [x] swapchains
- [x] surface formats
- [x] presentation modes
- [x] presentation preference versus Vulkan presentation mode
- [x] FIFO_LATEST_READY behavior and capability requirements
- [x] image views
- [ ] swapchain recreation

### Implement

- [x] Query swapchain support
- [x] Select surface format
- [x] Define presentation preference policy
- [x] Select presentation mode
- [x] Support FIFO selection
- [x] Support MAILBOX selection with FIFO fallback
- [x] Support FIFO_RELAXED selection with FIFO fallback
- [x] Support IMMEDIATE selection with safe fallback
- [x] Account for FIFO_LATEST_READY in presentation-mode policy
- [x] Select swap extent
- [x] Create swapchain
- [x] Select swapchain image count
- [x] Select composite-alpha mode
- [x] Create image views
- [ ] Handle out-of-date swapchain
- [ ] Handle suboptimal swapchain
- [ ] Handle minimized windows
- [ ] Recreate dependent resources safely

### Test

- [x] Test format selection
- [x] Test VSync presentation-mode selection
- [x] Test low-latency presentation-mode selection
- [x] Test adaptive presentation-mode selection
- [x] Test tearing-enabled presentation-mode selection
- [x] Test presentation-mode fallback behavior
- [x] Test FIFO_LATEST_READY selection policy
- [x] Test extent selection
- [x] Test swapchain image-count selection
- [x] Test composite-alpha selection
- [ ] Resize repeatedly
- [ ] Minimize and restore repeatedly

### Architecture

- [x] Keep presentation backend-specific
- [ ] Keep swapchain details out of game-facing APIs
- [x] Keep presentation preference independent from Vulkan presentation-mode enums
- [x] Keep presentation-mode capability separate from presentation policy

---

## Phase 5.5 — Foundation and Runtime Hardening

### Learn

- [x] programmer errors versus runtime failures
- [x] assertions and contract violations
- [x] development versus production diagnostics
- [x] Vulkan feature chains
- [x] Vulkan `pNext` composition
- [x] application configuration
- [x] continuous integration fundamentals
- [x] integration-test environment requirements

### Implement

- [x] Add assertion and contract handling
- [x] Separate programmer errors from runtime failures
- [x] Replace temporary `std::logic_error` handling in `VulkanDevice`
- [x] Add application configuration
- [x] Make application name configurable
- [x] Make initial window size configurable
- [x] Make presentation preference configurable
- [x] Make development diagnostics configurable
- [x] Allow Vulkan validation to be disabled
- [x] Enable debug-utils only when diagnostics require it
- [x] Replace legacy logical-device feature enablement
- [x] Add `VkPhysicalDeviceFeatures2` feature configuration
- [x] Add optional Vulkan feature-chain support
- [x] Discover FIFO_LATEST_READY extension support
- [x] Discover FIFO_LATEST_READY feature support
- [x] Enable FIFO_LATEST_READY only when requested and supported
- [x] Enable optional device extensions from selected features
- [x] Add logical-device queue-family preconditions
- [x] Validate required queue requests
- [x] Keep queue requests unique
- [x] Document Vulkan resource ownership and borrowed handles
- [x] Add Vulkan integration-test environment handling
- [x] Add GitHub Actions CI

### Test

- [x] Run clean Debug build
- [x] Run unit tests
- [x] Run supported integration tests
- [x] Run ASan/UBSan
- [x] Run Valgrind
- [x] Run Clang-Tidy
- [x] Run formatting checks
- [x] Test FIFO_LATEST_READY capability negotiation
- [x] Verify FIFO_LATEST_READY remains optional
- [x] Verify startup without FIFO_LATEST_READY support
- [x] Verify validation-enabled startup
- [x] Verify validation-disabled startup
- [x] Verify clean startup and shutdown
- [x] Generate Doxygen without new project warnings
- [x] Verify CI build and test workflows

### Architecture

- [x] Preserve explicit RAII ownership
- [x] Keep capability discovery separate from policy
- [x] Keep supported features separate from enabled features
- [x] Keep optional Vulkan features optional
- [x] Keep queue-request planning separate from device creation
- [x] Keep configuration small and explicit
- [x] Keep development diagnostics removable from production
- [x] Avoid global configuration state
- [x] Avoid speculative abstractions
- [x] Avoid premature subsystem splitting

---

## Phase 5.75 — Renderer Foundation Scalability

### Learn

- [x] composition-root boundaries
- [x] renderer runtime ownership
- [x] selected-resource invariants
- [x] frame-resource ownership boundaries
- [x] borrowed views versus owned containers
- [ ] swapchain recreation ownership requirements
- [ ] scalable queue-request modeling
- [ ] incremental abstraction versus speculative abstraction

### Implement

- [x] Define the renderer runtime ownership boundary
- [x] Prevent `Application` from becoming the permanent owner of per-frame Vulkan resources
- [x] Keep `Application` responsible for orchestration rather than rendering internals
- [x] Define where device, swapchain, command, and frame resources belong
- [x] Define per-frame resource ownership before adding synchronization
- [ ] Keep command-buffer lifetime tied to its command pool
- [x] Define a scalable command-pool ownership model
- [x] Define swapchain image and image-view access needed by rendering
- [x] Expose borrowed swapchain resource views without transferring ownership
- [x] Preserve selected physical-device queue-family invariants
- [x] Encode selected physical-device guarantees in runtime types
- [x] Remove hardcoded non-resizable window policy before swapchain recreation work
- [x] Prepare the application loop for non-blocking rendering
- [ ] Keep queue-request structures extensible for future compute and transfer queues
- [ ] Keep logical-device feature negotiation extensible for additional feature chains

### Test

- [x] Verify renderer resource destruction remains dependency-safe
- [x] Verify `Application` shutdown after renderer ownership changes
- [x] Verify command-pool lifetime remains shorter than logical-device lifetime
- [x] Verify borrowed swapchain views cannot outlive the swapchain owner
- [x] Verify selected physical devices always provide required queue families
- [x] Verify window creation remains valid with resizing enabled
- [x] Verify event polling does not block continuous renderer execution
- [x] Run clean Debug build
- [x] Run unit tests
- [x] Run supported integration tests
- [x] Run ASan/UBSan
- [x] Run Valgrind
- [x] Run Clang-Tidy
- [x] Run formatting checks
- [x] Run Vulkan validation cleanly
- [x] Generate Doxygen without new warnings

### Architecture

- [x] Keep `Application` as a composition root and high-level orchestrator
- [x] Keep renderer-runtime ownership outside normal application logic
- [x] Keep per-frame GPU state grouped by frame ownership
- [x] Avoid global command buffers, synchronization objects, and frame state
- [x] Support multiple frames in flight without redesigning ownership
- [ ] Allow future graphics, compute, and transfer command pools
- [x] Keep command-pool ownership independent from command-buffer recording policy
- [x] Keep swapchain ownership separate from frame ownership
- [x] Keep swapchain resource access non-owning
- [x] Keep physical-device discovery separate from selected-device guarantees
- [x] Keep supported capabilities separate from requested and enabled features
- [x] Keep queue planning separate from logical-device creation
- [ ] Allow queue requests to evolve without redesigning `VulkanDevice`
- [ ] Allow additional Vulkan feature structures without redesigning feature negotiation
- [ ] Avoid exposing Vulkan implementation details to game-facing APIs
- [x] Avoid introducing generic managers or registries without a concrete requirement
- [x] Prefer the smallest durable abstraction over demo-specific shortcuts
- [x] Do not introduce abstractions solely for hypothetical future systems
- [x] Preserve explicit RAII ownership and dependency-ordered destruction

---

## Phase 6 — Basic Rendering

### Learn

- [x] command pools
- [ ] command buffers
- [ ] synchronization
- [ ] frame ownership
- [ ] acquire-submit-present flow
- [ ] dynamic rendering

### Implement

- [x] Create command pool
- [ ] Create command buffers
- [ ] Create synchronization objects
- [ ] Implement frame loop
- [ ] Use Vulkan dynamic rendering
- [ ] Poll platform events
- [ ] Clear screen
- [ ] Render triangle
- [ ] Handle frame errors

### Test

- [ ] Run validation layers cleanly
- [ ] Run extended frame test
- [ ] Resize during rendering
- [ ] Verify clean renderer shutdown
- [ ] Run sanitizers

### Architecture

- [ ] Keep rendering loop independent from gameplay
- [ ] Keep synchronization details inside rendering subsystem
- [ ] Prefer dynamic rendering over legacy render-pass/framebuffer objects unless a concrete compatibility need requires them

---

## Phase 6.5 — GPU Memory, Transfers, and Lifetime

### Learn

- [ ] Vulkan memory heaps
- [ ] Vulkan memory types
- [ ] host-visible memory
- [ ] device-local memory
- [ ] coherent versus non-coherent memory
- [ ] memory requirements and alignment
- [ ] suballocation
- [ ] dedicated allocations
- [ ] staging transfers
- [ ] upload paths
- [ ] readback paths
- [ ] deferred destruction
- [ ] memory budgets

### Design

- [ ] Define GPU allocation responsibilities
- [ ] Define buffer-memory ownership
- [ ] Define image-memory ownership
- [ ] Define upload-memory policy
- [ ] Define readback-memory policy
- [ ] Define persistent-mapping policy
- [ ] Define resource-retirement policy
- [ ] Define allocation diagnostics
- [ ] Keep allocation implementation replaceable
- [ ] Avoid exposing allocator implementation to normal renderer callers

### Implement

- [ ] Add GPU memory allocation foundation
- [ ] Add memory-type selection
- [ ] Support buffer allocation
- [ ] Support image allocation
- [ ] Add staging-upload path
- [ ] Add mapped-upload path where appropriate
- [ ] Add readback path when needed
- [ ] Add alignment handling
- [ ] Add memory-budget queries when supported
- [ ] Add basic allocation statistics
- [ ] Add deferred resource retirement when frame lifetime requires it

### Test

- [ ] Test memory-type selection
- [ ] Test alignment calculations
- [ ] Test buffer allocation lifecycle
- [ ] Test image allocation lifecycle
- [ ] Test upload correctness
- [ ] Test partial allocation failure
- [ ] Test deferred retirement
- [ ] Run Vulkan validation
- [ ] Run sanitizers

### Architecture

- [ ] Separate GPU resource identity from memory allocation
- [ ] Keep allocation backend-specific
- [ ] Keep future streaming requirements possible
- [ ] Keep future memory-budget enforcement possible
- [ ] Keep future specialized allocator experiments possible
- [ ] Do not build a complex allocator before measurements justify it

---

## Phase 7 — Rendering Resource Foundation

### Learn

- [ ] shaders
- [ ] graphics pipelines
- [ ] buffers
- [ ] textures
- [ ] descriptors
- [ ] descriptor/binding models
- [ ] materials
- [ ] shader and pipeline variants
- [ ] compute pipelines
- [ ] GPU resource lifetime
- [ ] GPU resource dependency tracking

### Implement

- [ ] Add shader handling
- [ ] Add graphics pipelines
- [ ] Add vertex buffers
- [ ] Add index buffers
- [ ] Add uniform/storage buffers
- [ ] Add textures
- [ ] Add descriptor/binding infrastructure
- [ ] Add reusable shader/pipeline variant infrastructure
- [ ] Add compute-pipeline support when a real workload requires it
- [ ] Add material parameter/binding foundation
- [ ] Add depth buffering
- [ ] Render multiple objects
- [ ] Separate scene data from GPU resources
- [ ] Define rendering-facing data contracts
- [ ] Define GPU resource ownership
- [ ] Define GPU lifetime ordering
- [ ] Track GPU resource dependencies
- [ ] Expose resource lifetime/dependency data needed by later frame scheduling

### Test

- [ ] Test pure resource descriptions
- [ ] Test shader/pipeline/binding descriptions without optional rendering modules
- [ ] Test material parameter/binding behavior without effect-specific assumptions
- [ ] Test partial creation failures
- [ ] Verify destruction ordering
- [ ] Run Vulkan validation
- [ ] Run sanitizers

### Architecture

- [ ] Keep runtime object identity separate from GPU resource identity
- [ ] Keep GPU allocation details behind Phase 6.5 memory mechanisms
- [ ] Avoid raw Vulkan ownership in game entities
- [ ] Keep resource, shader, pipeline, binding, and material mechanisms independent from optional rendering features
- [ ] Keep core GPU resource APIs usable by built-in and developer-defined render features
- [ ] Avoid baking PBR, shadow, terrain, water, ray-tracing-effect, or other optional-feature semantics into renderer-core resource types

---

## Phase 8 — Production Error Handling

### Implement

- [x] Create structured `EngineError`
- [x] Add error context
- [x] Add source location
- [x] Add diagnostic formatter
- [x] Format subsystem
- [x] Format operation
- [x] Format error code
- [x] Format message
- [x] Format optional native error
- [x] Format file and line
- [x] Format function name
- [x] Add current GLFW error conversion
- [x] Add Vulkan error conversion
- [ ] Add nested causes only if needed
- [x] Define recoverable result type policy
- [ ] Enforce the Phase 5.5 exception policy across subsystems
- [ ] Enforce the Phase 5.5 assertion/contract policy across subsystems
- [x] Add fatal error reporting
- [x] Add graceful fatal shutdown
- [ ] Prevent duplicate error reporting
- [ ] Audit recoverable failures for accidental fatal handling

### Learn

- [x] formatting
- [x] reporting concept
- [x] exception boundaries
- [x] source-location diagnostics
- [ ] fatal reporting
- [ ] explicit result types
- [ ] nested error causes when justified

### Test

- [x] Test formatter without native context
- [x] Test formatter with native context
- [x] Test native values
- [x] Test core diagnostic fields
- [x] Test source location formatting
- [ ] Test fatal reporter
- [ ] Test recoverable result behavior
- [ ] Test programmer-contract failure behavior where testable
- [ ] Test runtime failure propagation through the fatal boundary
- [ ] Test duplicate-report prevention

### Architecture

- [x] Keep `EngineError` independent from terminal output
- [x] Keep formatting independent from reporting
- [x] Keep fatal reporting at defined boundary
- [ ] Keep recoverable failures non-fatal
- [ ] Keep programmer-contract failures distinct from runtime/environment failures

---

## Phase 9 — Logging and Diagnostics

### Learn

- [ ] log levels
- [ ] categories
- [ ] sinks
- [ ] structured logging
- [ ] thread-safe logging

### Implement

- [ ] Add logging abstraction
- [ ] Add trace level
- [ ] Add debug level
- [ ] Add info level
- [ ] Add warning level
- [ ] Add error level
- [ ] Add fatal level
- [ ] Add subsystem categories
- [ ] Add timestamps
- [ ] Add terminal sink
- [ ] Add file sink
- [ ] Route Vulkan validation messages
- [ ] Route GLFW diagnostics
- [ ] Support logging without editor

### Test

- [ ] Test level filtering
- [ ] Test category filtering
- [ ] Test terminal sink
- [ ] Test file sink
- [ ] Test logging failures

### Architecture

- [ ] Keep logging independent from editor
- [ ] Keep logging implementation out of gameplay APIs

---

## Phase 9.5 — Developer Diagnostics UI

### Learn

- [ ] immediate-mode debug UI concepts
- [ ] frame-time measurement
- [ ] rolling performance graphs
- [ ] debug-overlay ownership
- [ ] CPU/GPU metric boundaries

### Design

- [ ] Keep developer UI separate from engine systems
- [ ] Make developer UI consume diagnostics and metrics
- [ ] Avoid engine subsystems depending directly on Dear ImGui
- [ ] Keep developer UI removable from production builds
- [ ] Define debug-metric lifetime and ownership
- [ ] Keep profiler data independent from visualization

### Implement

- [ ] Integrate Dear ImGui for developer tooling
- [ ] Add developer UI root
- [ ] Add show/hide developer UI toggle
- [ ] Add frame-time display
- [ ] Add FPS display
- [ ] Add rolling frame-time graph
- [ ] Add Vulkan device information panel
- [ ] Add Vulkan API/version information
- [ ] Add swapchain information panel
- [ ] Add renderer statistics panel
- [ ] Add log viewer
- [ ] Display Vulkan validation messages
- [ ] Display engine diagnostics
- [ ] Add basic memory/resource counters
- [ ] Add basic frame counters
- [ ] Add debug visualization toggle infrastructure

### Test

- [ ] Launch with developer UI enabled
- [ ] Launch with developer UI disabled
- [ ] Toggle developer UI at runtime
- [ ] Verify diagnostics appear in log viewer
- [ ] Verify frame metrics update correctly
- [ ] Verify developer UI does not own engine diagnostic data
- [ ] Verify engine can build without developer UI
- [ ] Verify developer UI shutdown is clean

### Architecture

- [ ] Keep Dear ImGui behind developer-tooling boundaries
- [ ] Keep profiling data independent from Dear ImGui
- [ ] Keep logging independent from developer UI
- [ ] Keep renderer independent from developer UI
- [ ] Keep developer UI out of game-facing APIs
- [ ] Allow future editor UI to consume the same diagnostics data
- [ ] Allow future profiler views to replace temporary ImGui views

---

## Phase 10 — Testing and Debugging Foundation

### Implement

- [x] Use GoogleTest
- [x] Integrate GoogleTest with CTest
- [x] Run tests through `just`
- [x] Keep `just check` clean
- [x] Add ASan
- [x] Add UBSan
- [x] Add Valgrind workflow
- [x] Add Clang-Tidy
- [x] Treat project warnings as errors
- [x] Add integration-test structure
- [ ] Add benchmark target
- [ ] Add reusable benchmark helpers
- [ ] Define benchmark result-recording convention

### Learn

- [x] `EXPECT_*`
- [x] `ASSERT_*`
- [x] GoogleTest discovery
- [x] static-analysis limitations around test macros
- [ ] LLDB or GDB
- [ ] breakpoints
- [ ] stepping
- [ ] call stacks
- [ ] watch expressions
- [ ] conditional breakpoints
- [ ] test fixtures
- [ ] parameterized tests
- [ ] test doubles
- [ ] benchmark methodology

### Test

- [x] Run unit tests in debug
- [x] Run unit tests under sanitizers
- [x] Run memory checks
- [ ] Debug a deliberately failing test
- [ ] Inspect a thrown exception with debugger
- [ ] Establish initial benchmark baseline

### Architecture

- [ ] Keep deterministic logic in unit tests
- [ ] Keep driver/OS/hardware behavior in integration tests
- [ ] Keep benchmarks separate from correctness tests
- [ ] Make performance comparisons reproducible enough to guide later optimization

---

## Phase 11 — Input and Timing

### Learn

- [ ] event-driven input
- [ ] input state
- [ ] frame timing
- [ ] variable timestep
- [ ] fixed timestep

### Implement

- [ ] Add keyboard input
- [ ] Add mouse input
- [ ] Add scrolling
- [ ] Add resize events
- [ ] Add focus events
- [ ] Add close events
- [ ] Separate input from GLFW callbacks
- [ ] Add monotonic timing
- [ ] Add frame delta
- [ ] Add fixed-update support

### Test

- [ ] Test key press/release
- [ ] Test mouse state
- [ ] Test scroll input
- [ ] Test timing utilities
- [ ] Test event translation

### Architecture

- [ ] Keep GLFW types out of engine-facing input API
- [ ] Keep actions such as Jump/Shoot project-defined

---

## Phase 12 — Renderer Abstraction

### Learn

- [ ] RHI concepts
- [ ] backend abstraction
- [ ] renderer handles
- [ ] renderer resource descriptions
- [ ] render-feature architecture
- [ ] render-pass registration
- [ ] material abstraction
- [ ] abstraction overhead
- [ ] low-level backend escape hatches

### Design

- [ ] Define minimum renderer API
- [ ] Define backend-independent resource descriptions
- [ ] Define renderer resource-handle requirements
- [ ] Define renderer ownership model
- [ ] Define renderer-core versus render-feature boundary
- [ ] Define render-feature lifecycle and registration contract
- [ ] Define render-feature dependency and ordering declarations
- [ ] Define custom render-pass registration contract
- [ ] Define material-system boundary
- [ ] Define custom shader/material extension contracts
- [ ] Define optional render-feature enable/disable/configuration model
- [ ] Define built-in render-feature replacement and extension contract
- [ ] Define public mechanisms shared by built-in and developer-defined render features
- [ ] Define no-meaningful-overhead policy for disabled optional features
- [ ] Define hardware ray-tracing infrastructure boundary separately from ray-traced effects
- [ ] Define Vulkan escape-hatch policy
- [ ] Avoid forcing future backends into an artificial lowest common denominator

### Implement

- [ ] Move normal rendering callers away from raw Vulkan
- [ ] Keep Vulkan as first backend
- [ ] Avoid premature additional graphics backends
- [ ] Add render-feature registry when a real feature requires it
- [ ] Add public custom render-feature registration path
- [ ] Add public custom render-pass registration path
- [ ] Add public feature enable/disable/configuration path
- [ ] Expose shader, pipeline, descriptor/binding, and material extension mechanisms through renderer APIs
- [ ] Preserve explicit low-level Vulkan access for advanced extensions
- [ ] Keep abstraction measurable

### Test

- [ ] Test resource descriptions
- [ ] Verify public renderer API avoids unnecessary Vulkan types
- [ ] Verify renderer-core builds without optional rendering modules
- [ ] Disable optional render features without breaking unrelated rendering
- [ ] Test developer-defined render feature through public renderer mechanisms
- [ ] Verify built-in and custom render features use the same registration and resource contracts
- [ ] Verify built-in render features have no privileged private-core path unavailable to custom features
- [ ] Verify replacing one render-feature implementation does not require unrelated renderer changes
- [ ] Benchmark abstraction where useful

### Architecture

- [ ] Engine provides mechanisms, built-in modules provide implementations, developers provide specialization
- [ ] Keep renderer-core focused on reusable mechanisms rather than specific rendering effects
- [ ] Keep renderer-core independent from optional rendering modules
- [ ] Require optional rendering modules to depend on renderer-core, never the reverse
- [ ] Give built-in render features no privileged architectural shortcuts unavailable to custom features
- [ ] Allow built-in rendering modules to be enabled, disabled, configured, replaced, and extended
- [ ] Keep game rendering API independent from Vulkan
- [ ] Keep explicit low-level backend access for advanced extensions
- [ ] Keep unused optional render features uninitialized and unallocated
- [ ] Leave frame-graph scheduling and graph compilation to Phase 13.5

---

## Phase 13 — Resource Handles and Identity

### Learn

- [ ] handles
- [ ] stable IDs
- [ ] generation counters
- [ ] stale handles
- [ ] ownership versus references

### Design

- [ ] Define runtime resource identity
- [ ] Define invalid handle behavior
- [ ] Define stale handle behavior
- [ ] Define owning versus non-owning access

### Implement

- [ ] Add typed handle mechanism when needed
- [ ] Add stale-handle protection when needed

### Test

- [ ] Test invalid handles
- [ ] Test destroyed resources
- [ ] Test reused IDs
- [ ] Test stale generations if used

---

## Phase 13.5 — Render Graph and Frame Scheduling

### Learn

- [ ] render graphs
- [ ] directed acyclic graphs
- [ ] resource dependencies
- [ ] resource states
- [ ] synchronization planning
- [ ] transient resources
- [ ] pass ordering
- [ ] pass culling
- [ ] queue scheduling
- [ ] resource aliasing concepts

### Design

- [ ] Define render-pass declaration contract
- [ ] Define resource read/write declarations
- [ ] Define pass dependencies
- [ ] Define graph compilation
- [ ] Define resource lifetime analysis
- [ ] Define synchronization-generation boundary
- [ ] Define transient-resource ownership
- [ ] Define external-resource import/export
- [ ] Define render-feature registration against the graph
- [ ] Keep graph policy independent from individual effects

### Implement

- [ ] Add graph representation
- [ ] Add pass registration
- [ ] Add resource declarations
- [ ] Add dependency resolution
- [ ] Add deterministic pass ordering
- [ ] Detect dependency cycles
- [ ] Add lifetime analysis
- [ ] Add synchronization planning
- [ ] Add transient-resource support when required
- [ ] Add pass culling when justified
- [ ] Add queue scheduling when multiple queue types are actually used

### Test

- [ ] Test dependency ordering
- [ ] Test independent passes
- [ ] Test cycle detection
- [ ] Test read/write dependencies
- [ ] Test lifetime calculations
- [ ] Test transient-resource lifetime
- [ ] Test synchronization planning
- [ ] Test unused-pass removal when implemented

### Architecture

- [ ] Built-in and external render features use the same graph contracts
- [ ] Render graph depends on renderer mechanisms, not individual effects
- [ ] Do not encode PBR, shadows, terrain, or other feature semantics into graph core
- [ ] Keep synchronization details inside renderer/backend layers

---

## Phase 14 — Scene and World Foundation

### Learn

- [ ] scene containers
- [ ] world containers
- [ ] transforms
- [ ] transform hierarchies
- [ ] logical versus spatial hierarchy

### Design

- [ ] Define World ownership
- [ ] Define Scene role
- [ ] Define scene serialization role
- [ ] Define transform hierarchy rules

### Implement

- [ ] Add world lifecycle
- [ ] Add scene lifecycle
- [ ] Add transform type
- [ ] Add object creation
- [ ] Add object destruction
- [ ] Add hierarchy support when needed
- [ ] Connect world data to renderer through defined contracts

### Test

- [ ] Test creation
- [ ] Test destruction
- [ ] Test transforms
- [ ] Test hierarchy changes
- [ ] Test invalid references

### Architecture

- [ ] Keep gameplay types project-defined
- [ ] Keep scene ownership independent from renderer ownership

---

## Phase 15 — Entity and Component Model

### Learn

- [ ] entity-component architecture
- [ ] ECS architecture
- [ ] sparse sets
- [ ] archetypes
- [ ] cache locality
- [ ] component lifetime
- [ ] stable identity

### Design

- [ ] Define entity identity
- [ ] Define component ownership
- [ ] Define component behavior model
- [ ] Define component iteration requirements
- [ ] Define query requirements
- [ ] Define editor requirements
- [ ] Define serialization requirements
- [ ] Benchmark storage alternatives

### Implement

- [ ] Add entity identity
- [ ] Add component registration
- [ ] Add component storage
- [ ] Add component lookup
- [ ] Add component removal
- [ ] Add entity destruction
- [ ] Add iteration/query API
- [ ] Support project-defined components

### Test

- [ ] Test entity creation
- [ ] Test entity destruction
- [ ] Test component add/remove
- [ ] Test component lifetime
- [ ] Test stale entity access
- [ ] Test queries
- [ ] Benchmark iteration

### Architecture

- [ ] Do not hard-code Player
- [ ] Do not hard-code Enemy
- [ ] Do not hard-code Card
- [ ] Do not hard-code Weapon
- [ ] Do not hard-code Planet
- [ ] Do not hard-code gameplay components

---

## Phase 16 — Serialization

### Learn

- [ ] serialization
- [ ] schema evolution
- [ ] versioning
- [ ] migrations
- [ ] persistent IDs

### Design

- [ ] Separate memory layout from serialized layout
- [ ] Define format versions
- [ ] Define stable type IDs
- [ ] Define stable property IDs
- [ ] Define migration policy
- [ ] Define transient fields

### Implement

- [ ] Serialize basic engine values
- [ ] Serialize scenes
- [ ] Serialize entities/components
- [ ] Deserialize with validation
- [ ] Handle unsupported versions
- [ ] Add migrations when first required

### Test

- [ ] Round-trip tests
- [ ] malformed data tests
- [ ] missing field tests
- [ ] version mismatch tests
- [ ] migration tests

---

## Phase 17 — Reflection and Metadata

### Learn

- [ ] runtime metadata
- [ ] compile-time metadata
- [ ] type erasure
- [ ] property metadata
- [ ] registration systems
- [ ] generated-code tradeoffs

### Design

- [ ] Define metadata required by serialization
- [ ] Define metadata required by inspector
- [ ] Define stable type identity
- [ ] Define stable property identity
- [ ] Define user-type registration
- [ ] Minimize repeated registration
- [ ] Minimize unnecessary macros
- [ ] Keep metadata inspectable

### Implement

- [ ] Register engine value types
- [ ] Register engine components
- [ ] Expose type metadata
- [ ] Expose property metadata
- [ ] Connect metadata to serialization
- [ ] Support one project-defined component

### Test

- [ ] Test type lookup
- [ ] Test property lookup
- [ ] Test duplicate type IDs
- [ ] Test duplicate property IDs
- [ ] Test metadata serialization

### Future Consumers

- [ ] Inspector
- [ ] undo/redo
- [ ] prefabs
- [ ] scripting bindings
- [ ] asset editors
- [ ] networking metadata
- [ ] property animation

---

## Phase 17.5 — Filesystem and Project Path Foundation

### Learn

- [ ] physical paths
- [ ] normalized paths
- [ ] project roots
- [ ] engine roots
- [ ] source-asset paths
- [ ] filesystem error handling

### Design

- [ ] Define engine root
- [ ] Define project root
- [ ] Define source-asset root
- [ ] Define generated/cache root
- [ ] Define normalized path rules
- [ ] Separate physical path from asset identity
- [ ] Keep platform filesystem details behind filesystem utilities

### Implement

- [ ] Add basic file reads
- [ ] Add basic file writes
- [ ] Add path normalization
- [ ] Add project-relative path handling
- [ ] Add engine-relative path handling
- [ ] Add source-asset path handling
- [ ] Add cache/output paths
- [ ] Add filesystem diagnostics

### Test

- [ ] Test normalization
- [ ] Test missing files
- [ ] Test invalid paths
- [ ] Test project-relative paths
- [ ] Test engine-relative paths
- [ ] Test traversal and root-boundary rules

### Architecture

- [ ] Give Phase 18 stable file/path semantics
- [ ] Do not implement mounts or a full virtual filesystem yet
- [ ] Do not equate asset identity with filesystem path

---

## Phase 18 — Asset and Resource System

### Learn

- [ ] source assets
- [ ] runtime resources
- [ ] asset IDs
- [ ] importers
- [ ] dependency graphs
- [ ] cache invalidation
- [ ] asynchronous loading

### Design

- [ ] Define asset identity
- [ ] Separate path from asset identity
- [ ] Separate source asset from runtime resource
- [ ] Define importer interface
- [ ] Define asset metadata
- [ ] Define dependency tracking
- [ ] Define shader/material asset dependencies without coupling assets to built-in render features
- [ ] Define cache versioning

### Implement

- [ ] Add asset database
- [ ] Add resource manager
- [ ] Add first importer
- [ ] Add asset load
- [ ] Add asset unload
- [ ] Add dependency tracking
- [ ] Support custom shader/material assets through generic asset/resource mechanisms
- [ ] Add reload path
- [ ] Add asset diagnostics

### Test

- [ ] Test missing assets
- [ ] Test invalid assets
- [ ] Test asset identity
- [ ] Test dependencies
- [ ] Test custom shader/material asset dependencies
- [ ] Test shader/material assets with built-in rendering modules disabled
- [ ] Test reload

### Architecture

- [ ] Keep shader/material asset formats independent from ownership by any built-in render feature
- [ ] Let built-in and custom render features consume the same asset/resource mechanisms

---

## Phase 18.5 — Asset Cooking and Runtime Formats

### Learn

- [ ] asset cooking
- [ ] source versus cooked data
- [ ] content hashing
- [ ] cache invalidation
- [ ] format versioning
- [ ] binary layout
- [ ] compression
- [ ] platform-specific cooking

### Design

- [ ] Define cooked-resource contract
- [ ] Define cooked-format versioning
- [ ] Define importer-version tracking
- [ ] Define content hashes
- [ ] Define deterministic cooking requirements
- [ ] Define dependency fingerprints
- [ ] Define cache invalidation rules
- [ ] Keep source decoders separate from runtime formats
- [ ] Define platform-specific cooked variants where justified

### Implement

- [ ] Add cooking pipeline
- [ ] Add cooked-resource cache
- [ ] Add deterministic cache keys
- [ ] Add first cooked texture representation
- [ ] Add first cooked mesh representation when mesh resources exist
- [ ] Add resource-format version checks
- [ ] Add rebuild path for stale cooked data

### Source-Format Policy

- [ ] Use mature decoders for source formats where appropriate
- [ ] Keep `stb_image` or replacement decoder inside importer boundaries
- [ ] Do not make PNG/JPEG decoding part of runtime resource architecture
- [ ] Prefer Signum cooked textures at runtime
- [ ] Do not rewrite commodity codecs without a measured reason

### Test

- [ ] Test deterministic cooking
- [ ] Test cache hits
- [ ] Test cache invalidation
- [ ] Test importer-version changes
- [ ] Test corrupted cooked resources
- [ ] Test unsupported format versions
- [ ] Test source-file changes rebuild dependent resources

### Architecture

- [ ] Source formats are editor/import concerns
- [ ] Runtime formats are Signum-controlled
- [ ] Runtime resource layout may evolve independently from source file format
- [ ] Custom Signum formats target runtime requirements rather than novelty

---

## Phase 19 — Module and Build Architecture

### Learn

- [ ] static libraries
- [ ] shared libraries
- [ ] dynamic loading
- [ ] symbol visibility
- [ ] target dependency graphs
- [ ] C++ ABI limitations
- [ ] C ABI boundaries
- [ ] version negotiation

### Design

- [ ] Define engine module responsibilities
- [ ] Define module dependency direction
- [ ] Define CMake target dependency direction
- [ ] Define project module boundary
- [ ] Define optional module lifecycle
- [ ] Define optional rendering-module lifecycle
- [ ] Define built-in rendering features as optional modules rather than renderer-core services
- [ ] Define module dependency declarations
- [ ] Define render-feature module dependency declarations
- [ ] Define module enable/disable configuration
- [ ] Define replacement/override contract for built-in rendering modules
- [ ] Define replacement selection and precedence rules
- [ ] Define startup/shutdown contract
- [ ] Define interface versioning
- [ ] Define ABI policy
- [ ] Preserve editor/runtime/headless executable separation
- [ ] Keep backend targets replaceable without making every subsystem an interface

### Implement

- [ ] Split justified subsystems into CMake targets
- [ ] Separate platform/backend targets when real consumers justify it
- [ ] Separate renderer backend from higher-level renderer when justified
- [ ] Keep editor-only dependencies out of runtime targets
- [ ] Preserve headless-compatible reusable targets
- [ ] Add minimal module lifecycle
- [ ] Load optional rendering module through normal module mechanisms when justified
- [ ] Register a built-in render feature through the same public path available to custom modules
- [ ] Support per-project enable/disable of optional rendering modules
- [ ] Load test module
- [ ] Reject incompatible module versions

### Test

- [ ] Test module loading
- [ ] Test invalid module
- [ ] Test version mismatch
- [ ] Test dependency ordering
- [ ] Test CMake target dependency direction
- [ ] Verify runtime target does not link editor-only dependencies
- [ ] Verify reusable targets can support headless tooling
- [ ] Test renderer-core with all optional rendering modules disabled
- [ ] Test disabled rendering module does not initialize or allocate feature resources
- [ ] Test multiple optional rendering modules coexist through normal registration paths
- [ ] Test custom rendering module uses the same lifecycle and registration path as built-in modules
- [ ] Test replacement rendering module without modifying unrelated renderer systems
- [ ] Test module shutdown

### Architecture

- [ ] Avoid promising stable C++ ABI prematurely
- [ ] Avoid exposing unstable internals across module boundaries
- [ ] Avoid excessive micro-libraries
- [ ] Split targets only when ownership, dependencies, or consumers justify the boundary
- [ ] Keep renderer-core free of dependencies on optional rendering modules
- [ ] Require built-in rendering modules to use the same module and extension contracts available to external modules
- [ ] Keep optional rendering-module startup, shutdown, configuration, and replacement outside renderer-core special cases

---

## Phase 20 — Game-Facing C++ API

### Design

- [ ] Use C++23 as initial game-code language
- [ ] Allow ordinary project-defined C++ types
- [ ] Allow project-defined components
- [ ] Allow project-defined systems
- [ ] Avoid mandatory giant gameplay base classes
- [ ] Avoid mandatory proprietary runtime model
- [ ] Keep normal C++ debugging possible
- [ ] Keep normal C++ profiling possible
- [ ] Hide Vulkan boilerplate
- [ ] Hide platform boilerplate
- [ ] Hide unnecessary threading boilerplate
- [ ] Preserve advanced extension access
- [ ] Allow project-defined shaders and materials
- [ ] Allow project-defined render features and render passes through public renderer mechanisms
- [ ] Allow projects to enable, disable, and configure optional rendering modules without Vulkan-facing code

### Implement

- [ ] Create project/game module
- [ ] Expose runtime APIs
- [ ] Add project-defined component example
- [ ] Add project-defined system example
- [ ] Add project-defined render feature example when renderer extension API exists
- [ ] Run project without modifying engine core

### Test

- [ ] Build standalone project module
- [ ] Run project-defined behavior
- [ ] Verify project does not need Vulkan calls
- [ ] Verify project does not need GLFW calls
- [ ] Verify project-defined render feature does not require renderer-core modification
- [ ] Verify project-defined render feature can use the same render graph, resource, shader, pipeline, binding, and material mechanisms as built-in features
- [ ] Verify project runs without scripting runtime

---

## Phase 21 — Optional Language Strategy

### Lua/Luau

- [ ] Re-evaluate after metadata system exists
- [ ] Consider gameplay scripting
- [ ] Consider modding
- [ ] Consider editor automation
- [ ] Keep optional
- [ ] Generate bindings from metadata where practical

### C#

- [ ] Re-evaluate after metadata system exists
- [ ] Evaluate managed/native lifetime model
- [ ] Evaluate runtime deployment
- [ ] Evaluate garbage-collection implications
- [ ] Evaluate tooling benefits
- [ ] Keep optional

### Rust

- [ ] Consider for native modules where useful
- [ ] Consider for standalone tools where useful
- [ ] Do not require for game development

### Custom Language

- [ ] Do not implement during foundation work
- [ ] Reconsider only after concrete need

### Architecture

- [ ] Keep C++ usable without scripting
- [ ] Avoid separate duplicated engine API for every language
- [ ] Use shared metadata where practical
- [ ] Keep scripting overhead absent when scripting is unused

---

## Phase 22 — Job and Task System

### Learn

- [ ] threads
- [ ] `std::jthread`
- [ ] mutexes
- [ ] atomics
- [ ] condition variables
- [ ] data races
- [ ] task schedulers
- [ ] work stealing
- [ ] cancellation
- [ ] thread affinity

### Design

- [ ] Define task representation
- [ ] Define task dependencies
- [ ] Define cancellation
- [ ] Define task result ownership
- [ ] Define main-thread-only operations
- [ ] Define renderer thread rules

### Implement

- [ ] Add scheduler
- [ ] Add task submission
- [ ] Add task dependencies
- [ ] Add waiting
- [ ] Add cancellation
- [ ] Add main-thread dispatch

### Test

- [ ] Test task ordering
- [ ] Test dependencies
- [ ] Test cancellation
- [ ] Test shutdown with pending tasks
- [ ] Add concurrency stress tests
- [ ] Run ThreadSanitizer where supported

---

## Phase 23 — Performance and Memory Foundation

### Learn

- [ ] CPU caches
- [ ] memory locality
- [ ] allocations
- [ ] false sharing
- [ ] SIMD fundamentals
- [ ] CPU profiling
- [ ] GPU profiling
- [ ] benchmarking

### Implement

- [ ] Add CPU profiling hooks
- [ ] Add frame timing metrics
- [ ] Add allocation diagnostics
- [ ] Add benchmark suite
- [ ] Profile component iteration
- [ ] Profile asset loading
- [ ] Profile renderer submission
- [ ] Establish renderer-core baseline with optional rendering features disabled
- [ ] Add per-render-feature CPU/GPU/resource counters when feature modules exist

### Optimize When Measured

- [ ] Cache-friendly layouts
- [ ] batching
- [ ] specialized allocators
- [ ] SIMD
- [ ] task batching
- [ ] asynchronous pipelines
- [ ] indirect rendering
- [ ] GPU-driven rendering
- [ ] resource indexing/bindless techniques
- [ ] renderer multithreading
- [ ] GPU work

### Test

- [ ] Track benchmark regressions
- [ ] Record optimization baselines
- [ ] Verify optimized behavior remains correct
- [ ] Measure CPU, GPU, memory, and resource overhead of disabled optional rendering features
- [ ] Verify disabled optional rendering features remain at the renderer-core baseline within defined tolerances

---

## Phase 24 — Virtual Filesystem and Mount Architecture

### Learn

- [ ] virtual paths
- [ ] mount tables
- [ ] mount priority
- [ ] packaged resources
- [ ] archive/container access
- [ ] read-only mounts
- [ ] overlays

### Design

- [ ] Build on Phase 17.5 normalized physical-path rules
- [ ] Define virtual path syntax
- [ ] Define mount lifecycle
- [ ] Define mount priority
- [ ] Define duplicate-path behavior
- [ ] Define read-only and writable mounts
- [ ] Define packaged-resource access
- [ ] Define project/engine mount conventions
- [ ] Preserve physical-path escape hatch for tooling
- [ ] Keep logical asset identity independent from mount location

### Implement

- [ ] Add mount table
- [ ] Add virtual-to-physical resolution
- [ ] Add read-only mounts
- [ ] Add writable mounts where justified
- [ ] Add mount priorities
- [ ] Add packaged/archive mount when needed
- [ ] Integrate asset loading through virtual paths where beneficial

### Test

- [ ] Test mount resolution
- [ ] Test mount priority
- [ ] Test duplicate paths
- [ ] Test missing resources
- [ ] Test read-only behavior
- [ ] Test unmount lifecycle
- [ ] Test project and engine mounts

### Architecture

- [ ] Do not make virtual paths mandatory for every engine user
- [ ] Keep asset identity separate from both physical and virtual paths
- [ ] Keep mount behavior out of gameplay-specific concepts

---

## Phase 25 — Physics

### Learn

- [ ] rigid-body simulation architecture
- [ ] collision shapes
- [ ] broad phase
- [ ] narrow phase
- [ ] constraints
- [ ] fixed-step simulation
- [ ] physics queries
- [ ] backend capability boundaries

### Design

- [ ] Define Signum physics API before exposing backend types
- [ ] Keep physics backend replaceable
- [ ] Define physics ownership
- [ ] Define physics-resource identity
- [ ] Define simulation ownership
- [ ] Define physics/world synchronization
- [ ] Define fixed-step interaction
- [ ] Define backend capability reporting
- [ ] Keep gameplay collision responses project-defined
- [ ] Keep specialized large-world simulation possible

### Initial Backend

- [ ] Evaluate Jolt against current requirements
- [ ] Use Jolt as initial backend if requirements still fit
- [ ] Keep Jolt types behind backend implementation boundaries
- [ ] Avoid designing gameplay around Jolt-specific behavior
- [ ] Measure representative physics workloads before considering replacement

### Implement

- [ ] Add physics world
- [ ] Add body representation
- [ ] Add collision shapes
- [ ] Add queries
- [ ] Add events
- [ ] Connect fixed-step simulation
- [ ] Connect world synchronization through Signum-facing contracts

### Test

- [ ] Test creation/destruction
- [ ] Test collision
- [ ] Test queries
- [ ] Test synchronization
- [ ] Test fixed-step behavior
- [ ] Test backend shutdown
- [ ] Verify gameplay-facing code does not require Jolt types

### Future Evolution

- [ ] Allow specialized procedural-collision systems
- [ ] Allow large-world physics specialization
- [ ] Allow experimental solver or broad-phase research without rewriting gameplay APIs
- [ ] Replace general-purpose backend pieces only when measurements justify it

---

## Phase 26 — Audio

### Learn

- [ ] audio devices and contexts
- [ ] buffers and streaming
- [ ] voices/sources
- [ ] listeners
- [ ] spatial audio
- [ ] attenuation
- [ ] HRTF concepts
- [ ] mixing and DSP boundaries

### Design

- [ ] Define Signum audio resource model
- [ ] Define playback API
- [ ] Define audio ownership
- [ ] Define audio-device ownership
- [ ] Define voices/sources
- [ ] Define listener model
- [ ] Define spatial-audio API
- [ ] Define streaming-audio model
- [ ] Define backend capability reporting
- [ ] Keep backend replaceable
- [ ] Keep native audio handles out of game-facing APIs

### Initial Backend

- [ ] Evaluate OpenAL Soft against current requirements
- [ ] Use OpenAL Soft as initial backend if requirements still fit
- [ ] Keep OpenAL types inside backend implementation
- [ ] Keep audio resources independent from OpenAL buffer identity
- [ ] Keep future custom mixer/DSP work possible

### Implement

- [ ] Add audio backend
- [ ] Add audio resources
- [ ] Add playback
- [ ] Add streaming playback when required
- [ ] Add spatial audio
- [ ] Add listener state
- [ ] Add volume categories
- [ ] Integrate source-audio import/cooking with the asset pipeline

### Test

- [ ] Test missing audio device
- [ ] Test missing resource
- [ ] Test playback lifecycle
- [ ] Test streaming lifecycle
- [ ] Test spatial-source lifecycle
- [ ] Verify gameplay-facing code does not require OpenAL types

### Future Evolution

- [ ] Support custom DSP when justified
- [ ] Support custom acoustic simulation when justified
- [ ] Support alternative backend without changing gameplay-facing API
- [ ] Keep mature OS/device infrastructure when replacing it offers no measurable benefit

---

## Phase 27 — Runtime UI

### Learn

- [ ] retained-mode UI concepts
- [ ] layout systems
- [ ] text shaping/rendering requirements
- [ ] input routing
- [ ] focus and navigation
- [ ] UI rendering pipelines

### Design

- [ ] Build runtime UI as a Signum-owned system
- [ ] Separate runtime UI from editor UI
- [ ] Separate runtime UI from Phase 9.5 Dear ImGui developer tooling
- [ ] Define UI ownership
- [ ] Define input integration
- [ ] Define rendering integration
- [ ] Define backend-independent widget state
- [ ] Define layout independently from rendering backend
- [ ] Keep runtime UI optional
- [ ] Keep input routing independent from GLFW
- [ ] Keep rendering integration independent from Vulkan-specific UI types
- [ ] Do not make Dear ImGui the runtime UI architecture

### Implement

- [ ] Add UI root
- [ ] Add basic controls
- [ ] Add layout
- [ ] Add text
- [ ] Add images
- [ ] Add input handling
- [ ] Add focus/navigation foundation where required

### Test

- [ ] Test layout
- [ ] Test input
- [ ] Test lifecycle
- [ ] Test focus/navigation behavior when implemented
- [ ] Verify runtime UI can be disabled
- [ ] Verify runtime UI does not depend on Dear ImGui

### Architecture

- [ ] Keep runtime widget state independent from renderer backend
- [ ] Keep runtime UI data reusable by future editor tooling where appropriate
- [ ] Avoid coupling game-facing UI to editor-only systems

---

## Phase 28 — Events and Messaging

### Learn

- [ ] direct calls
- [ ] events
- [ ] messaging
- [ ] observer patterns

### Design

- [ ] Prefer direct calls when appropriate
- [ ] Define generic event mechanism only where needed
- [ ] Define event lifetime
- [ ] Define thread rules
- [ ] Avoid global event-bus dependency for everything

### Implement

- [ ] Add event mechanism when real use case exists

### Test

- [ ] Test subscription
- [ ] Test unsubscription
- [ ] Test lifetime behavior
- [ ] Test dispatch ordering where defined

---

## Phase 29 — Editor Foundation

### Learn

- [ ] immediate-mode UI
- [ ] retained-mode UI
- [ ] docking
- [ ] editor/runtime separation
- [ ] editor command systems

### Implement

- [ ] Create editor executable
- [ ] Keep editor separate from game executable
- [ ] Integrate editor UI rendering
- [ ] Add docking
- [ ] Add main menu
- [ ] Add toolbar
- [ ] Add viewport
- [ ] Add console
- [ ] Add diagnostics panel
- [ ] Add status bar
- [ ] Add editor command abstraction

### Dear ImGui Migration Policy

- [ ] Allow Dear ImGui developer panels during editor bootstrap
- [ ] Keep editor models independent from Dear ImGui
- [ ] Keep inspector/hierarchy/project state independent from widget implementation
- [ ] Allow editor panels to migrate incrementally to Signum UI
- [ ] Avoid a single all-at-once UI rewrite
- [ ] Preserve Phase 9.5 tooling during migration
- [ ] Remove individual Dear ImGui dependencies only when replacement functionality exists

### Test

- [ ] Launch editor
- [ ] Close editor cleanly
- [ ] Verify runtime library does not require editor
- [ ] Verify editor displays diagnostics
- [ ] Verify editor data/model behavior is not owned by Dear ImGui state

### Architecture

- [ ] Make editor consume engine/tooling APIs
- [ ] Avoid arbitrary access to private engine internals
- [ ] Keep editor state separate from presentation/widget implementation
- [ ] Keep editor-only dependencies out of game/runtime targets

---

## Phase 30 — Editor Scene Tooling

### Implement

- [ ] Add hierarchy
- [ ] Add inspector
- [ ] Add asset browser
- [ ] Add scene viewport
- [ ] Add game viewport
- [ ] Add selection model
- [ ] Add transform editing
- [ ] Connect inspector to metadata
- [ ] Support project-defined components in inspector
- [ ] Add error notifications

### Test

- [ ] Test selection
- [ ] Test transform edits
- [ ] Test reflected property editing
- [ ] Test project-defined component display

### Architecture

- [ ] Avoid hard-coded editor support for each gameplay component

---

## Phase 31 — Undo and Redo

### Learn

- [ ] command pattern
- [ ] transactions
- [ ] change tracking

### Implement

- [ ] Add editor command history
- [ ] Add undo
- [ ] Add redo
- [ ] Connect property edits
- [ ] Connect object creation
- [ ] Connect object deletion

### Test

- [ ] Test property undo
- [ ] Test property redo
- [ ] Test object creation undo
- [ ] Test object deletion undo
- [ ] Test long edit sequences

---

## Phase 32 — Prefabs and Templates

### Design

- [ ] Define prefab format
- [ ] Version prefab format
- [ ] Define prefab identity
- [ ] Define instance overrides
- [ ] Define nested prefab policy

### Implement

- [ ] Create prefab
- [ ] Instantiate prefab
- [ ] Edit prefab
- [ ] Apply overrides
- [ ] Revert overrides

### Test

- [ ] Test serialization
- [ ] Test overrides
- [ ] Test nested instances
- [ ] Test malformed prefab files

---

## Phase 33 — Project and Build System

### Design

- [ ] Define project format
- [ ] Version project format
- [ ] Define source layout
- [ ] Define asset layout
- [ ] Define project settings
- [ ] Define build configurations
- [ ] Define project module configuration
- [ ] Separate editor-only dependencies from runtime

### Implement

- [ ] Add project loader
- [ ] Add project creation
- [ ] Add project settings
- [ ] Build project game module
- [ ] Package runtime dependencies
- [ ] Export runnable game

### Test

- [ ] Create clean project
- [ ] Build clean project
- [ ] Run clean project
- [ ] Export clean project
- [ ] Test missing dependencies
- [ ] Test version mismatch

---

## Phase 34 — Headless Runtime and Tool Frontends

### Design

- [ ] Separate reusable engine startup from windowed application startup
- [ ] Define headless frontend requirements
- [ ] Define tooling frontend requirements
- [ ] Define renderer-without-presentation requirements
- [ ] Reuse Phase 5.5 application configuration instead of creating separate startup systems

### Implement

- [ ] Run reusable engine systems without window creation
- [ ] Run reusable engine systems without Vulkan where possible
- [ ] Allow renderer initialization without a presentation surface when supported
- [ ] Add tooling executable
- [ ] Add project validation commands
- [ ] Add asset processing commands
- [ ] Add headless asset cooking
- [ ] Add benchmark commands
- [ ] Add structured CLI diagnostics
- [ ] Add meaningful exit codes

### Test

- [ ] Run headless tests
- [ ] Test tooling failures
- [ ] Test exit codes
- [ ] Verify tools reuse engine modules
- [ ] Verify tools do not pull editor-only dependencies
- [ ] Verify non-rendering tools do not require Vulkan
- [ ] Verify renderer-only tooling does not require a presentation window where supported

### Architecture

- [ ] Keep frontend type separate from reusable engine subsystems
- [ ] Keep window ownership optional for reusable tools
- [ ] Preserve one shared diagnostics/error infrastructure

---

## Phase 34.5 — Large-World Spatial Foundation

### Learn

- [ ] floating-point precision at large coordinates
- [ ] local versus global coordinates
- [ ] floating origins
- [ ] hierarchical coordinate spaces
- [ ] reference frames
- [ ] double-precision world coordinates
- [ ] local high-precision simulation regions

### Design

- [ ] Define world-coordinate representation
- [ ] Define local-coordinate representation
- [ ] Define world-to-local conversion
- [ ] Define reference-frame ownership
- [ ] Define origin-shift policy if used
- [ ] Define transform-hierarchy interaction
- [ ] Define renderer coordinate contract
- [ ] Define physics coordinate contract
- [ ] Define networking coordinate contract
- [ ] Avoid assuming one flat coordinate space

### Implement

- [ ] Add global spatial representation when required
- [ ] Add local simulation/rendering coordinates
- [ ] Add reference-frame transforms
- [ ] Add origin shifting if selected
- [ ] Preserve stable object identity across reference-frame changes
- [ ] Integrate with physics when large-world physics requires it

### Test

- [ ] Test large coordinate values
- [ ] Test world/local conversion
- [ ] Test reference-frame transitions
- [ ] Test origin shifts
- [ ] Test transform stability
- [ ] Test renderer precision
- [ ] Test physics stability

### Architecture

- [ ] Keep large-world coordinates independent from streaming
- [ ] Keep procedural generation independent from absolute coordinate representation
- [ ] Do not make planets a core engine primitive
- [ ] Support ordinary small worlds without large-world overhead

---

## Phase 35 — Networking

### Learn

- [ ] networking transport
- [ ] client/server architecture
- [ ] authoritative simulation
- [ ] replication
- [ ] latency
- [ ] packet loss
- [ ] network security basics
- [ ] networked coordinate/reference-frame representation

### Design

- [ ] Keep networking optional
- [ ] Separate transport from gameplay replication
- [ ] Avoid assuming every project replicates entities
- [ ] Define generic messages
- [ ] Define connection lifecycle
- [ ] Define replication identity separately from local object addresses
- [ ] Define serialization boundary for network messages
- [ ] Use Phase 34.5 world/reference-frame contracts for large-world positions
- [ ] Avoid assuming one flat single-precision coordinate space
- [ ] Keep transport replaceable independently from replication policy

### Implement

- [ ] Add transport abstraction
- [ ] Add connections
- [ ] Add messages
- [ ] Add optional replication support
- [ ] Add reference-frame-aware spatial replication only when a project requires it

### Test

- [ ] Loopback tests
- [ ] Disconnect tests
- [ ] Malformed packet tests
- [ ] Latency simulation
- [ ] Packet-loss simulation
- [ ] Test spatial replication across reference-frame/origin changes when implemented

### Architecture

- [ ] Keep networking out of projects that do not use it
- [ ] Keep transport details out of gameplay state
- [ ] Keep large-world coordinate encoding consistent with Phase 34.5
- [ ] Avoid coupling networking to one ECS storage implementation

---

## Phase 36 — Streaming and Large Worlds

### Design

- [ ] Define generic streamable resources
- [ ] Define loading priorities
- [ ] Define memory budgets
- [ ] Define cancellation
- [ ] Define spatial streaming inputs using Phase 34.5 coordinate/reference-frame contracts
- [ ] Keep spatial streaming optional
- [ ] Avoid requiring chunk-based worlds
- [ ] Keep resource streaming independent from world partition strategy
- [ ] Define residency decisions separately from procedural generation

### Implement

- [ ] Add asynchronous load
- [ ] Add asynchronous unload
- [ ] Add streaming requests
- [ ] Add streaming priorities
- [ ] Add streaming cancellation
- [ ] Add memory-budget enforcement
- [ ] Add spatial streaming integration when a real large-world workload exists
- [ ] Integrate GPU residency budgets with Phase 6.5 memory information where appropriate

### Test

- [ ] Test load/unload
- [ ] Test cancellation
- [ ] Test memory limits
- [ ] Stress streaming
- [ ] Test shutdown during streaming
- [ ] Test streaming across large-world reference-frame transitions when applicable
- [ ] Test that non-spatial resources can use the same generic streaming infrastructure

### Architecture

- [ ] Keep streaming infrastructure independent from terrain, water, cloud, and other optional render-feature modules
- [ ] Let optional large-world rendering modules consume generic streaming APIs rather than own streaming-core policy
- [ ] Keep streaming separate from coordinate representation
- [ ] Keep streaming separate from procedural-generation algorithms
- [ ] Keep ordinary small projects free from large-world streaming requirements

---

## Phase 37 — Procedural Generation Capabilities

### Implement

- [ ] Add reusable seeded random utilities
- [ ] Add deterministic-generation utilities
- [ ] Support project-defined generators
- [ ] Support generation through task system
- [ ] Support generated resource creation
- [ ] Support CPU generation
- [ ] Support GPU compute generation when useful
- [ ] Support editor generation tools through extensions

### Test

- [ ] Test seed determinism
- [ ] Test generation cancellation
- [ ] Test concurrent deterministic behavior
- [ ] Benchmark generation utilities

### Architecture

- [ ] Keep procedural algorithms project-defined unless generally reusable
- [ ] Do not require procedural generation for normal projects
- [ ] Do not make chunks or planets core engine concepts

---

## Phase 38 — Optional Rendering Feature Modules

### Hardware Ray-Tracing Infrastructure

- [ ] Add optional acceleration-structure resource support
- [ ] Add optional ray-tracing pipeline support
- [ ] Add optional shader-binding-table support
- [ ] Add optional ray-tracing command/resource integration
- [ ] Integrate hardware ray-tracing resources with renderer-core lifetime/dependency tracking
- [ ] Integrate hardware ray-tracing work with render graph/frame graph

### Built-In Feature Modules

- [ ] Add optional PBR module
- [ ] Add optional shadow module
- [ ] Add optional parallax-mapping module
- [ ] Add optional SSS module
- [ ] Add optional SSGI module
- [ ] Add optional reflection module
- [ ] Add optional dynamic cubemap/reflection-probe module
- [ ] Add optional post-processing module
- [ ] Add optional HDR module
- [ ] Add optional temporal-effects module
- [ ] Add optional visibility module

### Ray-Traced Feature Modules

- [ ] Add optional ray-traced shadow module
- [ ] Add optional ray-traced reflection module
- [ ] Add optional ray-traced GI module

### Environment Feature Modules

- [ ] Add optional terrain-rendering module
- [ ] Add optional water-rendering module
- [ ] Add optional wetness module
- [ ] Add optional volumetric-cloud module
- [ ] Add optional terrain/cloud-shadow module

### Test

- [ ] Profile each major feature module
- [ ] Verify GPU synchronization for composed feature modules
- [ ] Track GPU memory per feature module
- [ ] Track frame-time regressions per feature module
- [ ] Disable each optional rendering feature without breaking unrelated rendering
- [ ] Compose multiple built-in and custom render features through the render graph
- [ ] Verify built-in features use only public renderer mechanisms available to custom features
- [ ] Compare CPU, GPU, memory, and resource use with each optional feature disabled
- [ ] Replace one built-in implementation without modifying unrelated renderer systems
- [ ] Verify render-graph resource dependencies and lifetimes across multiple features
- [ ] Verify ray-tracing infrastructure can exist without enabling any ray-traced effect module

### Architecture

- [ ] Keep every feature in this phase optional
- [ ] Keep PBR, shadows, SSS, SSGI, reflections, terrain, water, wetness, clouds, and ray-traced effects outside renderer-core
- [ ] Keep hardware ray-tracing infrastructure independent from specific ray-traced effects
- [ ] Allow developers to enable, disable, configure, replace, and extend built-in rendering modules
- [ ] Allow developers to implement equivalent or new features through the same public renderer mechanisms
- [ ] Avoid initializing or allocating resources for unused optional rendering modules

---

## Phase 39 — Editor Extensibility

### Implement

- [ ] Define editor extension API
- [ ] Add custom panels
- [ ] Add custom inspectors
- [ ] Add custom asset editors
- [ ] Add custom importers
- [ ] Add custom tools
- [ ] Add custom editor commands
- [ ] Add renderer debug extensions
- [ ] Define extension lifetime

### Test

- [ ] Load editor extension
- [ ] Handle invalid extension
- [ ] Handle extension failure
- [ ] Test extension version mismatch

---

## Phase 40 — API Compatibility

### Design

- [ ] Identify public runtime API
- [ ] Identify public editor/tooling API
- [ ] Identify internal API
- [ ] Version extension interfaces
- [ ] Version serialized formats
- [ ] Define deprecation policy
- [ ] Define migration policy

### ABI

- [ ] Decide whether stable binary plugins are required
- [ ] Investigate stable C ABI if required
- [ ] Avoid unstable C++ ABI promises
- [ ] Document supported compilers
- [ ] Document supported platforms

### Test

- [ ] API compatibility tests
- [ ] project migration tests
- [ ] asset migration tests
- [ ] plugin version tests

---

## Phase 41 — Profiling and Performance Scaling

### Implement

- [ ] Add CPU profiler
- [ ] Add GPU profiler
- [ ] Add frame breakdown
- [ ] Add scheduler profiling
- [ ] Add asset-loading profiling
- [ ] Add memory profiling
- [ ] Add renderer statistics
- [ ] Add per-render-feature CPU/GPU/resource statistics
- [ ] Add editor profiling views

### Optimize From Measurements

- [ ] cache-friendly layouts
- [ ] batching
- [ ] asynchronous loading
- [ ] parallel updates
- [ ] task graphs
- [ ] SIMD
- [ ] GPU compute
- [ ] GPU-driven rendering
- [ ] memory pools
- [ ] reduced synchronization
- [ ] incremental processing

### Test

- [ ] Maintain benchmark suite
- [ ] Track regressions
- [ ] Measure latency
- [ ] Measure throughput
- [ ] Measure peak memory
- [ ] Stress concurrency
- [ ] Verify unused optional rendering features impose no meaningful CPU/GPU/resource overhead
- [ ] Compare renderer-core baseline against enabled feature modules
- [ ] Measure feature activation/deactivation cost
- [ ] Track per-feature CPU, GPU, memory, descriptor, and resource usage

---

## Phase 41.5 — Experimental Systems and Backend Evolution

### Learn

- [ ] experimental-design methodology
- [ ] workload characterization
- [ ] A/B benchmarking
- [ ] statistical performance comparison
- [ ] hardware-specific optimization tradeoffs
- [ ] maintainability versus performance tradeoffs

### Baseline

- [ ] Record existing implementation performance
- [ ] Define representative workloads
- [ ] Define CPU measurements
- [ ] Define GPU measurements
- [ ] Define memory measurements
- [ ] Define latency measurements
- [ ] Define scaling measurements
- [ ] Define correctness criteria

### Experimental Renderer Work

- [ ] Evaluate newer GPU-driven techniques
- [ ] Evaluate visibility alternatives
- [ ] Evaluate bindless/resource-indexing strategies
- [ ] Evaluate descriptor-management alternatives
- [ ] Evaluate specialized geometry pipelines
- [ ] Evaluate asynchronous compute workloads
- [ ] Evaluate resource-residency strategies
- [ ] Evaluate custom GPU allocation strategies
- [ ] Keep experiments interchangeable with baselines

### Experimental Physics Work

- [ ] Profile Jolt-backed workloads
- [ ] Identify workload-specific limitations
- [ ] Prototype specialized Signum physics where justified
- [ ] Evaluate procedural-collision specialization
- [ ] Evaluate large-world broad-phase specialization
- [ ] Evaluate large-scale/reference-frame simulation
- [ ] Replace general-purpose physics components only when evidence supports it

### Experimental Audio Work

- [ ] Profile current audio backend
- [ ] Identify DSP or mixing limitations
- [ ] Prototype custom mixer when justified
- [ ] Prototype custom acoustic systems when justified
- [ ] Retain OS/device backend when replacing it provides no benefit

### Experimental Asset Work

- [ ] Profile decoding
- [ ] Profile cooking
- [ ] Profile runtime asset loading
- [ ] Evaluate custom compression/layout where useful
- [ ] Evaluate streaming-oriented runtime formats
- [ ] Keep commodity source decoders unless replacement has measurable value

### UI Evolution

- [ ] Profile Signum UI
- [ ] Identify remaining Dear ImGui dependencies
- [ ] Migrate remaining editor tooling when Signum UI provides equivalent capability
- [ ] Keep developer tooling usable during migration
- [ ] Remove Dear ImGui only when no longer needed

### Replacement Rules

- [ ] Keep baseline implementation available during experiments
- [ ] Verify behavioral equivalence where required
- [ ] Benchmark before and after
- [ ] Measure regression on weaker hardware
- [ ] Measure memory cost
- [ ] Measure complexity and maintenance cost
- [ ] Prefer specialized coexistence when full replacement is unnecessary
- [ ] Replace a dependency only when the new implementation has a concrete advantage
- [ ] Document why each replacement exists

### Architecture

- [ ] Third-party implementation must not define Signum architecture
- [ ] Experimental backends use normal Signum boundaries
- [ ] Baseline and experimental implementations can coexist
- [ ] Custom technology must remain measurable
- [ ] Avoid custom implementations whose only advantage is being custom

---

## Phase 42 — Production Hardening

### Correctness

- [ ] Expand unit tests
- [ ] Expand integration tests
- [ ] Run ASan regularly
- [ ] Run UBSan regularly
- [ ] Run Valgrind regularly
- [ ] Add ThreadSanitizer workflow where supported
- [ ] Add parser fuzz tests where useful
- [ ] Add property tests where useful
- [ ] Test partial initialization failures
- [ ] Test abnormal shutdown

### Compatibility

- [ ] Test Wayland
- [ ] Test X11 where supported
- [ ] Test multiple GPU vendors
- [ ] Test multiple supported drivers
- [ ] Test project migrations
- [ ] Test asset migrations

### Performance

- [ ] Track startup time
- [ ] Track runtime overhead
- [ ] Track frame time
- [ ] Track memory usage
- [ ] Track asset load time

### Release

- [ ] Audit dependencies
- [ ] Audit licenses
- [ ] Package editor
- [ ] Package runtime
- [ ] Package developer SDK
- [ ] Document supported toolchain
- [ ] Test clean-machine install
- [ ] Test clean project creation
- [ ] Test clean project build
- [ ] Test clean project export

---

## Long-Term Architecture

### Rendering Principles

- [ ] Engine provides mechanisms, built-in modules provide implementations, developers provide specialization
- [ ] Renderer-core does not depend on optional rendering modules
- [ ] Built-in rendering modules use the same public mechanisms available to developer-defined features
- [ ] Optional rendering modules can be enabled, disabled, configured, replaced, and extended
- [ ] Unused optional rendering modules impose no meaningful CPU, GPU, memory, descriptor, or resource overhead
- [ ] Replacing one rendering implementation does not require modifying unrelated renderer systems
- [ ] Experimental rendering techniques remain measurable against a baseline

### Engine Internals

- [ ] Core
- [ ] Platform
- [ ] Memory
- [ ] GPU memory and transfer infrastructure
- [ ] Job system
- [ ] Filesystem
- [ ] Virtual filesystem/mount infrastructure
- [ ] Logging
- [ ] Errors
- [ ] Diagnostics
- [ ] Asset infrastructure
- [ ] Asset cooking/runtime formats
- [ ] Renderer backend/RHI
- [ ] Shader/pipeline infrastructure
- [ ] Descriptor/binding infrastructure
- [ ] Material infrastructure
- [ ] Render graph/frame graph
- [ ] Render-feature registration infrastructure
- [ ] Custom render-pass registration infrastructure
- [ ] GPU resource lifetime/dependency tracking
- [ ] Hardware ray-tracing infrastructure
- [ ] Large-world spatial/reference-frame infrastructure

### Runtime APIs

- [ ] World
- [ ] Scene
- [ ] Entities
- [ ] Components
- [ ] Assets
- [ ] Resources
- [ ] Input
- [ ] Timing
- [ ] Rendering
- [ ] Physics
- [ ] Audio
- [ ] Runtime UI
- [ ] Events
- [ ] Serialization
- [ ] Optional networking

### Replaceable / Experimental Backends

- [ ] Renderer implementation boundaries
- [ ] Physics backend
- [ ] Audio backend
- [ ] Source-asset decoders/importers
- [ ] Developer/editor UI implementation
- [ ] Baseline implementations remain available while experimental alternatives are measured
- [ ] Replace implementations only for concrete capability, performance, scalability, or maintenance benefits

### Extension APIs

- [ ] Runtime modules
- [ ] Custom systems
- [ ] Custom components
- [ ] Custom asset types
- [ ] Custom importers
- [ ] Renderer extensions
- [ ] Custom render features
- [ ] Custom render passes
- [ ] Custom shaders/materials
- [ ] Replaceable rendering modules
- [ ] Editor extensions
- [ ] Custom tools

### Editor

- [ ] Viewport
- [ ] Hierarchy
- [ ] Inspector
- [ ] Asset browser
- [ ] Console
- [ ] Project settings
- [ ] Undo/redo
- [ ] Profiling
- [ ] Debugging
- [ ] Build/export
- [ ] Extension panels

### Game Project

- [ ] Project-defined C++ code
- [ ] Project-defined components
- [ ] Project-defined systems
- [ ] Project-defined assets
- [ ] Optional project modules
- [ ] Optional scripting

---

## Current Work

- [x] Application foundation
- [x] GLFW lifetime foundation
- [x] Window foundation
- [x] Structured `EngineError`
- [x] Error codes
- [x] Error subsystems
- [x] Native GLFW errors
- [x] Operation context
- [x] Source locations
- [x] Diagnostic formatter
- [x] GoogleTest migration
- [x] Diagnostic formatter tests
- [x] ASan/UBSan test workflow
- [x] Clang-Tidy workflow
- [x] Valgrind workflow
- [x] Fatal error reporting
- [x] Graceful fatal shutdown
- [x] Recoverable error policy
- [ ] Exception/programmer-error policy
- [ ] Verify clean application shutdown
- [x] Vulkan instance foundation
- [x] GPU device foundation
- [x] Swapchain support discovery
- [x] Surface-format selection
