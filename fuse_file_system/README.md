# NUFS: A Simple User-Space File System

This project implements a custom file system in C using FUSE (Filesystem in Userspace). The system provides disk abstraction, block and inode management, directory structure, and POSIX-compliant file operations. It was designed and built for a systems programming course to demonstrate low-level file system architecture and FUSE integration.

---

## Features

- **Disk Abstraction**  
  - Block device simulated via a memory-mapped disk image (1MB, 256 blocks x 4KB)
  - Efficient block allocation and management with bitmaps

- **Inode Management**  
  - Fixed-size inodes with fields for reference count, type/permissions, size, and data block pointer
  - Inode allocation, retrieval, and metadata updates

- **Directory Structure**  
  - Hierarchical directories with fixed-length entries and directory traversal
  - Directory lookup, insertion, deletion, and listing

- **File Operations (POSIX Calls via FUSE)**  
  - Create and delete files and directories (`mknod`, `mkdir`, `unlink`, `rmdir`)
  - List directory contents (`readdir`)
  - Read/write file data with offset and size
  - Rename, access, and metadata operations

- **Built-In Tests and Utilities**  
  - Bitmap and block allocation unit tests
  - String list manipulation for path parsing

---

## How to Build and Run

1. **Install Dependencies**  
   - Requires `gcc`, `make`, and `libfuse-dev` (FUSE 2.6+)
   - Example install:  
     ```bash
     sudo apt-get install libfuse-dev
     ```

2. **Compile the Project**  
   ```bash
   make
