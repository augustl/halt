HALT is a research project exploring what an operating system could look like if all values are immutable.


- Treat state as a sequence of immutable snapshots rather than mutable memory.
- Keep mutation isolated to controlled indirection points ("atoms"), not in-place updates of values.
- Make safe concurrency the default model by eliminating shared mutable data as a programming primitive.
- Prefer value-oriented semantics over place-oriented semantics (no pointer arithmetic in the system language model).
- Explore architecture ideas first, practicality second: HALT is a research vehicle, not a production OS roadmap.
- HALT’s intended state model follows a Clojure-like design
- Readers always observe complete immutable values, never partially-written state.
- Immutable values can be shared across processes safely without defensive copying.
- Process fork can become very cheap, because immutable data does not need copy-on-write for safety.
- Protection and synchronization concerns center on atoms/reference ownership, not raw memory mutation.
- The long-term language/runtime direction avoids exposed pointers to preserve value semantics.
- HALT assumes garbage collection is fundamental in an immutable system
- The collector can exploit immutability for safer relocation/defragmentation strategies.
- Durable storage may be a file system, append-only store, or key/value model with immutable semantics.
- The system language is intended to be Lisp-like and value-centric.
