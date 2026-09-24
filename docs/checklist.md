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
- [ ] application composition root
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

- [ ] Vulkan instances
- [ ] Vulkan extensions
- [ ] Vulkan layers
- [ ] debug callbacks
- [ ] surfaces
- [ ] `VkResult`

### Implement

- [x] Create Vulkan instance
- [x] Get required GLFW Vulkan extensions
- [ ] Validate required extensions
- [ ] Enable validation layers
- [ ] Create debug messenger
- [ ] Create window surface
- [ ] Preserve `VkResult`
- [x] Add Vulkan error conversion

### Test

- [ ] Test extension-selection logic
- [ ] Test missing-extension behavior
- [ ] Verify validation output
- [ ] Verify Vulkan lifetime order
- [ ] Verify Vulkan shutdown

### Architecture

- [ ] Keep Vulkan inside renderer/backend code
- [ ] Avoid exposing Vulkan types to normal game APIs
- [ ] Keep Vulkan initialization out of gameplay code

---

## Phase 4 — GPU Device Foundation

### Learn

- [ ] physical devices
- [ ] logical devices
- [ ] queue families
- [ ] queues
- [ ] device capabilities
- [ ] feature negotiation

### Implement

- [ ] Enumerate physical devices
- [ ] Inspect device capabilities
- [ ] Find queue families
- [ ] Check required device extensions
- [ ] Separate required and optional capabilities
- [ ] Select physical device
- [ ] Create logical device
- [ ] Retrieve graphics queue
- [ ] Retrieve presentation queue
- [ ] Report unsupported hardware

### Test

- [ ] Test device-selection logic
- [ ] Test queue-family selection
- [ ] Test unsupported-device behavior

### Architecture

- [ ] Separate capability discovery from device-selection policy
- [ ] Keep hardware policy independent from gameplay

---

## Phase 5 — Swapchain and Presentation

### Learn

- [ ] swapchains
- [ ] surface formats
- [ ] presentation modes
- [ ] image views
- [ ] swapchain recreation

### Implement

- [ ] Query swapchain support
- [ ] Select surface format
- [ ] Select presentation mode
- [ ] Select swap extent
- [ ] Create swapchain
- [ ] Create image views
- [ ] Handle out-of-date swapchain
- [ ] Handle suboptimal swapchain
- [ ] Handle minimized windows
- [ ] Recreate dependent resources safely

### Test

- [ ] Test format selection
- [ ] Test presentation-mode selection
- [ ] Test extent selection
- [ ] Resize repeatedly
- [ ] Minimize and restore repeatedly

### Architecture

- [ ] Keep presentation backend-specific
- [ ] Keep swapchain details out of game-facing APIs

---

## Phase 6 — Basic Rendering

### Learn

- [ ] command pools
- [ ] command buffers
- [ ] synchronization
- [ ] frame ownership
- [ ] acquire-submit-present flow

### Implement

- [ ] Create command pool
- [ ] Create command buffers
- [ ] Create synchronization objects
- [ ] Implement frame loop
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

---

## Phase 7 — Rendering Resource Foundation

### Learn

- [ ] shaders
- [ ] graphics pipelines
- [ ] buffers
- [ ] textures
- [ ] descriptors
- [ ] staging
- [ ] GPU resource lifetime

### Implement

- [ ] Add shader handling
- [ ] Add graphics pipelines
- [ ] Add vertex buffers
- [ ] Add index buffers
- [ ] Add uniform/storage buffers
- [ ] Add textures
- [ ] Add depth buffering
- [ ] Render multiple objects
- [ ] Separate scene data from GPU resources
- [ ] Define rendering-facing data contracts
- [ ] Define GPU resource ownership
- [ ] Define GPU lifetime ordering

### Test

- [ ] Test pure resource descriptions
- [ ] Test partial creation failures
- [ ] Verify destruction ordering
- [ ] Run Vulkan validation
- [ ] Run sanitizers

### Architecture

- [ ] Keep runtime object identity separate from GPU resource identity
- [ ] Avoid raw Vulkan ownership in game entities

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
- [ ] Define exception policy
- [ ] Add assertion policy
- [ ] Separate programmer errors from runtime failures
- [x] Add fatal error reporting
- [x] Add graceful fatal shutdown
- [ ] Prevent duplicate error reporting

### Learn

- [x] formatting
- [x] reporting concept
- [x] exception boundaries
- [x] source-location diagnostics
- [ ] fatal reporting
- [ ] assertion semantics
- [ ] explicit result types

### Test

- [x] Test formatter without native context
- [x] Test formatter with native context
- [x] Test native values
- [x] Test core diagnostic fields
- [x] Test source location formatting
- [ ] Test fatal reporter
- [ ] Test recoverable result behavior

### Architecture

- [x] Keep `EngineError` independent from terminal output
- [x] Keep formatting independent from reporting
- [x] Keep fatal reporting at defined boundary
- [ ] Keep recoverable failures non-fatal

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
- [ ] Add integration-test structure
- [ ] Add benchmark target

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
- [ ] abstraction overhead

### Design

- [ ] Define minimum renderer API
- [ ] Define backend-independent resource descriptions
- [ ] Define renderer resource handles
- [ ] Define renderer ownership model
- [ ] Define Vulkan escape-hatch policy

### Implement

- [ ] Move normal rendering callers away from raw Vulkan
- [ ] Keep Vulkan as first backend
- [ ] Avoid premature additional graphics backends
- [ ] Keep abstraction measurable

### Test

- [ ] Test resource descriptions
- [ ] Verify public renderer API avoids unnecessary Vulkan types
- [ ] Benchmark abstraction where useful

### Architecture

- [ ] Keep game rendering API independent from Vulkan
- [ ] Keep explicit low-level backend access for advanced extensions

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
- [ ] Define cache versioning

### Implement

- [ ] Add asset database
- [ ] Add resource manager
- [ ] Add first importer
- [ ] Add asset load
- [ ] Add asset unload
- [ ] Add dependency tracking
- [ ] Add reload path
- [ ] Add asset diagnostics

### Test

- [ ] Test missing assets
- [ ] Test invalid assets
- [ ] Test asset identity
- [ ] Test dependencies
- [ ] Test reload

---

## Phase 19 — Module Architecture

### Learn

- [ ] static libraries
- [ ] shared libraries
- [ ] dynamic loading
- [ ] symbol visibility
- [ ] C++ ABI limitations
- [ ] C ABI boundaries
- [ ] version negotiation

### Design

- [ ] Define engine module responsibilities
- [ ] Define module dependency direction
- [ ] Define project module boundary
- [ ] Define optional module lifecycle
- [ ] Define module dependency declarations
- [ ] Define startup/shutdown contract
- [ ] Define interface versioning
- [ ] Define ABI policy

### Implement

- [ ] Split justified subsystems into CMake targets
- [ ] Add minimal module lifecycle
- [ ] Load test module
- [ ] Reject incompatible module versions

### Test

- [ ] Test module loading
- [ ] Test invalid module
- [ ] Test version mismatch
- [ ] Test dependency ordering
- [ ] Test module shutdown

### Architecture

- [ ] Avoid promising stable C++ ABI prematurely
- [ ] Avoid exposing unstable internals across module boundaries

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

### Implement

- [ ] Create project/game module
- [ ] Expose runtime APIs
- [ ] Add project-defined component example
- [ ] Add project-defined system example
- [ ] Run project without modifying engine core

### Test

- [ ] Build standalone project module
- [ ] Run project-defined behavior
- [ ] Verify project does not need Vulkan calls
- [ ] Verify project does not need GLFW calls
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

### Optimize When Measured

- [ ] Cache-friendly layouts
- [ ] batching
- [ ] specialized allocators
- [ ] SIMD
- [ ] task batching
- [ ] asynchronous pipelines
- [ ] GPU work

### Test

- [ ] Track benchmark regressions
- [ ] Record optimization baselines
- [ ] Verify optimized behavior remains correct

---

## Phase 24 — Filesystem and Virtual Paths

### Learn

- [ ] filesystem abstraction
- [ ] path normalization
- [ ] virtual paths
- [ ] mount concepts

### Design

- [ ] Define project paths
- [ ] Define engine paths
- [ ] Define asset paths
- [ ] Define normalized path rules
- [ ] Separate physical paths from logical asset identity

### Implement

- [ ] Add filesystem service
- [ ] Add file read/write helpers
- [ ] Add path normalization
- [ ] Add virtual/project paths where useful

### Test

- [ ] Test path normalization
- [ ] Test missing files
- [ ] Test invalid paths
- [ ] Test project-relative paths

---

## Phase 25 — Physics

### Design

- [ ] Define physics abstraction
- [ ] Keep physics backend replaceable
- [ ] Define physics ownership
- [ ] Define physics/world synchronization
- [ ] Keep gameplay collision responses project-defined

### Implement

- [ ] Select backend when requirements are known
- [ ] Add physics world
- [ ] Add body representation
- [ ] Add collision shapes
- [ ] Add queries
- [ ] Add events

### Test

- [ ] Test creation/destruction
- [ ] Test collision
- [ ] Test queries
- [ ] Test synchronization

---

## Phase 26 — Audio

### Design

- [ ] Define audio resource model
- [ ] Define playback API
- [ ] Define audio ownership
- [ ] Define spatial-audio API
- [ ] Keep backend replaceable

### Implement

- [ ] Add audio backend
- [ ] Add audio resources
- [ ] Add playback
- [ ] Add spatial audio
- [ ] Add volume categories

### Test

- [ ] Test missing audio device
- [ ] Test missing resource
- [ ] Test playback lifecycle

---

## Phase 27 — Runtime UI

### Design

- [ ] Separate runtime UI from editor UI
- [ ] Define UI ownership
- [ ] Define input integration
- [ ] Define rendering integration
- [ ] Keep runtime UI optional

### Implement

- [ ] Add UI root
- [ ] Add basic controls
- [ ] Add layout
- [ ] Add text
- [ ] Add images
- [ ] Add input handling

### Test

- [ ] Test layout
- [ ] Test input
- [ ] Test lifecycle

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

### Test

- [ ] Launch editor
- [ ] Close editor cleanly
- [ ] Verify runtime library does not require editor
- [ ] Verify editor displays diagnostics

### Architecture

- [ ] Make editor consume engine/tooling APIs
- [ ] Avoid arbitrary access to private engine internals

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

## Phase 34 — Headless Runtime and Tools

### Implement

- [ ] Run reusable engine systems without window creation
- [ ] Run reusable engine systems without Vulkan where possible
- [ ] Add tooling executable
- [ ] Add project validation commands
- [ ] Add asset processing commands
- [ ] Add benchmark commands
- [ ] Add structured CLI diagnostics
- [ ] Add meaningful exit codes

### Test

- [ ] Run headless tests
- [ ] Test tooling failures
- [ ] Test exit codes
- [ ] Verify tools reuse engine modules

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

### Design

- [ ] Keep networking optional
- [ ] Separate transport from gameplay replication
- [ ] Avoid assuming every project replicates entities
- [ ] Define generic messages
- [ ] Define connection lifecycle

### Implement

- [ ] Add transport abstraction
- [ ] Add connections
- [ ] Add messages
- [ ] Add optional replication support

### Test

- [ ] Loopback tests
- [ ] Disconnect tests
- [ ] Malformed packet tests
- [ ] Latency simulation
- [ ] Packet-loss simulation

---

## Phase 36 — Streaming and Large Worlds

### Design

- [ ] Define generic streamable resources
- [ ] Define loading priorities
- [ ] Define memory budgets
- [ ] Define cancellation
- [ ] Keep spatial streaming optional
- [ ] Avoid requiring chunk-based worlds

### Implement

- [ ] Add asynchronous load
- [ ] Add asynchronous unload
- [ ] Add streaming requests
- [ ] Add streaming priorities
- [ ] Add streaming cancellation
- [ ] Add memory-budget enforcement

### Test

- [ ] Test load/unload
- [ ] Test cancellation
- [ ] Test memory limits
- [ ] Stress streaming
- [ ] Test shutdown during streaming

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

## Phase 38 — Advanced Rendering

### Implement as Needed

- [ ] compute workloads
- [ ] indirect rendering
- [ ] GPU-driven rendering
- [ ] resource indexing/bindless techniques
- [ ] material system
- [ ] shadows
- [ ] post-processing
- [ ] HDR
- [ ] temporal effects
- [ ] visibility systems
- [ ] renderer multithreading
- [ ] optional ray tracing
- [ ] renderer extension API

### Test

- [ ] Profile each major feature
- [ ] Verify GPU synchronization
- [ ] Track GPU memory
- [ ] Track frame-time regressions

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

### Engine Internals

- [ ] Core
- [ ] Platform
- [ ] Memory
- [ ] Job system
- [ ] Filesystem
- [ ] Logging
- [ ] Errors
- [ ] Diagnostics
- [ ] Asset infrastructure
- [ ] Renderer backend/RHI

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

### Extension APIs

- [ ] Runtime modules
- [ ] Custom systems
- [ ] Custom components
- [ ] Custom asset types
- [ ] Custom importers
- [ ] Renderer extensions
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
- [ ] Fatal error reporting
- [ ] Graceful fatal shutdown
- [ ] Recoverable error policy
- [ ] Exception policy
- [ ] Verify clean application shutdown
- [ ] Vulkan instance foundation
