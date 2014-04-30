# HALT

See this blog post for a conceptual overview: http://augustl.com/blog/2014/an_immutable_operating_system/

Halt is a (work in progress) operating system. It's mostly a playground for now - I still learning C, assembly and other basics.

* H: Hardware. We don't care about mechanical sympathy for a specific architecture.
* A: Asynchronous. Blocking calls and threads is a horrible abstraction.
* L: Lisp. The system language.
* T: Time. All values will be immutable, giving us a good time model.

# Not UNIX

Halt is not UNIX. The goal is to create something completely new. See the blog post :)
