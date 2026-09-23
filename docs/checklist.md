# Engine Development Checklist

## Phase 1 — Application Foundation

- [x] Create application entry point
- [x] Create `GlfwContext`
- [x] Make `Application` own `GlfwContext`
- [x] Create `Window`
- [x] Separate Core and Platform code
- [x] Make `GlfwContext` non-copyable
- [x] Add header guard to `glfw_context.hpp`
- [x] Remove unnecessary GLFW include from `glfw_context.hpp`
- [x] Review platform selection
- [x] Clean up namespace usage
- [x] Build engine implementation as a reusable library target
- [x] Verify clean build
- [x] Verify clean command
- [ ] Verify clean shutdown

## Phase 2 — Error Handling Foundation

- [x] Define engine error codes
- [x] Separate errors by subsystem
- [x] Store human-readable error messages
- [x] Create native error type
- [x] Model optional native API errors
- [x] Capture GLFW native errors
- [x] Define fatal error boundary
- [ ] Preserve native API errors across GLFW and Vulkan
- [ ] Define recoverable errors
- [ ] Define exception policy
- [ ] Avoid generic catch-all error handling

## Phase 3 — Vulkan Foundation

- [ ] Create Vulkan instance
- [ ] Get required GLFW Vulkan extensions
- [ ] Validate required extensions
- [ ] Enable validation layers
- [ ] Create debug messenger
- [ ] Create window surface
- [ ] Preserve Vulkan result codes

## Phase 4 — GPU Setup

- [ ] Enumerate physical devices
- [ ] Inspect physical device capabilities
- [ ] Find queue families
- [ ] Check required device extensions
- [ ] Select physical device
- [ ] Create logical device
- [ ] Retrieve graphics queue
- [ ] Retrieve presentation queue
- [ ] Report unsupported hardware clearly

## Phase 5 — Swapchain

- [ ] Query swapchain support
- [ ] Select surface format
- [ ] Select presentation mode
- [ ] Select swap extent
- [ ] Create swapchain
- [ ] Create image views
- [ ] Handle out-of-date swapchain
- [ ] Handle suboptimal swapchain
- [ ] Handle minimized windows

## Phase 6 — Basic Rendering

- [ ] Create command pool
- [ ] Create command buffers
- [ ] Create synchronization objects
- [ ] Implement frame loop
- [ ] Poll window events
- [ ] Clear the screen
- [ ] Render a triangle
- [ ] Handle frame errors

## Phase 7 — Rendering Foundation

- [ ] Add shader handling
- [ ] Add graphics pipelines
- [ ] Add vertex buffers
- [ ] Add index buffers
- [ ] Add uniform buffers
- [ ] Add textures
- [ ] Add depth buffering
- [ ] Add camera
- [ ] Render multiple objects
- [ ] Separate scene data from GPU resources
- [ ] Define rendering-facing data contracts
- [ ] Define GPU resource ownership
- [ ] Define GPU resource lifetime ordering

## Phase 8 — Production Error Handling

- [x] Create structured engine error type
- [x] Add error context
- [ ] Add nested error causes
- [ ] Add source location information
- [ ] Add Vulkan error conversion
- [ ] Add platform error conversion
- [ ] Distinguish native API origins
- [ ] Use explicit results for recoverable operations
- [ ] Use exceptions only for defined failure boundaries
- [ ] Add assertions
- [ ] Separate programmer errors from runtime errors
- [ ] Add fatal error reporting
- [ ] Add graceful shutdown path
- [ ] Prevent duplicate error reporting

## Phase 9 — Logging and Diagnostics

- [ ] Add logging system
- [ ] Add log levels
- [ ] Add log categories
- [ ] Add timestamps
- [ ] Add debug-only diagnostic output
- [ ] Route Vulkan validation messages into logging
- [ ] Route GLFW errors into logging
- [ ] Add log file output
- [ ] Support diagnostics without the editor

## Phase 10 — Testing Foundation

- [x] Add unit test framework
- [x] Test basic engine error behavior
- [x] Test native error behavior
- [x] Test optional native error context
- [x] Test subsystem classification
- [x] Add sanitizer build
- [x] Run tests through `just`
- [x] Keep `just check` clean
- [ ] Test ownership-sensitive utilities
- [ ] Test pure Vulkan selection logic
- [ ] Add integration tests
- [ ] Test actual GLFW failure paths
- [ ] Establish repeatable performance benchmarks

## Phase 11 — Input

- [ ] Add keyboard input
- [ ] Add mouse input
- [ ] Add scroll input
- [ ] Add window events
- [ ] Separate input from GLFW callbacks

## Phase 12 — Editor UI Foundation

- [ ] Integrate editor UI rendering
- [ ] Add docking
- [ ] Add main menu
- [ ] Add toolbar
- [ ] Add central viewport
- [ ] Route engine errors to the editor

## Phase 13 — Editor Layout

- [ ] Add Scene Hierarchy
- [ ] Add Inspector
- [ ] Add Content Browser
- [ ] Add Console
- [ ] Add Scene View
- [ ] Add Game View
- [ ] Add play controls
- [ ] Add status bar
- [ ] Add error notifications

## Phase 14 — Editor Shell

- [ ] Support resizable docked panels
- [ ] Support tabbed panels
- [ ] Support floating panels
- [ ] Save editor layout
- [ ] Add multiple workspaces
- [ ] Add command palette
- [ ] Add contextual Inspector
- [ ] Add editor theme system
- [ ] Add diagnostics workspace
- [ ] Finalize default editor layout

## Phase 15 — Modular Architecture and Headless Runtime

- [ ] Define module responsibilities and dependency direction
- [ ] Separate reusable engine modules from executable entry points
- [ ] Create independent CMake targets where needed
- [ ] Establish headless engine execution
- [ ] Keep module interfaces minimal
- [ ] Keep third-party API types inside their owning modules
- [ ] Define explicit dependency ownership
- [ ] Avoid unnecessary global state and service locators
- [ ] Document resource ownership and lifetime boundaries
- [ ] Prevent circular module dependencies
- [ ] Define thread-safety contracts when needed
- [ ] Test modules independently
- [ ] Avoid premature abstractions and unnecessary managers

## Phase 16 — Procedural Generation Foundation

- [ ] Define generation inputs and configuration
- [ ] Define generation output types
- [ ] Implement a small CPU-only generator
- [ ] Support deterministic generation from seeds
- [ ] Test identical seeds and parameters
- [ ] Test different seeds and parameter boundaries
- [ ] Define coordinate and region conventions
- [ ] Define generation algorithm versions
- [ ] Separate generation algorithms from orchestration
- [ ] Handle generation failures explicitly
- [ ] Test generation without GLFW or Vulkan

## Phase 17 — CLI Prototyping

- [ ] Create `signum-cli` executable
- [ ] Link CLI to shared generation modules
- [ ] Run CLI without GLFW or Vulkan initialization
- [ ] Add generation commands and options
- [ ] Add help output and argument validation
- [ ] Support explicit seeds and configurations
- [ ] Support reproducible generation
- [ ] Export inspectable generation results
- [ ] Add generation benchmarking
- [ ] Support batch generation
- [ ] Add structured diagnostics and exit codes
- [ ] Test CLI commands and failure paths
- [ ] Test CLI in a headless environment
- [ ] Keep CLI code separate from generation algorithms

## Phase 18 — Generation Scalability and Streaming

- [ ] Define independently addressable generation regions
- [ ] Define stable region identities
- [ ] Test neighboring region boundaries
- [ ] Separate generation scheduling from algorithms
- [ ] Introduce bounded generation workloads
- [ ] Add generation cancellation
- [ ] Define generated-data ownership and lifetime
- [ ] Add generation caching when needed
- [ ] Define cache identity and invalidation
- [ ] Add incremental regeneration
- [ ] Introduce asynchronous generation
- [ ] Verify deterministic results under concurrency
- [ ] Establish CPU and GPU transfer boundaries
- [ ] Add world streaming
- [ ] Add level-of-detail support
- [ ] Define memory and workload budgets
- [ ] Benchmark generation throughput and latency
- [ ] Measure peak memory usage
- [ ] Add generation stress tests
- [ ] Profile before introducing low-level optimizations

## Phase 19 — Procedural Editor Integration

- [ ] Connect editor to shared generation modules
- [ ] Add generation configuration controls
- [ ] Add generated-result preview
- [ ] Support asynchronous preview updates
- [ ] Cancel obsolete preview requests
- [ ] Display generation diagnostics
- [ ] Support reproducible generation sessions
- [ ] Verify CLI and editor output consistency
- [ ] Support generated-data export
- [ ] Integrate generation with world streaming
- [ ] Keep editor dependencies out of generation modules
