# Engine Development Checklist

## Phase 1 — Application Foundation

- [x] Create application entry point

  - Starts and shuts down the engine.

- [x] Create `GlfwContext`

  - Initializes and terminates GLFW.

- [x] Make `Application` own `GlfwContext`

  - Keeps GLFW alive for the application lifetime.

- [x] Create `Window`

  - Creates and destroys the GLFW window.

- [x] Separate Core and Platform code

  - Keeps application logic separate from platform code.

- [x] Make `GlfwContext` non-copyable

  - Prevents invalid GLFW lifetime duplication.

- [x] Add header guard to `glfw_context.hpp`

  - Prevents duplicate header inclusion.

- [x] Remove unnecessary GLFW include from `glfw_context.hpp`

  - Keeps implementation details out of the header.

- [x] Review platform selection

  - Lets GLFW select the supported platform automatically.

- [x] Clean up namespace usage

  - Keeps namespace usage simple and consistent.

- [x] Verify clean build

  - Builds with zero errors and warnings.

- [ ] Verify clean shutdown

  - Window closes normally before GLFW terminates.

---

## Phase 2 — Error Handling Foundation

- [x] Define engine error codes

  - Gives failures stable machine-readable identifiers.

- [ ] Separate errors by subsystem

  - Distinguishes Platform, Vulkan, Renderer, Editor, and other failures.

- [x] Store human-readable error messages

  - Explains what failed.

- [x] Create native error type

  - Groups an external API error code with its human-readable description.

- [x] Model optional native API errors

  - Allows an `EngineError` to contain native API information only when one exists.

- [ ] Preserve native API errors

  - Keeps GLFW and Vulkan error information available when failures occur.

- [x] Capture GLFW native errors

  - Preserves GLFW's native error code and description when an operation fails.

- [x] Define fatal error boundary

  - Unhandled startup errors propagate to `main`.

- [ ] Define recoverable errors

  - Allows failures that do not require engine shutdown.

- [ ] Define exception policy

  - Establishes where exceptions are allowed and where errors are returned explicitly.

- [ ] Avoid generic catch-all error handling

  - Keeps failure causes visible and debuggable.

---

## Phase 3 — Vulkan Foundation

- [ ] Create Vulkan instance

  - Initializes Vulkan for the engine.

- [ ] Get required GLFW Vulkan extensions

  - Provides extensions needed for presentation.

- [ ] Validate required extensions

  - Detects unavailable Vulkan requirements before initialization continues.

- [ ] Enable validation layers

  - Detects incorrect Vulkan usage.

- [ ] Create debug messenger

  - Reports Vulkan validation messages.

- [ ] Create window surface

  - Connects the GLFW window to Vulkan.

- [ ] Preserve Vulkan result codes

  - Keeps original Vulkan failure information.

---

## Phase 4 — GPU Setup

- [ ] Enumerate physical devices

  - Finds available GPUs.

- [ ] Inspect physical device capabilities

  - Determines supported features and limits.

- [ ] Find queue families

  - Finds graphics and presentation queues.

- [ ] Check required device extensions

  - Verifies required GPU functionality.

- [ ] Select physical device

  - Chooses a compatible GPU.

- [ ] Create logical device

  - Creates the Vulkan device used by the engine.

- [ ] Retrieve graphics queue

  - Executes graphics work.

- [ ] Retrieve presentation queue

  - Presents frames to the window.

- [ ] Report unsupported hardware clearly

  - Explains why a GPU cannot run the engine.

---

## Phase 5 — Swapchain

- [ ] Query swapchain support

  - Finds supported formats, modes, and sizes.

- [ ] Select surface format

  - Chooses the swapchain image format.

- [ ] Select presentation mode

  - Chooses how frames are presented.

- [ ] Select swap extent

  - Chooses rendering dimensions.

- [ ] Create swapchain

  - Creates presentable images.

- [ ] Create image views

  - Makes swapchain images usable by Vulkan.

- [ ] Handle out-of-date swapchain

  - Recreates the swapchain when required.

- [ ] Handle suboptimal swapchain

  - Responds safely when presentation conditions change.

- [ ] Handle minimized windows

  - Avoids invalid rendering while the framebuffer has no usable size.

---

## Phase 6 — Basic Rendering

- [ ] Create command pool

  - Allocates command buffers.

- [ ] Create command buffers

  - Records GPU work.

- [ ] Create synchronization objects

  - Coordinates CPU, GPU, and presentation.

- [ ] Implement frame loop

  - Acquires, records, submits, and presents frames.

- [ ] Poll window events

  - Processes events without blocking rendering.

- [ ] Clear the screen

  - Produces the first visible Vulkan frame.

- [ ] Render a triangle

  - Verifies the graphics pipeline.

- [ ] Handle frame errors

  - Prevents recoverable presentation failures from crashing the engine.

---

## Phase 7 — Rendering Foundation

- [ ] Add shader handling

  - Loads and manages shaders.

- [ ] Add graphics pipelines

  - Defines rendering state.

- [ ] Add vertex buffers

  - Stores vertex data.

- [ ] Add index buffers

  - Stores indexed geometry.

- [ ] Add uniform buffers

  - Sends dynamic data to shaders.

- [ ] Add textures

  - Supports textured rendering.

- [ ] Add depth buffering

  - Supports correct 3D depth.

- [ ] Add camera

  - Provides view and projection transforms.

- [ ] Render multiple objects

  - Establishes basic scene rendering.

- [ ] Define GPU resource ownership

  - Ensures resources are destroyed by the correct owner.

- [ ] Define GPU resource lifetime ordering

  - Prevents resources from outliving required Vulkan objects.

---

## Phase 8 — Production Error Handling

- [ ] Create structured engine error type

  - Combines error code, message, subsystem, and native API information.

- [ ] Add error context

  - Records which operation failed.

- [ ] Add nested error causes

  - Preserves the original failure when higher layers add context.

- [ ] Add source location information

  - Records where internal failures originate.

- [ ] Add Vulkan error conversion

  - Converts `VkResult` values without discarding the original result.

- [ ] Add platform error conversion

  - Converts GLFW failures into engine errors.

- [ ] Use explicit results for recoverable operations

  - Returns success or failure without requiring exceptions.

- [ ] Use exceptions only for defined failure boundaries

  - Keeps exception behavior predictable.

- [ ] Add assertions

  - Detects programmer errors and broken invariants during development.

- [ ] Separate programmer errors from runtime errors

  - Distinguishes engine bugs from expected external failures.

- [ ] Add fatal error reporting

  - Reports unrecoverable failures before shutdown.

- [ ] Add graceful shutdown path

  - Releases owned resources after recoverable shutdown failures.

- [ ] Prevent duplicate error reporting

  - Ensures one failure is not repeatedly logged by every layer.

---

## Phase 9 — Logging and Diagnostics

- [ ] Add logging system

  - Records engine activity and failures.

- [ ] Add log levels

  - Supports trace, debug, info, warning, error, and fatal output.

- [ ] Add log categories

  - Separates Core, Platform, Vulkan, Renderer, Editor, and other messages.

- [ ] Add timestamps

  - Records when events occur.

- [ ] Add debug-only diagnostic output

  - Keeps development details out of production output.

- [ ] Route Vulkan validation messages into logging

  - Integrates GPU diagnostics with engine logs.

- [ ] Route GLFW errors into logging

  - Integrates platform diagnostics with engine logs.

- [ ] Add log file output

  - Preserves diagnostics after the engine closes.

---

## Phase 10 — Testing Foundation

- [x] Add unit test framework

  - Provides automated C++ tests.

- [x] Test basic engine error behavior

  - Verifies engine error codes, messages, and standard exception behavior.

- [x] Test native error behavior

  - Verifies native error codes and descriptions remain intact.

- [x] Test optional native error context

  - Verifies engine-only errors contain no native error and external failures preserve native context.

- [ ] Test ownership-sensitive utilities

  - Verifies important lifetime behavior.

- [ ] Test pure Vulkan selection logic

  - Tests formats, queues, extensions, and device-selection rules without rendering.

- [ ] Add integration tests

  - Tests Vulkan initialization and other system boundaries.

- [ ] Add sanitizer build

  - Detects memory and undefined-behavior problems.

- [x] Run tests through `just`

  - Keeps verification repeatable.

- [ ] Keep `just check` clean

  - Provides the normal development verification gate.

---

## Phase 11 — Input

- [ ] Add keyboard input

  - Tracks keyboard state.

- [ ] Add mouse input

  - Tracks mouse state and movement.

- [ ] Add scroll input

  - Tracks scrolling.

- [ ] Add window events

  - Handles resize, focus, and close events.

- [ ] Separate input from GLFW callbacks

  - Provides an engine-facing input API.

---

## Phase 12 — Editor UI Foundation

- [ ] Integrate editor UI rendering

  - Renders the editor through Vulkan.

- [ ] Add docking

  - Supports rearrangeable editor panels.

- [ ] Add main menu

  - Provides editor commands.

- [ ] Add toolbar

  - Provides common editor controls.

- [ ] Add central viewport

  - Displays the rendered scene.

- [ ] Route engine errors to the editor

  - Makes failures visible without relying only on terminal output.

---

## Phase 13 — Editor Layout

- [ ] Add Scene Hierarchy

  - Displays scene objects.

- [ ] Add Inspector

  - Displays and edits selected-object properties.

- [ ] Add Content Browser

  - Displays project assets.

- [ ] Add Console

  - Displays logs, warnings, and errors.

- [ ] Add Scene View

  - Provides the editor scene viewport.

- [ ] Add Game View

  - Displays runtime camera output.

- [ ] Add play controls

  - Provides play, pause, and stop.

- [ ] Add status bar

  - Displays engine and project status.

- [ ] Add error notifications

  - Surfaces important recoverable failures inside the editor.

---

## Phase 14 — Editor Shell

- [ ] Support resizable docked panels

  - Allows flexible layouts.

- [ ] Support tabbed panels

  - Groups related tools.

- [ ] Support floating panels

  - Allows detached tools.

- [ ] Save editor layout

  - Restores the workspace between sessions.

- [ ] Add multiple workspaces

  - Supports different editor workflows.

- [ ] Add command palette

  - Provides fast command access.

- [ ] Add contextual Inspector

  - Changes controls based on selection.

- [ ] Add editor theme system

  - Defines consistent styling.

- [ ] Add diagnostics workspace

  - Provides dedicated access to logs, errors, validation, and engine state.

- [ ] Finalize default editor layout

  - Produces the editor shell.
