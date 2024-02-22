# Operating System Enhancing Kernel (Copy-on-Write Fork in xv6)

## Project Overview

The goal of this project was to modify the existing fork() implementation in xv6, an educational operating system developed by MIT to teach operating systems concepts. The current implementation involved a simple copy of each page in the address space. We decided to optimize this process by implementing a copy-on-write (CoW) approach, which defers the actual allocation and copying of physical memory pages for the child process until the copies are actually needed.

## Features

- Creation of system call that retrieves the current count of free pages in the system. This was crucial for debugging and testing.
- Implementation of reference count tracking which helps to monitor the number of processes pointing to a specific page.
- Modification of fork() function to use copy-on-write (CoW), thus delaying memory allocation and copy until needed.

## Importance

This project served as an excellent opportunity to delve deeper into Unix-style forking and x86 memory management. It further solidified our understanding of C programming, operating system concepts and it showcased our ability to improve system performance through the implementation of copy-on-write (CoW). Overall, it is a demonstration of our skills in systems programming and memory management.
