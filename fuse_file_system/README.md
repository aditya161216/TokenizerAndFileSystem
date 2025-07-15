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

3. **Run the Filesystem**  
   ```bash
   mkdir mount_dir
   ./nufs -f mount_dir backing_store.img

4. **Run Tests**  
   ```bash
   make test

---

## File Structure
- `nufs.c`: Main FUSE operations implementation (filesystem driver)
- `storage.c/.h`: Core file and directory logic (create, read, write, delete)
- `blocks.c/.h`: Block device abstraction, allocation, free, mmap
- `inode.c/.h`: Inode structure and management
- `directory.c/.h`: Directory entries, lookup, path resolution
- `bitmap.c/.h`: Bitmap manipulation utilities
- `slist.c/.h`: String list utilities for path parsing
- `Makefile`: Build and test automation

---

## Credits
Developed by Aditya Vikrant and Rohil Doshi

   
