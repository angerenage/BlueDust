# Blue Dust

**Blue Dust** is an experimental operating system project with a focus on minimalism and practicality. The goal is to build a basic kernel with essential features, allowing for future growth and exploration as the project evolves.

**Note:** "Blue Dust" is a temporary codename for the project. The name will likely change once the project reaches a stage where a more definitive name is required.

---

## Organization and Methodology

To be honest, this project is pretty experimental, and my workflow is... let's say "unique." Right now, I'm using a **.sh** script to handle compilation instead of a proper build system like **Make** with a cross-compiler. Is it the most efficient way? Nope. But it works for now, and that's what matters.

As things grow (if they grow), I'll probably switch to something more robust. For now, the simplicity of a script fits the project's current size and vibe. Let's see where this leads!

---

## Current Progress

Here’s what’s been accomplished so far in the development of **Blue Dust**:

- **Bootloader**:
  A functional bootloader that:
  - Activates the A20 line.
  - Reads the memory map.
  - Loads the kernel from disk.
  - Finally sets the processor to 64-bit long mode before launching the kernel.

- **Kernel**:
  - Initializes a basic terminal for output.
  - Sets up an Interrupt Descriptor Table (IDT) to handle:
    - Major CPU faults, including division by zero, invalid opcode, page fault, general protection fault, and double fault.  
    - Keyboard input.
    - System calls.

- **System Calls**:
  Currently, there is only one system call implemented, allowing allocation and deallocation of heap memory while the memory allocator is not initialized (and not implemented yet).

---

## Building and Running

Currently, the project is built using a simple shell script. To build and run the kernel:
1. Clone the repository.
2. Execute the build script compiles all components and organizes the outputs into a `.build` folder:
   ```bash
   ./build.sh
   ```
3. Use QEMU (or another compatible emulator) to test the generated binary:
   ```bash
   qemu-system-x86_64 -drive format=raw,file=.build/os.bin
   ```

### Requirements:
To build and run **Blue Dust**, you need the following tools installed:

- **GCC**: A cross-compiler targeting `x86_64-elf` binaries.  
- **NASM**: An assembler capable of generating `elf64` binaries.  
- **QEMU**: An x86-64 emulator for testing the kernel (tested only on QEMU).  

---

## Kernel Capabilities

The Blue Dust kernel is designed to provide the following core functionalities.
**Note:** Given the project's current state, these features are subject to change as development progresses.

### Memory Management
- Allocation and deallocation of physical pages.
- Management of page tables to provide virtual memory.

### Interrupt Management
- Support for hardware interrupts: timer, keyboard, disk, etc.

### Inter-Process Communication (IPC)
- Basic primitives to exchange messages between services (implementation details to be defined).

### Task Management
- A simple preemptive multitasking scheduler (implementation details to be defined).

### Minimal File System
- Read-only access.
- Direct access via predefined inodes (no path support).
- Enough to load files necessary for the system to boot.

### Program Execution
- Loading a bare-bones loader.
- Executing the service manager via the loader.

---

## Essential Components

### Kernel
A minimal kernel that provides only the functionalities described above.

### Loader
A program that reads an executable file, maps it into memory, and returns a pointer to its entry point.

### Service Manager
A program responsible for configuring and launching user services based on a configuration file.

### File System
A user-space service that mounts a complete filesystem.

---

## Boot Sequence

### 1. Bootloader
The bootloader loads the kernel into memory and transfers control to it.

### 2. Kernel
The kernel:
1. Initializes its core components:
   - Memory management.
   - Interrupt management.
   - Minimal file system.
   - Task scheduler.
2. Reads the bare-bones loader program.
3. Maps the loader into memory and transfers control to it.

### 3. Loader
The loader:
1. Reads the service manager's file from inode 0 of the minimal file system.
2. Loads the service manager's ELF file into memory.
3. Passes a pointer to its entry point to the service manager.

### 4. Service Manager
The service manager:
1. Reads its configuration file.
2. Identifies and loads user services specified in the configuration:
   - Loads and launches the file system service to mount a complete filesystem.
   - Loads and launches other user services (networking, logging, etc.).

### 5. User Services
Once loaded, user services operate independently and interact via the IPC provided by the kernel.

---

## Project Goals

Blue Dust serves as a foundation for experimenting with low-level system design. While minimal in its initial scope, the project is designed to be extensible and adaptable for implementing more advanced features over time.  
Given the experimental nature of the project, many features and components described here may change or evolve as the system grows and as I learn along the way.

---

## Contributing

While this is currently a personal project, if you ended up here and you're interested in discussing or contributing, feel free to reach out with ideas or suggestions.

---

## License

To be determined.
