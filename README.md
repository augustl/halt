# HALT

See this blog post for a conceptual overview: http://augustl.com/blog/2014/an_immutable_operating_system/

Halt is a (work in progress) operating system, and very much a learning project. The goal is to have a proof of concept can run on qemu and at least my own PC.

The name HALT is a retrofitted acronym. These definitions are subject to change in the future, whenever I come up with something that sounds cooler.

* H: Hardware. Or rather, the abstraction of it. We will deliberately prefer immutability over mechanical sympathy.
* A: Asynchronous. Blocking calls and threads is a horrible abstraction.
* L: Lisp. The system language.
* T: Time. All values will be immutable, giving us a good time model.

# Not UNIX

Halt is not UNIX. The goal is to create something completely new. See the blog post :)
