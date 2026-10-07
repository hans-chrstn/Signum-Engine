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
- [x] Verify clean shutdown

### Learn

- [x] RAII
- [x] ownership
- [x] constructor/destructor lifetime relationships
- [x] non-copyable resource owners
- [x] application composition root
- [x] process lifetime
- [x] subsystem lifetime

### Test

- [x] Verify Window destruction before GLFW termination
- [x] Verify normal window-close path
- [x] Verify repeated startup and shutdown

### Architecture

- [x] Keep `Application` as orchestration
- [x] Avoid turning `Application` into a global subsystem container
- [x] Keep platform lifecycle ownership explicit
- [x] Keep subsystem construction and lifetime policy behind composition-root boundaries
- [ ] Avoid making the current windowed frontend a requirement for reusable engine subsystems

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
- [x] Avoid generic catch-all handling that discards context

### Learn

- [x] exception propagation
- [x] `std::optional`
- [x] `std::source_location`
- [x] exception boundaries
- [ ] exceptions versus explicit result types
- [x] runtime errors versus programmer errors

### Test

- [x] Test error codes
- [x] Test error messages
- [x] Test native error preservation
- [x] Test optional native context
- [x] Test subsystem classification
- [x] Test operation context
- [x] Test source location

### Architecture

- [x] Keep error storage separate from formatting
- [x] Keep error formatting separate from reporting
- [x] Keep recoverability separate from error type
- [x] Keep native API information intact
- [x] Keep subsystem-specific native failures as context rather than forcing backend details into engine-wide error APIs
- [ ] Allow exception and explicit-result policies to evolve without duplicating the engine error model

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
- [x] Prevent Vulkan instance/surface ownership details from defining future backend-independent renderer contracts

---

## Phase 4 — GPU Device Foundation

### Learn

- [x] physical devices
- [x] logical devices
- [x] queue families
- [x] queues
- [x] device capabilities
- [x] feature negotiation
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
- [ ] Keep queue roles and requests extensible beyond the initial graphics/presentation pair
- [x] Avoid exposing physical queue topology as a permanent higher-level renderer contract

---

## Phase 5 — Swapchain and Presentation

### Learn

- [x] swapchains
- [x] surface formats
- [x] presentation modes
- [x] presentation preference versus Vulkan presentation mode
- [x] FIFO_LATEST_READY behavior and capability requirements
- [x] image views
- [x] swapchain recreation

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
- [ ] Keep presentation optional so offscreen, headless, and non-presenting renderer frontends remain possible
- [x] Keep swapchain-dependent resources distinguishable from device-lifetime renderer resources

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
- [x] Treat current cardinalities, queue roles, frame counts, and feature chains as implementation choices rather than permanent public contracts unless explicitly required

---

## Phase 5.75 — Renderer Foundation Scalability

### Learn

- [x] composition-root boundaries
- [x] renderer runtime ownership
- [x] selected-resource invariants
- [x] frame-resource ownership boundaries
- [x] borrowed views versus owned containers
- [x] swapchain recreation ownership requirements
- [x] scalable queue-request modeling
- [x] incremental abstraction versus speculative abstraction

### Implement

- [x] Define the renderer runtime ownership boundary
- [x] Prevent `Application` from becoming the permanent owner of per-frame Vulkan resources
- [x] Keep `Application` responsible for orchestration rather than rendering internals
- [x] Define where device, swapchain, command, and frame resources belong
- [x] Define per-frame resource ownership before adding synchronization
- [x] Keep command-buffer lifetime tied to its command pool
- [x] Keep command-buffer allocation and recycling strategy replaceable without changing frame-resource or renderer callers
- [x] Allow per-frame command resources to evolve from one primary command buffer to multiple primary or secondary command buffers without redesigning frame ownership
- [x] Define a scalable command-pool ownership model
- [x] Define swapchain image and image-view access needed by rendering
- [x] Expose borrowed swapchain resource views without transferring ownership
- [x] Preserve selected physical-device queue-family invariants
- [x] Encode selected physical-device guarantees in runtime types
- [x] Remove hardcoded non-resizable window policy before swapchain recreation work
- [x] Prepare the application loop for non-blocking rendering
- [x] Keep queue-request structures extensible for future compute and transfer queues
- [x] Keep logical-device feature negotiation extensible for additional feature chains

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
- [x] Keep non-frame-scoped GPU resources independently ownable instead of forcing all GPU state into frame ownership
- [x] Avoid global command buffers, synchronization objects, and frame state
- [x] Support multiple frames in flight without redesigning ownership
- [x] Allow future graphics, compute, and transfer command pools
- [x] Keep command-pool ownership independent from command-buffer recording policy
- [x] Keep command-pool and command-buffer mechanisms reusable across graphics, compute, and transfer roles
- [x] Do not make one command buffer per frame a permanent renderer contract
- [x] Allow command-buffer allocation to evolve from individual to batched or pool-managed strategies without changing unrelated renderer code
- [x] Keep swapchain ownership separate from frame ownership
- [x] Keep swapchain resource access non-owning
- [x] Keep physical-device discovery separate from selected-device guarantees
- [x] Keep supported capabilities separate from requested and enabled features
- [x] Keep queue planning separate from logical-device creation
- [x] Allow queue requests to evolve without redesigning `VulkanDevice`
- [x] Allow additional Vulkan feature structures without redesigning feature negotiation
- [ ] Avoid exposing Vulkan implementation details to game-facing APIs
- [x] Avoid introducing generic managers or registries without a concrete requirement
- [x] Prefer focused abstractions over demo-specific shortcuts
- [x] Add abstractions only when a concrete requirement exists
- [x] Preserve explicit RAII ownership and dependency-ordered destruction

---

## Phase 6 — Basic Rendering

### Learn

- [x] command pools
- [x] command buffers
- [x] synchronization
- [x] frame ownership
- [x] acquire-submit-present flow
- [x] dynamic rendering

### Implement

- [x] Create command pool
- [x] Create initial per-frame primary command buffers without making one-buffer-per-frame a permanent abstraction
- [x] Create initial frame synchronization objects without making one fixed synchronization layout a permanent renderer contract
- [x] Implement frame loop
- [x] Use Vulkan dynamic rendering
- [x] Poll platform events
- [x] Clear screen
- [x] Render triangle
- [x] Handle frame errors

### Test

- [x] Run validation layers cleanly
- [x] Run extended frame test
- [x] Resize during rendering
- [x] Verify clean renderer shutdown
- [x] Run sanitizers

### Architecture

- [x] Keep rendering loop independent from gameplay
- [x] Keep synchronization details inside rendering subsystem
- [x] Keep command-buffer allocation, recording, and recycling policy inside rendering boundaries
- [ ] Keep synchronization ownership extensible for additional queues and submission paths
- [x] Prefer dynamic rendering over legacy render-pass/framebuffer objects unless a concrete compatibility need requires them

---

## Phase 6.5 — GPU Memory, Transfers, and Lifetime

### Learn

- [x] Vulkan memory heaps
- [x] Vulkan memory types
- [x] host-visible memory
- [x] device-local memory
- [x] coherent versus non-coherent memory
- [x] memory requirements and alignment
- [x] suballocation
- [x] dedicated allocations
- [x] staging transfers
- [x] upload paths
- [x] readback paths
- [x] deferred destruction
- [x] memory budgets

### Design

- [x] Define GPU allocation responsibilities
- [x] Define buffer-memory ownership
- [x] Define image-memory ownership
- [x] Define upload-memory policy
- [x] Define readback-memory policy
- [x] Define persistent-mapping policy
- [x] Define resource-retirement policy
- [x] Define allocation diagnostics
- [x] Keep allocation implementation replaceable
- [x] Avoid exposing allocator implementation to normal renderer callers

### Implement

- [x] Integrate Vulkan Memory Allocator (VMA) as the initial Vulkan allocation backend
- [x] Add GPU memory allocation foundation
- [x] Add memory-type selection policy through the GPU allocation abstraction
- [x] Support buffer allocation
- [x] Support image allocation
- [x] Add staging-upload path
- [x] Add mapped-upload path where appropriate
- [x] Add readback path when needed
- [x] Respect Vulkan resource and allocation alignment requirements
- [x] Add memory-budget queries when supported
- [x] Add basic allocation statistics
- [ ] Add deferred resource retirement when frame lifetime requires it

### Test

- [x] Test memory-type selection
- [ ] Test alignment calculations
- [x] Test buffer allocation lifecycle
- [x] Test image allocation lifecycle
- [x] Test upload correctness
- [ ] Test partial allocation failure
- [ ] Test deferred retirement
- [ ] Run Vulkan validation
- [ ] Run sanitizers

### Architecture

- [x] Keep VMA behind engine-owned GPU allocation/resource abstractions
- [x] Do not expose VMA types to normal renderer or game-facing APIs
- [x] Keep the allocation backend replaceable without redesigning GPU resource identity or callers
- [ ] Separate GPU resource identity from memory allocation
- [x] Keep allocation backend-specific
- [x] Keep allocation policy replaceable without changing resource identity or normal renderer callers
- [ ] Keep future streaming requirements possible
- [ ] Keep future memory-budget enforcement possible
- [ ] Keep future specialized allocator experiments possible
- [ ] Allow dedicated, suballocated, pooled, and externally managed allocations where concrete workloads justify them
- [x] Do not build a complex allocator before measurements justify it

---

## Phase 6.75 — Foundation Hardening

### Learn

- [x] Review preconditions, postconditions, and invariants
- [x] Review programmer errors versus runtime failures
- [x] Review exception safety for reusable engine operations
- [x] Review Vulkan queue host-synchronization requirements
- [x] Review Vulkan queue-family resource ownership
- [x] Review integer narrowing at Vulkan API boundaries
- [x] Review RAII move semantics for GPU resources
- [x] Review shutdown and destructor error handling

### Design

- [x] Define consistent rules for `Precondition`, `Invariant`, and `Postcondition`
- [x] Define when failures use `EngineError` versus `failAssertion`
- [x] Define reusable byte-range validation
- [x] Define checked Vulkan count/index conversion policy
- [x] Define Vulkan buffer usage introspection
- [x] Define immediate-submission threading and reuse policy
- [x] Define queue submission ownership and synchronization policy
- [x] Define requirements for future dedicated transfer queues
- [x] Define queue-family ownership-transfer policy
- [x] Define swapchain recreation behavior
- [x] Define minimized-window and zero-framebuffer behavior
- [x] Define `VK_ERROR_OUT_OF_DATE_KHR` handling
- [x] Define `VK_SUBOPTIMAL_KHR` handling
- [x] Define GPU resource move-semantics policy
- [ ] Define application-level exception boundary
- [ ] Define scalable error-code metadata handling
- [ ] Define consistent warnings-as-errors policy
- [ ] Define reusable Vulkan integration-test infrastructure

### Implement

- [x] Audit existing `failAssertion` calls for correct assertion type
- [x] Add missing preconditions to backend-facing operations
- [x] Add invariants for internal assumptions that must always hold
- [x] Add postconditions where successful operations establish required state
- [x] Remove redundant contract checks already guaranteed by lower-level ownership
- [x] Add reusable overflow-safe byte-range validation
- [x] Replace duplicated buffer range calculations with the shared helper
- [x] Add checked conversion for Vulkan count fields
- [x] Replace unchecked `size_t` to Vulkan count conversions
- [x] Store Vulkan usage flags in `VulkanBuffer`
- [x] Add backend-local buffer usage access
- [x] Validate `VK_BUFFER_USAGE_TRANSFER_SRC_BIT` before buffer copies
- [x] Validate `VK_BUFFER_USAGE_TRANSFER_DST_BIT` before buffer copies
- [x] Reject invalid overlapping copies when source and destination are the same buffer
- [x] Harden `VulkanImmediateSubmission` reuse
- [x] Prevent invalid or recursive immediate submission
- [x] Document immediate-submission thread-safety requirements
- [x] Keep staging resources alive until submitted GPU work finishes
- [x] Keep immediate submission generic instead of upload-specific
- [x] Keep transfer recording separate from transfer submission
- [x] Keep graphics-queue transfers as the current baseline
- [ ] Prevent adding a dedicated transfer queue without resource ownership handling
- [x] Validate selected queue-family capacity
- [x] Verify retrieved queues satisfy selected-device guarantees
- [x] Assert frame command-buffer availability before using `.front()`
- [ ] Harden swapchain selection against invalid or empty inputs
- [x] Handle zero-sized framebuffer state
- [x] Add swapchain recreation after acquire-time out-of-date results
- [x] Add swapchain recreation after present-time out-of-date results
- [x] Handle suboptimal swapchains consistently
- [x] Recreate swapchain-dependent resources in dependency order
- [x] Re-query framebuffer and surface capabilities during recreation
- [x] Preserve valid frame-fence state through swapchain recreation
- [ ] Audit Vulkan wrappers for owned-versus-borrowed lifetime correctness
- [ ] Audit destructors for safe partially constructed or empty state
- [ ] Review GPU resource wrappers for move support required by future containers
- [ ] Add move semantics only where a concrete ownership requirement exists
- [x] Verify successful VMA buffer creation produces valid buffer/allocation state
- [x] Verify successful VMA image creation produces valid image/allocation state
- [x] Verify command-buffer allocation returns the requested number of handles
- [ ] Widen `Core::Error::Code` storage beyond `std::uint8_t`
- [ ] Prevent error-code string mappings from silently becoming incomplete
- [ ] Prevent error-code subsystem mappings from silently becoming incomplete
- [ ] Add final handling for unexpected `std::exception`
- [ ] Add final handling for unknown exceptions if needed
- [ ] Keep assertion failures separate from recoverable exception handling
- [ ] Document failures that cannot be propagated from destructors
- [ ] Add reusable Vulkan integration-test setup for instance, device, queue, and allocator
- [ ] Add immediate-submission support to Vulkan integration-test infrastructure
- [ ] Remove duplicated Vulkan setup from buffer and image integration tests
- [ ] Remove unused `Window::waitEvents()` if no longer needed
- [ ] Remove empty `application_configuration.cpp`
- [ ] Remove stale empty namespaces, aliases, and includes
- [ ] Make warnings-as-errors consistent between local builds and CI
- [ ] Keep CMake source lists synchronized with renderer files
- [ ] Keep Doxygen ownership, lifetime, and contract documentation current

### Test

- [x] Test byte-range validation
- [x] Test exact-end byte ranges
- [x] Test zero-size ranges
- [x] Test invalid offsets
- [x] Test overflow-resistant range checks
- [x] Test checked Vulkan count conversion
- [ ] Test buffer usage tracking
- [x] Test invalid transfer source usage
- [x] Test invalid transfer destination usage
- [x] Test overlapping same-buffer copies
- [x] Test immediate submission callback execution
- [x] Test repeated immediate submissions
- [x] Test immediate command-pool reuse
- [x] Test immediate fence reuse
- [x] Test immediate submission recovery after recording failure where supported
- [x] Test complete staging upload
- [ ] Test staging upload with destination offsets
- [ ] Test staging upload range boundaries
- [x] Test upload correctness through readback
- [x] Test CPU -> upload -> device -> readback -> CPU round trip
- [ ] Test partial transfer correctness
- [ ] Test minimized-window behavior
- [x] Test repeated window resizing
- [x] Test swapchain recreation
- [ ] Test acquire-time out-of-date handling
- [ ] Test present-time out-of-date handling where reproducible
- [ ] Test frame-resource reuse after early returns
- [ ] Test swapchain-dependent destruction ordering
- [x] Add representative assertion death tests
- [ ] Test every error code has a valid string
- [ ] Test every error code has a valid subsystem
- [ ] Test error metadata completeness
- [x] Run clean Debug build
- [x] Run unit tests
- [x] Run Vulkan integration tests
- [x] Run ASan/UBSan
- [x] Run Valgrind
- [x] Run Clang-Tidy
- [x] Run formatting checks
- [ ] Run warnings-as-errors build
- [ ] Run Vulkan validation cleanly
- [ ] Generate Doxygen without warnings
- [ ] Verify CI from a clean checkout

### Architecture

- [ ] Preserve explicit RAII ownership
- [ ] Preserve dependency-ordered destruction
- [ ] Keep programmer errors separate from runtime failures
- [ ] Keep native runtime failures out of assertions
- [ ] Keep assertions out of normal control flow
- [ ] Detect invalid internal state close to where it originates
- [ ] Prefer small correctness helpers over repeated subtle logic
- [ ] Keep Vulkan-specific validity checks inside the Vulkan backend
- [ ] Keep VMA details out of normal renderer-facing APIs
- [ ] Keep transfer recording independent from submission policy
- [ ] Keep upload policy independent from transfer-command construction
- [ ] Keep synchronous immediate submission replaceable by future asynchronous paths
- [ ] Keep future streaming uploads possible
- [ ] Keep graphics, compute, and transfer queue roles extensible
- [ ] Do not add dedicated transfer queues without queue-family ownership support
- [ ] Do not assume queue submission will permanently remain single-threaded
- [ ] Keep swapchain recreation inside renderer/presentation ownership
- [ ] Keep frame-resource identity separate from swapchain-image identity
- [ ] Keep resource ownership valid when resources move between containers
- [ ] Keep error metadata scalable as error counts grow
- [ ] Preserve one clear runtime exception boundary
- [ ] Keep assertion reporting separate from runtime error reporting
- [ ] Keep deterministic logic in unit tests
- [ ] Keep Vulkan driver and lifetime behavior in integration tests
- [ ] Reuse Vulkan integration-test infrastructure
- [ ] Keep test helpers out of production architecture
- [ ] Avoid generic managers, service locators, or registries without a concrete need
- [ ] Avoid introducing the renderer abstraction before Phase 12

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
- [ ] Render first simple 3D reference object
- [ ] Render multiple objects
- [ ] Scale the reference rendering workload from one 3D object to multiple independently transformed objects
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
- [ ] Use a simple sphere, star, or planet-like mesh as reference content without adding celestial concepts to renderer APIs
- [ ] Increase reference object counts to expose resource, transform, synchronization, and rendering bugs
- [ ] Run Vulkan validation
- [ ] Run sanitizers

### Architecture

- [ ] Keep runtime object identity separate from GPU resource identity
- [ ] Keep GPU allocation details behind Phase 6.5 memory mechanisms
- [ ] Avoid raw Vulkan ownership in game entities
- [ ] Keep resource, shader, pipeline, binding, and material mechanisms independent from optional rendering features
- [ ] Keep core GPU resource APIs usable by built-in and developer-defined render features
- [ ] Avoid baking PBR, shadow, terrain, water, ray-tracing-effect, or other optional-feature semantics into renderer-core resource types
- [ ] Do not make vertex/index-buffer meshes the only renderable representation
- [ ] Keep renderer resource mechanisms usable by mesh, procedural, indirect, compute-generated, and future GPU-driven workloads
- [ ] Keep reference-scene concepts such as stars, planets, systems, and galaxies outside renderer-core types

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
- [ ] Keep log sinks replaceable and independently extensible
- [ ] Keep structured log and diagnostic data independent from sink presentation

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
- [ ] Display active renderable counts and draw/submission statistics when available
- [ ] Add debug visualization toggle infrastructure

### Test

- [ ] Launch with developer UI enabled
- [ ] Launch with developer UI disabled
- [ ] Toggle developer UI at runtime
- [ ] Verify diagnostics appear in log viewer
- [ ] Verify frame metrics update correctly
- [ ] Verify renderer statistics remain usable while scaling the current reference rendering workload
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
- [ ] Add repeatable reference-workload configurations for correctness and performance testing

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
- [ ] Record an initial multi-object rendering reference baseline

### Architecture

- [ ] Keep deterministic logic in unit tests
- [ ] Keep driver/OS/hardware behavior in integration tests
- [ ] Keep benchmarks separate from correctness tests
- [ ] Make performance comparisons reproducible enough to guide later optimization
- [ ] Keep reference workloads outside engine-core behavior and reusable as regression inputs as later phases add capabilities

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
- [ ] Navigate the current 3D reference workload with test-side camera controls built from engine input and timing APIs
- [ ] Test rapid camera movement, focus changes, resizing, and long-running movement against the reference workload

### Architecture

- [ ] Keep GLFW types out of engine-facing input API
- [ ] Keep actions such as Jump/Shoot project-defined
- [ ] Keep input-device/backend translation replaceable without changing gameplay-facing action/state contracts
- [ ] Avoid assuming one window, keyboard/mouse-only input, or one platform event source in engine-facing APIs

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
- [ ] Run the existing 3D reference workload entirely through backend-independent renderer APIs
- [ ] Compare reference-workload correctness and basic performance with the pre-abstraction Vulkan path
- [ ] Benchmark abstraction where useful

### Architecture

- [ ] Keep renderer mechanisms separate from built-in feature implementations and project-specific extensions
- [ ] Keep renderer-core focused on reusable mechanisms rather than specific rendering effects
- [ ] Keep renderer-core independent from optional rendering modules
- [ ] Require optional rendering modules to depend on renderer-core, never the reverse
- [ ] Give built-in render features no privileged architectural shortcuts unavailable to custom features
- [ ] Allow built-in rendering modules to be enabled, disabled, configured, replaced, and extended
- [ ] Keep game rendering API independent from Vulkan
- [ ] Keep explicit low-level backend access for advanced extensions
- [ ] Keep unused optional render features uninitialized and unallocated
- [ ] Avoid freezing Vulkan-specific queue counts, command-buffer layouts, descriptor strategies, or submission topology into backend-independent APIs
- [ ] Let backend implementations expose specialized capabilities without forcing every backend into a least-common-denominator interface
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
- [ ] Repeatedly create, destroy, and reuse reference-workload resources to exercise invalid and stale-handle behavior

### Architecture

- [ ] Keep logical identity independent from memory address and backend-native handle values
- [ ] Keep handle representation replaceable without changing resource ownership semantics
- [ ] Avoid requiring globally persistent identity for temporary or representation-local resources that do not need it

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
- [ ] Run the existing reference rendering workload through the render graph and verify equivalent output
- [ ] Scale the reference workload while validating generated dependencies, lifetimes, and synchronization
- [ ] Test unused-pass removal when implemented

### Architecture

- [ ] Built-in and external render features use the same graph contracts
- [ ] Render graph depends on renderer mechanisms, not individual effects
- [ ] Do not encode PBR, shadows, terrain, or other feature semantics into graph core
- [ ] Keep synchronization details inside renderer/backend layers
- [ ] Keep graph queue declarations extensible beyond graphics-only execution
- [ ] Do not couple pass declarations to the current physical queue topology
- [ ] Preserve specialized backend scheduling paths where they outperform a generic path without changing graph semantics

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
- [ ] Define logical world identity independently from one physical storage model
- [ ] Allow authoring/scene representation to differ from optimized runtime representations

### Implement

- [ ] Add world lifecycle
- [ ] Add scene lifecycle
- [ ] Add transform type
- [ ] Add object creation
- [ ] Add object destruction
- [ ] Add hierarchy support when needed
- [ ] Connect world data to renderer through defined contracts
- [ ] Build the first reusable reference scene from generic World, Scene, Transform, and renderer mechanisms

### Test

- [ ] Test creation
- [ ] Test destruction
- [ ] Test transforms
- [ ] Test hierarchy changes
- [ ] Test invalid references
- [ ] Build a reference scene containing one project/test-defined celestial body using only generic world, transform, and renderer contracts
- [ ] Expand the reference scene to a small static star-and-bodies arrangement as hierarchy and transform support becomes available
- [ ] Verify the same world mechanisms represent non-celestial scenes without special cases

### Architecture

- [ ] Keep gameplay types project-defined
- [ ] Keep scene ownership independent from renderer ownership
- [ ] Keep World and Scene APIs independent from a mandatory ECS, object hierarchy, or renderer storage layout
- [ ] Allow specialized subsystem representations to coexist without making one subsystem's storage the definition of the world
- [ ] Keep star, planet, moon, orbit, solar-system, and galaxy semantics outside engine-core world types

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
- [ ] Benchmark archetype, sparse-set, and other justified storage alternatives instead of assuming one layout is universally optimal
- [ ] Keep entity identity separable from whichever component-storage strategy is selected

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
- [ ] Represent the growing reference-scene bodies with project-defined components and scale entity count
- [ ] Benchmark iteration using both small and larger reference-scene populations
- [ ] Benchmark iteration

### Architecture

- [ ] Do not hard-code Player
- [ ] Do not hard-code Enemy
- [ ] Do not hard-code Card
- [ ] Do not hard-code Weapon
- [ ] Do not hard-code Planet
- [ ] Do not hard-code gameplay components
- [ ] Treat ECS/entity-component storage as one world representation mechanism rather than the definition of every engine subsystem
- [ ] Do not require rendering, physics, audio, navigation, procedural data, or editor metadata to become ECS-shaped
- [ ] Keep component semantics independent from one physical storage layout where practical

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
- [ ] Round-trip the growing reference scene and verify identities, transforms, hierarchy, and project-defined data survive serialization

### Architecture

- [ ] Keep serialized schemas independent from in-memory layout and module addresses
- [ ] Allow storage/backends to define serialization adapters without exposing their private physical representation
- [ ] Keep migration/version policy usable across editor state, world data, modules, and future reload workflows

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
- [ ] Define opt-in reflection boundaries rather than attempting to reflect all C++
- [ ] Define property attributes as metadata rather than hard-coded inspector behavior
- [ ] Minimize repeated registration
- [ ] Minimize unnecessary macros
- [ ] Keep metadata inspectable
- [ ] Keep the reflection API independent from whether metadata comes from manual registration, generated code, compiler tooling, or future language facilities

### Implement

- [ ] Register engine value types
- [ ] Register engine components
- [ ] Expose type metadata
- [ ] Expose property metadata
- [ ] Connect metadata to serialization
- [ ] Support one project-defined component
- [ ] Add reflected property attributes only as real inspector/serialization needs appear
- [ ] Add metadata generation only after manual registration becomes meaningfully repetitive

### Test

- [ ] Test type lookup
- [ ] Test property lookup
- [ ] Test duplicate type IDs
- [ ] Test duplicate property IDs
- [ ] Test metadata serialization
- [ ] Test missing/invalid metadata without requiring unrelated systems to fail
- [ ] Verify reflected IDs remain meaningful across executions and module reloads
- [ ] Reflect project-defined reference-scene types without adding celestial concepts to engine metadata

### Architecture

- [ ] Keep reflection incremental and independently useful at each stage
- [ ] Keep reflection opt-in for explicitly exposed engine/project types
- [ ] Do not make reflection the universal communication mechanism between engine subsystems
- [ ] Keep strongly typed C++ APIs as the default where reflection is unnecessary
- [ ] Keep internal renderer/backend types unreflected unless a concrete tooling requirement justifies exposure
- [ ] Keep consumers dependent on the metadata API rather than one metadata-generation mechanism

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
- [ ] Add runtime resource management mechanism
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
- [ ] Share geometry/material resources across many reference-scene objects and verify resource identity and dependency behavior
- [ ] Repeatedly load and unload reference-scene resources while the scene remains valid

### Architecture

- [ ] Keep shader/material asset formats independent from ownership by any built-in render feature
- [ ] Let built-in and custom render features consume the same asset/resource mechanisms
- [ ] Avoid one global resource manager becoming the permanent owner of unrelated resource domains
- [ ] Keep resource lifetime, loading, caching, and residency policies replaceable behind asset/resource contracts

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
- [ ] Prefer engine-native cooked textures at runtime
- [ ] Do not rewrite commodity codecs without a measured reason

### Test

- [ ] Test deterministic cooking
- [ ] Test cache hits
- [ ] Test cache invalidation
- [ ] Test importer-version changes
- [ ] Test corrupted cooked resources
- [ ] Test unsupported format versions
- [ ] Test source-file changes rebuild dependent resources
- [ ] Cook and load reference-scene assets through runtime formats and compare behavior with the imported source path

### Architecture

- [ ] Source formats are editor/import concerns
- [ ] Runtime formats are engine-controlled
- [ ] Runtime resource layout may evolve independently from source file format
- [ ] Custom runtime formats target runtime requirements rather than novelty

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
- [ ] Define explicit game-module interface and ownership boundary
- [ ] Define game-module load, unload, and failure lifecycle
- [ ] Define how callbacks, systems, metadata, and module-owned objects are invalidated before unload
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

- [ ] Use C++23 as the first-class initial game-code language
- [ ] Separate engine/editor code from ordinary project game code
- [ ] Allow ordinary project-defined C++ types
- [ ] Allow project-defined components
- [ ] Allow project-defined systems
- [ ] Define a narrow explicit runtime/game-module API boundary instead of exposing arbitrary engine internals
- [ ] Keep game-module lifetime explicit
- [ ] Ensure engine-owned resources cannot depend on code or objects after their game module unloads
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
- [ ] Keep basic C++ gameplay usable before advanced reflection or state-preserving reload exists

### Implement

- [ ] Create separately buildable project/game module
- [ ] Produce a dynamically loadable game library on supported platforms
- [ ] Load and unload the game module through the module boundary
- [ ] Expose runtime APIs
- [ ] Add project-defined component example
- [ ] Add project-defined system example
- [ ] Add project-defined render feature example when renderer extension API exists
- [ ] Run project without modifying engine core
- [ ] Move the growing celestial reference workload into ordinary project/game-module code
- [ ] Implement simple project-defined orbital motion through generic timing and transform APIs without engine celestial types

### Test

- [ ] Build standalone project module
- [ ] Load and unload a project module repeatedly without leaking module-owned resources
- [ ] Run project-defined behavior
- [ ] Verify ordinary game-code changes do not require rebuilding unrelated engine/editor systems
- [ ] Verify project does not need Vulkan calls
- [ ] Verify project does not need GLFW calls
- [ ] Verify project-defined render feature does not require renderer-core modification
- [ ] Verify project-defined render feature can use the same render graph, resource, shader, pipeline, binding, and material mechanisms as built-in features
- [ ] Verify project runs without scripting runtime
- [ ] Build and run a single reference star system entirely through game-facing APIs
- [ ] Expand the reference project to multiple systems without modifying engine core

### Architecture

- [ ] Keep C++ gameplay first-class without making scripting mandatory
- [ ] Keep game-module interfaces explicit, versionable, and smaller than arbitrary engine internals
- [ ] Prefer handles, IDs, opaque ownership, and explicit data contracts where raw C++ layout coupling would make module reload unsafe
- [ ] Do not require the editor or engine binaries to restart for ordinary game-module rebuild/reload workflows
- [ ] Keep module/reload capability independent from one reflection-generation implementation
- [ ] Keep reference-workload gameplay and celestial semantics project-owned even when they are used to stress engine systems

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
- [ ] Use shared metadata and runtime contracts where practical
- [ ] Keep scripting overhead absent when scripting is unused
- [ ] Keep optional language runtimes from owning world, component, asset, or renderer architecture
- [ ] Allow scripting backends to be added, removed, or replaced without rewriting the native C++ gameplay API
- [ ] Keep language-specific garbage collection, sandboxing, and lifetime rules behind each language integration boundary

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
- [ ] Keep scheduling policy independent from task semantics
- [ ] Avoid making one fixed worker topology or renderer-thread model a permanent public contract
- [ ] Allow subsystem-specific schedulers or specialized execution paths to integrate through explicit dependencies when justified

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
- [ ] Update increasing numbers of project-defined reference bodies through the task system and compare with the sequential path
- [ ] Verify scheduled reference-workload updates remain deterministic where determinism is required
- [ ] Run ThreadSanitizer where supported

### Architecture

- [ ] Keep task description separate from worker-pool implementation
- [ ] Keep dependency semantics usable by gameplay, rendering, asset, procedural, and future Composable World Model workloads
- [ ] Allow the scheduler implementation to evolve without rewriting task producers

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
- [ ] Add scalable reference-workload tiers for one object, one system, multiple systems, and a large synthetic population
- [ ] Record CPU, GPU, memory, allocation, and submission statistics for each reference-workload tier

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
- [ ] Increase reference object/system counts until bottlenecks become measurable
- [ ] Preserve fixed-size reference tiers as regression benchmarks across later optimization phases

### Architecture

- [ ] Optimize only after representative measurements identify a bottleneck
- [ ] Keep optimized fast paths replaceable and comparable with a correct baseline
- [ ] Prefer specialized coexistence over forcing every workload through one optimization strategy
- [ ] Keep performance instrumentation independent from one storage, scheduler, renderer, or world architecture
- [ ] Treat the celestial reference workload as one benchmark family rather than the engine's assumed world or game model

---

## Phase 23.5 — Composable World Model Research

### Learn

- [ ] logical world models versus physical storage models
- [ ] archetype and chunk-oriented ECS storage
- [ ] sparse-set storage
- [ ] relationship and graph representations
- [ ] spatial indices and spatial representations
- [ ] procedural and partially materialized representations
- [ ] GPU-native world representations
- [ ] stable logical identity independent from representation storage
- [ ] authority and source-of-truth semantics
- [ ] projections, indices, caches, mirrors, and derived representations
- [ ] capability-based interfaces
- [ ] cross-representation lifetime and synchronization
- [ ] dependency-driven scheduling
- [ ] prior art from Unity Entities, Unreal Mass, Flecs, EnTT, Bevy, Godot, physics engines, databases, and task systems

### Design

- [ ] Keep the Composable World Model experimental until prototypes and benchmarks justify adoption
- [ ] Define stable world identity independently from ECS rows, object addresses, physics handles, renderer handles, and other representation-local identifiers
- [ ] Allow a logical identity to have zero, one, or many physical representations
- [ ] Allow representation-local data to exist without requiring global world identity when global identity adds no value
- [ ] Allow one domain to use multiple compatible representations instead of requiring exactly one storage model
- [ ] Allow one logical data concept to participate in multiple representations with explicit roles
- [ ] Define representation roles such as authoritative state, derived projection, index, cache, mirror, transient materialization, external/native representation, and read-only view
- [ ] Require explicit authority/source-of-truth declaration for mutable state
- [ ] Define read/write permissions for cross-representation access
- [ ] Define lifetime ownership independently from representation type
- [ ] Define dependency and projection/update declarations
- [ ] Define consistency and synchronization boundaries
- [ ] Define thread-safety and CPU/GPU residency declarations where relevant
- [ ] Define serialization and editor/introspection expectations per representation capability
- [ ] Define deterministic and unload/failure behavior where required
- [ ] Prefer small capability declarations over one giant World Protocol interface
- [ ] Preserve representation-native fast paths instead of forcing a least-common-denominator abstraction
- [ ] Keep domain semantics independent from one built-in representation
- [ ] Keep the public custom-representation contract provisional until several materially different built-in representations reveal the minimum stable contract
- [ ] Define explicit validation for unsupported or semantically inconsistent representation combinations
- [ ] Keep initial representation choice explicit rather than silently migrating storage at runtime
- [ ] Keep small ordinary games usable without configuring Composable World Model internals

### Implement

- [ ] Establish conventional object, archetype ECS, sparse-set ECS, and specialized-direct-structure baselines where relevant
- [ ] Build representative benchmark workloads before designing a large World Protocol
- [ ] Prototype world identity independent from archetype/component storage
- [ ] Measure identity lookup and generational-safety costs
- [ ] Prototype exactly two materially different representations first
- [ ] Start with an archetype representation plus a sparse or spatial representation
- [ ] Attach one logical identity to both prototype representations
- [ ] Declare authority, projection, lifetime, and dependencies explicitly
- [ ] Integrate explicit representation dependencies with the task system
- [ ] Parallelize independent representation work only when dependency declarations prove it safe
- [ ] Evaluate relationship workloads against ECS relationship models before adding a dedicated graph representation
- [ ] Add a spatial representation only after a workload justifies it
- [ ] Add a GPU-native representation only after renderer/resource foundations can measure extraction, residency, and synchronization
- [ ] Add a procedural representation capable of conceptual state without full materialization only after a representative workload exists
- [ ] Add custom representation support only after built-in experiments establish the minimum stable capability contract
- [ ] Implement one useful representation outside engine core as the custom-backend proof
- [ ] Evaluate cross-representation query planning only when representation-native queries are insufficient
- [ ] Evaluate a world compiler or planner only after static logical/physical separation demonstrates concrete value

### Test

- [ ] Keep benchmark source and negative results in the repository
- [ ] Compare equivalent semantics rather than benchmark-friendly approximations
- [ ] Measure frame/wall time, memory, allocations, cache behavior, mutation cost, query cost, synchronization stalls, worker utilization, and CPU/GPU transfer cost where relevant
- [ ] Test homogeneous iteration workloads
- [ ] Test structural-churn workloads
- [ ] Test relationship-heavy workloads
- [ ] Test spatial-query workloads
- [ ] Test procedural materialization, eviction, regeneration, and persistent-delta workloads
- [ ] Test render extraction and GPU projection workloads
- [ ] Test one logical identity represented across gameplay, physics, and rendering
- [ ] Test authority changes, projections, synchronization, destruction, and lifetime correctness
- [ ] Test a small-game workload to measure abstraction overhead when extreme scale is unnecessary
- [ ] Compare dedicated graph candidates against mature relationship approaches before retaining them
- [ ] Compare multi-representation state against a simpler one-representation plus manual-index baseline
- [ ] Measure scheduling overhead against explicit sequential and simpler scheduling baselines
- [ ] Verify a custom representation can integrate with tooling, serialization, scheduling, failure/unload behavior, and profiling without privileged engine internals
- [ ] Verify unified queries preserve native fast paths if cross-representation query planning is introduced
- [ ] Validate the architecture against multiple genres and workloads rather than only the space-game target

### Architecture

- [ ] Use the best proven representation for each workload instead of requiring every workload to be ECS-shaped
- [ ] Keep one coherent logical world capable of using multiple optimized physical representations
- [ ] Keep logical identity independent from physical storage
- [ ] Keep authority explicit and avoid multiple independent mutable sources of truth for the same semantic state
- [ ] Treat derived data as explicit projections, indices, caches, mirrors, or transient materializations
- [ ] Keep specialized representations independently optimizable
- [ ] Do not require custom representations to imitate built-in representations
- [ ] Do not require every representation to support every capability
- [ ] Keep cross-representation communication explicit, measurable, and dependency-aware
- [ ] Keep Composable World Model mechanisms independent from gameplay-specific concepts such as Player, Weapon, Planet, terrain, or one game genre
- [ ] Keep renderer, physics, networking, editor, and procedural systems free to retain native representations behind explicit integration contracts
- [ ] Keep ordinary single-representation use free from unnecessary multi-representation overhead
- [ ] Allow advanced developers to override representation policy without requiring ordinary developers to understand storage internals
- [ ] Allow third-party representations long term without requiring edits to engine core
- [ ] Keep extension contracts versioned, capability-based, testable, introspectable, and unable to silently violate world invariants
- [ ] Remove or redesign Composable World Model abstractions when benchmarks show a simpler established architecture is equally effective
- [ ] Require measurable or concrete technical justification for architecture decisions
- [ ] Preserve simpler baselines and define measurable rejection criteria during Composable World Model experiments
- [ ] Move Composable World Model mechanisms into core only after performance, correctness, flexibility, usability, and generality criteria are met

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
- [ ] Keep mount/source implementations replaceable without changing logical asset identity
- [ ] Allow directory, archive, package, network, or future custom mounts without forcing all mounts to share one physical storage model

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

- [ ] Define engine physics API before exposing backend types
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
- [ ] Connect world synchronization through engine-facing contracts

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
- [ ] Allow physics-native representations to participate in the Composable World Model if validated without forcing physics state into ECS storage
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

- [ ] Define engine audio resource model
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
- [ ] Keep audio-native spatial/state representations independent from ECS or renderer storage assumptions
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

- [ ] Build runtime UI as a engine-owned system
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
- [ ] Keep UI hierarchy/storage independent from the engine's entity/component or future Composable World Model representation choices

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

### Architecture

- [ ] Keep direct calls available for simple strongly coupled interactions
- [ ] Keep event transport independent from event payload semantics
- [ ] Avoid making a global event bus the mandatory communication path between subsystems or world representations

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
- [ ] Add explicit Editor World and Runtime World roles before Play-in-Editor
- [ ] Keep the editable Editor World intact while gameplay runs against a separate Runtime World

### Dear ImGui Migration Policy

- [ ] Allow Dear ImGui developer panels during editor bootstrap
- [ ] Keep editor models independent from Dear ImGui
- [ ] Keep inspector/hierarchy/project state independent from widget implementation
- [ ] Allow editor panels to migrate incrementally to editor UI
- [ ] Avoid a single all-at-once UI rewrite
- [ ] Preserve Phase 9.5 tooling during migration
- [ ] Remove individual Dear ImGui dependencies only when replacement functionality exists

### Test

- [ ] Launch editor
- [ ] Close editor cleanly
- [ ] Verify runtime library does not require editor
- [ ] Verify editor displays diagnostics
- [ ] Verify editor data/model behavior is not owned by Dear ImGui state
- [ ] Open and run the growing reference project through the editor/runtime separation without adding project-specific editor paths

### Architecture

- [ ] Make editor consume engine/tooling APIs
- [ ] Avoid arbitrary access to private engine internals
- [ ] Keep editor state separate from presentation/widget implementation
- [ ] Keep editor-only dependencies out of game/runtime targets
- [ ] Keep Editor World authority separate from temporary Runtime World simulation state
- [ ] Keep Play-in-Editor independent from one concrete world-storage or component-storage implementation

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
- [ ] Inspect, select, and transform project-defined reference-scene objects without hard-coded celestial editor support

### Architecture

- [ ] Avoid hard-coded editor support for each gameplay component
- [ ] Drive inspector/tooling behavior from metadata and extension contracts rather than concrete game classes
- [ ] Keep editor tooling capable of inspecting Composable World Model representations if validated through declared introspection capabilities rather than privileged storage access

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

### Architecture

- [ ] Keep undo/redo commands expressed against stable editor/runtime contracts rather than raw widget state or backend-native storage addresses
- [ ] Allow new editor operations to participate without modifying one monolithic command type

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

### Architecture

- [ ] Keep prefab identity and overrides independent from transient memory addresses and one component-storage layout
- [ ] Keep prefab format versionable so world/component representation changes do not require abandoning existing project data

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
- [ ] Define incremental project/game-module build integration
- [ ] Define compiler-diagnostic capture for editor tooling
- [ ] Separate editor-only dependencies from runtime

### Implement

- [ ] Add project loader
- [ ] Add project creation
- [ ] Add project settings
- [ ] Build project game module
- [ ] Track whether game-module source is dirty
- [ ] Trigger incremental game-module builds from the editor
- [ ] Capture compiler diagnostics for editor display
- [ ] Add Build command
- [ ] Add Build and Play command
- [ ] Keep the previous valid game module available when a new build fails
- [ ] Package runtime dependencies
- [ ] Export runnable game

### Test

- [ ] Create clean project
- [ ] Build clean project
- [ ] Run clean project
- [ ] Export clean project
- [ ] Test missing dependencies
- [ ] Test version mismatch
- [ ] Verify changing one gameplay translation unit does not rebuild unrelated engine targets

### Architecture

- [ ] Keep project builds separate from engine/editor builds
- [ ] Keep compiler/build tooling replaceable behind project-build contracts
- [ ] Keep project format independent from one build-system generator or one optional scripting runtime

---

## Phase 33.5 — C++ Play-in-Editor and Game Module Reload

### Learn

- [ ] dynamic library loading and unloading
- [ ] module lifetime and code-pointer safety
- [ ] incremental C++ build workflows
- [ ] Editor World versus Runtime World ownership
- [ ] module reload boundaries
- [ ] ABI-compatible versus ABI-incompatible changes
- [ ] state reconstruction versus state-preserving reload

### Design

- [ ] Keep the editor running across ordinary game-code build, play, stop, and reload cycles
- [ ] Treat the Editor World as authoritative editable state
- [ ] Create or clone a separate Runtime World when entering Play Mode
- [ ] Keep gameplay changes isolated from the Editor World unless an explicit editor workflow applies them
- [ ] Destroy Runtime World state when leaving Play Mode
- [ ] Define safe game-module unload preconditions
- [ ] Prevent module unload while old module code is executing
- [ ] Require destruction or detachment of module-owned objects before unload
- [ ] Remove or invalidate old callbacks, systems, metadata, function pointers, and other code-dependent registrations before unload
- [ ] Keep basic reload independent from full state-preserving hot reload
- [ ] Define compatibility checks before restoring state across module versions
- [ ] Prefer Runtime World reconstruction when migration cannot be proven safe
- [ ] Keep reflection/reload consumers independent from manual, generated, compiler-assisted, or future standard C++ metadata sources

### Implement

- [ ] Enter Play Mode by creating Runtime World state from Editor World state
- [ ] Run game systems only against Runtime World during Play Mode
- [ ] Stop Play Mode without restarting the editor
- [ ] Destroy Runtime World and return control to Editor World
- [ ] Reload a newly built game module after a successful build
- [ ] Keep the previous valid module when compilation fails
- [ ] Implement safe basic reload by stopping play, destroying runtime state, unloading the old module, loading the new module, and recreating Runtime World
- [ ] Add module-reload diagnostics
- [ ] Add reflected-state snapshot/reconstruction only after reflection and serialization are mature enough
- [ ] Preserve primitive reflected properties only as the first state-preserving reload experiment
- [ ] Add entity/reference restoration incrementally
- [ ] Add schema-change detection for removed, added, or changed reflected properties
- [ ] Add explicit migration hooks only after a concrete incompatible-change use case exists
- [ ] Add implementation-only live reload during active Play Mode only after basic reload is proven safe

### Test

- [ ] Verify repeated Play/Stop cycles do not leak resources
- [ ] Verify Runtime World changes do not mutate Editor World implicitly
- [ ] Verify repeated game-module load/unload cycles
- [ ] Verify no module-owned object survives module unload
- [ ] Verify old callbacks, systems, metadata, function pointers, and vtables cannot be used after unload
- [ ] Verify failed builds preserve the last valid playable module
- [ ] Verify failed module loads leave the editor operational
- [ ] Verify ordinary game-code changes can be rebuilt and played without restarting editor
- [ ] Verify compatible reflected state can be reconstructed after reload when state-preserving reload is implemented
- [ ] Verify incompatible state migration is rejected rather than corrupting memory
- [ ] Verify fallback Runtime World reconstruction works when state preservation is unsafe
- [ ] Use the reference project for repeated build, Play, Stop, and reload cycles as scene complexity grows

### Architecture

- [ ] Keep game code separately buildable from engine/editor code
- [ ] Keep Play-in-Editor independent from full hot reload
- [ ] Keep full state-preserving reload optional rather than a prerequisite for productive C++ gameplay
- [ ] Keep game-module boundaries explicit and avoid dependence on arbitrary cross-module object layout
- [ ] Keep stable type/property/entity identity independent from memory addresses across module reloads
- [ ] Keep reflection descriptive rather than making it the universal engine communication mechanism
- [ ] Keep module reload correctness ahead of preserving every possible C++ object
- [ ] Allow reload implementation to evolve without changing ordinary gameplay APIs
- [ ] Fall back safely to Runtime World reconstruction instead of pretending every ABI/schema change can be hot reloaded
- [ ] Keep the editor alive when game-module reload fails whenever engine/editor binaries themselves remain valid
- [ ] Keep low-level engine, renderer, and editor binary reload outside the initial game-module reload contract

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
- [ ] Run reference simulation and benchmark logic headlessly when rendering is not part of the measurement

### Architecture

- [ ] Keep frontend type separate from reusable engine subsystems
- [ ] Keep window ownership optional for reusable tools
- [ ] Preserve one shared diagnostics/error infrastructure
- [ ] Keep editor, game-module, scripting, and presentation dependencies optional for tools that do not need them

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
- [ ] Expand the reference workload from one system to multiple widely separated systems
- [ ] Test reference-frame transitions between systems at increasing coordinate magnitudes
- [ ] Compare rendering and transform stability for nearby and extremely distant reference content

### Architecture

- [ ] Keep large-world coordinates independent from streaming
- [ ] Keep procedural generation independent from absolute coordinate representation
- [ ] Keep coordinate precision strategy replaceable behind world/reference-frame contracts rather than freezing one numeric representation into every subsystem
- [ ] Do not make planets a core engine primitive
- [ ] Support ordinary small worlds without large-world overhead
- [ ] Keep large-world/reference-frame data compatible with Composable World Model representations if validated without requiring CWM for ordinary worlds

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
- [ ] Keep replication identity and authority independent from one local world representation
- [ ] Allow a Composable World Model network representation if validated without making networking the authority for unrelated local state

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
- [ ] Stream reference systems in and out while moving across large-world space
- [ ] Scale streamable reference systems and resources until memory-budget, cancellation, and residency paths are exercised

### Architecture

- [ ] Keep streaming infrastructure independent from terrain, water, cloud, and other optional render-feature modules
- [ ] Let optional large-world rendering modules consume generic streaming APIs rather than own streaming-core policy
- [ ] Keep streaming separate from coordinate representation
- [ ] Keep streaming separate from procedural-generation algorithms
- [ ] Keep streaming policy independent from one world/component storage representation
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
- [ ] Add a project-defined procedural reference generator that uses generic generation contracts
- [ ] Scale procedural reference generation from one system to many systems and a galaxy-like synthetic dataset

### Test

- [ ] Test seed determinism
- [ ] Test generation cancellation
- [ ] Test concurrent deterministic behavior
- [ ] Benchmark generation utilities
- [ ] Verify fixed seeds reproduce the same generated reference systems across runs
- [ ] Stress generation, materialization, eviction, and regeneration at increasing reference-workload sizes

### Architecture

- [ ] Keep procedural algorithms project-defined unless generally reusable
- [ ] Do not require procedural generation for normal projects
- [ ] Do not make chunks or planets core engine concepts
- [ ] Keep generator definition independent from CPU or GPU execution strategy
- [ ] Allow specialized procedural representations without making materialized ECS/component state mandatory
- [ ] Keep procedural generation compatible with but not dependent on the Composable World Model if validated

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
- [ ] Run the growing reference scene with optional rendering features independently enabled and disabled
- [ ] Use larger reference scenes to expose feature interaction, precision, visibility, synchronization, and memory regressions

### Architecture

- [ ] Keep every feature in this phase optional
- [ ] Keep PBR, shadows, SSS, SSGI, reflections, terrain, water, wetness, clouds, and ray-traced effects outside renderer-core
- [ ] Keep hardware ray-tracing infrastructure independent from specific ray-traced effects
- [ ] Allow developers to enable, disable, configure, replace, and extend built-in rendering modules
- [ ] Allow developers to implement equivalent or new features through the same public renderer mechanisms
- [ ] Avoid initializing or allocating resources for unused optional rendering modules
- [ ] Keep feature modules compatible with multiple geometry, visibility, queue, and world-data representations rather than assuming one built-in path

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

### Architecture

- [ ] Keep editor extensions on versioned public tooling/metadata contracts rather than arbitrary private engine access
- [ ] Allow editor extensions to add support for custom assets, systems, world representations, and tools without modifying editor core
- [ ] Keep extension failure isolated from unrelated editor/runtime systems where practical

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
- [ ] Avoid promoting temporary implementation details, cardinalities, storage layouts, or backend-native types into compatibility promises
- [ ] Define compatibility boundaries narrowly enough that internal implementations can continue to evolve

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
- [ ] Add configurable reference-workload scaling controls for object count, system count, and spatial extent

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
- [ ] Track Composable World Model representation/projection/synchronization overhead separately if CWM research is active
- [ ] Profile fixed reference tiers from a single object through system, multi-system, and galaxy-scale synthetic workloads
- [ ] Identify CPU, GPU, memory, synchronization, streaming, and submission bottlenecks at each tier
- [ ] Keep smaller reference tiers so regressions can be localized instead of testing only maximum scale

### Architecture

- [ ] Keep profiling capable of comparing baseline and replacement implementations under equivalent workloads
- [ ] Keep performance metrics attributable to subsystems, representations, features, and synchronization boundaries
- [ ] Use measurements to choose specialization without turning one benchmark winner into a mandatory architecture for unrelated workloads
- [ ] Treat reference workloads as measurements of engine mechanisms rather than requirements that projects adopt their data model

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
- [ ] Reuse fixed reference-workload tiers for A/B comparisons between baseline and experimental implementations

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
- [ ] Prototype specialized engine physics where justified
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

- [ ] Profile editor UI
- [ ] Identify remaining Dear ImGui dependencies
- [ ] Migrate remaining editor tooling when editor UI provides equivalent capability
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

- [ ] Third-party implementation must not define engine architecture
- [ ] Experimental backends use normal engine boundaries
- [ ] Baseline and experimental implementations can coexist
- [ ] Custom technology must remain measurable
- [ ] Avoid custom implementations whose only advantage is being custom
- [ ] Keep Composable World Model experiments comparable with conventional ECS, object, and specialized direct-structure baselines
- [ ] Preserve simpler implementations when a specialized or composable architecture does not justify its complexity

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
- [ ] Run long-duration reference-scene stress tests across increasing workload tiers

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
- [ ] Track fixed reference-workload regression baselines across supported configurations

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

### Architecture

- [ ] Audit public APIs for accidental dependencies on temporary backend, storage, queue, threading, editor, or module implementation details
- [ ] Verify optional subsystems can remain absent without forcing placeholder initialization or resource allocation
- [ ] Verify replacement backends and extension points do not require unrelated engine-core modifications
- [ ] Preserve migration paths for serialized/project data when internal representations evolve

---

## Long-Term Architecture

### Architecture Scaling Principles

- [ ] Add only the abstraction required by the current use case
- [ ] Do not encode current counts, cardinalities, queue topology, frame layout, storage layout, thread topology, or backend-native handles into public contracts unless the requirement is intentionally fundamental
- [ ] Keep ownership explicit and dependency lifetimes mechanically enforceable where practical
- [ ] Keep logical identity separate from physical storage and backend-native identity where future replacement or multi-representation use requires it
- [ ] Keep policy separate from capability discovery and mechanism
- [ ] Keep optional systems optional in initialization, runtime overhead, dependencies, and public APIs
- [ ] Prefer capability-based composition over giant interfaces that force every implementation into the same shape
- [ ] Preserve specialized/native fast paths when a generic abstraction would erase the reason specialization exists
- [ ] Allow implementations to be replaced, specialized, or experimentally coexisted without rewriting unrelated callers
- [ ] Generalize only after multiple concrete use cases reveal a real shared abstraction
- [ ] Avoid generic managers, registries, service locators, or global buses without a concrete ownership/coordination requirement
- [ ] Avoid forcing renderer, physics, audio, networking, UI, world, assets, or tooling into one storage or execution model
- [ ] Keep built-in implementations on the same public extension mechanisms intended for project-defined or third-party implementations where practical
- [ ] Keep development/editor conveniences from becoming runtime requirements
- [ ] Preserve headless, non-rendering, non-networked, non-scripted, and small-project configurations without unrelated subsystem overhead
- [ ] Keep experimental architecture measurable with explicit acceptance, rejection, and redesign criteria
- [ ] Prefer correctness and safe fallback over preserving an abstraction, optimization, or hot-reload path that cannot prove its invariants
- [ ] Keep architecture documents and roadmap decisions revisable as requirements and evidence change
- [ ] Grow representative reference workloads as new engine capabilities become available instead of deferring integration until the engine is feature-complete
- [ ] Keep domain-specific reference content outside engine-core APIs while using it to validate general-purpose mechanisms
- [ ] Preserve small, medium, and stress reference tiers so correctness and performance regressions can be isolated

### Rendering Principles

- [ ] Keep renderer mechanisms separate from built-in feature implementations and project-specific extensions
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
- [ ] Composable World Model identity/authority/projection mechanisms if research validates them
- [ ] Capability-based custom world-representation integration if research validates it

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
- [ ] Reflection/metadata where explicitly exposed
- [ ] Game-module lifecycle and reload contracts
- [ ] Optional networking

### Replaceable / Experimental Backends

- [ ] Renderer implementation boundaries
- [ ] Physics backend
- [ ] Audio backend
- [ ] Source-asset decoders/importers
- [ ] Developer/editor UI implementation
- [ ] Optional scripting runtimes
- [ ] Project/game-module build integration
- [ ] Composable World Model representations if CWM research is validated
- [ ] Baseline implementations remain available while experimental alternatives are measured
- [ ] Replace implementations only for concrete capability, performance, scalability, or maintenance benefits
- [ ] Subsystem implementations may be replaced or specialized without rewriting unrelated public APIs
- [ ] Initial third-party libraries and backend choices must not become permanent architectural dependencies

### Extension APIs

- [ ] Runtime modules
- [ ] Game modules
- [ ] Custom systems
- [ ] Custom components
- [ ] Custom world representations if CWM research validates the extension contract
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
- [ ] Separately buildable/reloadable C++ game module
- [ ] Project-defined components
- [ ] Project-defined systems
- [ ] Project-defined assets
- [ ] Optional project modules
- [ ] Optional scripting
- [ ] Play-in-Editor without requiring editor restart for ordinary game-code changes

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
