Using
Formats:
- [[TBSP]]
- [[TWAD]]
- [[Protocol]]


Engine is client and server.

## **Modules:**
### Client
consist of:
- Window  
- Input  
- Renderer  
- Audio
- ClientWorld  
- ClientUI  
- DevUI  (???)
- ClientConnection (???) 
- ClientFSM  
- LocalServerHost, optional (???)

It is an orchestrator module, a top-level module, it takes all input and passes it on to the modules inside, essentially an orchestrator module.

Client does not implement gameplay logic directly.  
Client does not render objects directly.  
Client does not simulate the server world directly.  
Client does not contain hardcoded any logic directly.
## Renderer

### Responsibility
Renderer draws the current frame using GPU resources.

Renderer renders:
- world geometry
- dynamic entities
- sprites
- particles
- debug primitives
- game UI primitives
- optional editor/dev overlays

### Owns
- OpenGL objects
- shaders
- GPU buffers
- textures
- framebuffers
- render caches
- uploaded map resources
### Inputs
- WorldRenderState
- Camera
- RenderSettings
- UiDrawData or immediate UI draw calls
### Outputs
- pixels on the window framebuffer
- optional render statistics

### Depends on
- OpenGL backend
- asset/resource handles
- core math/types

### Does not
- own the game world
- own the BSP source data
- simulate entities
- handle SDL input
- know about menu flow
- know about server state



## Server

## Audio

## Input

## Window



