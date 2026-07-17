# Tungsten Architecture 🚧

[![Language](https://shields.io)](https://cppreference.com)
[![Engine Architecture](https://shields.io)]()
[![Platform Layer](https://shields.io)]()

**Tungsten** is a titanic, monolithic game engine ecosystem built entirely from scratch in **C++20**. Inspired by the legendary technology of the late 90s (Quake/GoldSrc architecture), the project aims to implement a highly performant, modular, and sandboxed game development framework without relying on standard runtime libraries.

Currently, the primary focus is establishing a bulletproof, high-performance **Client-Server network layer** alongside a custom binary protocol to handle state replication and multi-stage synchronization.

---

## 🏗 Modular Ecosystem Architecture

The project is strictly split into decoupled dynamic libraries (DLLs) and low-level pipeline tools to ensure maximum isolation and modularity:

### 🧩 Engine Core Modules
* **`tclient.dll`**: The client-side runtime environment. Responsible for window management, input polling, portal-based rendering, PVS culling, and prediction/interpolation of network states.
* **`tserver.dll`**: The dedicated server core. Manages authoritative simulation, collision resolution, connected state machine lifecycles, and delta-compressed snapshot broadcasting.
* **`tgame.dll`**: The isolated game logic assembly. Contains gameplay mechanics, entity definitions, and systems shared by both client and server via a strict interface.

### 🛠 Toolchain & Asset Pipelines
* **`tasm.exe`**: A custom assembler and VM compiler. Scripting in Tungsten is compiled into native executable headers with defined sections (code, data) and a specialized instruction set, ensuring safe and high-speed execution of game scripts.
* **`tbsp.exe`**: The map compiler (derived from a heavily modified and modernized QBSP codebase) that processes level geometry into structured Binary Space Partitioning trees.
* **`twad.exe`**: The asset packaging and indexing utility used to compress, archive, and manage raw game textures, sounds, and manifests.
* **`tmapeditor.exe`**: The upcoming visual level editor tailored to the engine's portal and BSP constraints.

---

## ⚡ Architectural Principles

### 🛸 Zero-Dependency Platform Layer (No STL / No OS Intrusion)
The core architecture of Tungsten enforces a **strict hardware/OS boundary**. The engine modules do not communicate directly with operating system APIs, external libraries, or even the C++ Standard Template Library (`std::`).

### 🌐 High-Performance Networking Engine
The networking framework provides a custom, strictly-typed protocol designed for real-time multiplayer simulation:

---

## 🗺 Current Development Roadmap

- [x] Abstract Platform Layer interfaces & primitive wrappers
- [x] Custom stream serialization (`protocol_reader`/`writer`) and Endian mapping
- [x] Base Network Finite State Machine framework (`base_fsm`)
- [🔄] **In Progress**: Authority handshake logic and state mapping in `client_fsm` / `server_fsm`
- [ ] Asset delta-transfer engine (file manifest replication during `file_sync` stage)
- [ ] OpenGL Modern Profile rendering implementation for BSP map parsing
- [ ] PVS (Potentially Visible Set) visibility tables runtime compiler
- [ ] Virtual Machine execution pipeline for compiled `tasm.exe` binaries
