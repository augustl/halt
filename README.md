# HALT

Halt is a (work in progress) operating system.

* H: Hardware. The OS does not aim to be close to today's hardware.
* A: AST. Files are rich data structures, not dumb bytes.
* L: Lisp. The system language.
* T: Time. All values will be immutable, giving us a good time model.

# Random thoughts

## Garbage collection

You do not manually allocate memory in HALT. All values are immutable values, implemented as persistent data structures. This means GC is required. However, how much synchronization do you need to GC when everything is immutable? None! A separate process (or even a separate core) can GC, without anyone ever knowing about it.

## Inter-process sharing

How dangerous is it to share immutable values to other processes? None at all! Instead of COW structures with lots of overhead, we can just pass our values to other processes. Only writable state (in the form of an atim that points to an immutable value) needs protection.

## Business logic

The goal is to have as much of the operating system logic as possible in the system language Lisp. Only a separate hardware abstraction layer (HAL) will be in C, that implements GC, data structures, etc. Things like TCP implementations, drivers, and so on, should be in Lisp. Because we don't want to tie the OS business logic to hardware.

This allows for hardware vendors to experiment as well. When all memory is immutable, perhaps something interesting can happen with memory architectures? In other words, some of the C code in HALT could be replaced with hardware implementations, of things like persistent data structures, GC, and what not.
