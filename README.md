# HALT

Halt is a (work in progress) operating system.

* H: Hardware. We don't care about mechanical sympathy for a specific architecture.
* A: Asynchronous. Blocking calls and threads is a horrible abstraction.
* L: Lisp. The system language.
* T: Time. All values will be immutable, giving us a good time model.

# Not UNIX

Halt is not UNIX. The goal is to create something completely new.

# Immutability

All reads in the HALT userspace will return immutable values. Even things like the bytes representing the data for an IP packet. You will never be able to mutate a value. You will have things like atoms, that point to an immutable value, and can be changed to point to another immutable value. allowing you to model the world as a succession of immutable values. But most programs will have very few or no  atoms.

This means data can safely be shared across processes. The only thing that needs protection are the atoms. If you have a value and want another process/sandbox to see it, just pass it in. It's guaranteed by HALT to be immutable.

Garbage collection will also be interesting when everything is immutable. How much synchronization and stop-the-world do you need when all values are immutable? None at all!

Process forking will also be easy. In traditional operating systems, very clever copy-on-write semantics make a fork no-op, memory will only be coped ince the parent or child process mutates it. When everything is immutable, though, the only thing that needs monitoring is the atoms. Since there will probably be few of them per process (in the low 100s), the plan for now is to just copy all the atoms on fork. Since everything else is immutable, no further copying should  be needed.

# Business logic

The kernel should be as small as possible. The goal of the kernel is to abstract away today's hardware for the HALT runtime. As much as possible of the libraries and userspace should be implemented in Lisp, such as the IP stack.
