# HALT Architecture (Working Model)

This document captures HALT's current architecture model.

It is a working design for experimentation, not a finalized specification.

## Core model

HALT centers on immutable values and controlled mutation:

- Values are immutable once created.
- Mutable state exists only through atom-like references to immutable values.
- State evolution is represented as swaps from old immutable values to new immutable values.

The goal is to make concurrency easier to reason about by construction.

## State semantics

- A read from an atom returns one complete immutable value.
- Updates create new values and attempt an atomic pointer/reference swap.
- Compare-and-swap style updates are used for conditional writes.
- Readers should never observe partially written values.

This model follows the same conceptual lineage as Clojure atoms and immutable persistent structures.

## Memory model

## Value memory

- Values live in managed memory.
- In-place mutation of value payloads is not exposed as a programming primitive.
- Internal temporary mutation may be used only as an implementation optimization if external semantics remain immutable.

## Atom memory

- Atoms are mutable indirection points with synchronization semantics.
- Access control and sharing rules focus on atoms, not immutable value payloads.

## Sharing and process implications

- Immutable values can be shared across processes safely.
- Fork can be cheaper than traditional mutable-memory models because shared immutable graphs do not need defensive copy-on-write for correctness.
- Coordination and protection become primarily about atom ownership, visibility, and update rights.

## Garbage collection

GC is a first-class concern in HALT's model:

- Managed memory is expected, not optional.
- Immutability should allow safer relocation/defragmentation strategies.
- Initial GC strategy should prioritize correctness and observability over sophistication.
- Advanced representation and relocation optimizations are future research topics.

## Runtime layering

HALT uses a layered implementation strategy.

## Core runtime layer

- Platform-agnostic logic for values, atoms, and memory management.
- Must run against a caller-provided contiguous preallocated arena.
- Used for fast library-level tests of semantics.

## Kernel integration layer

- Boot, interrupts, paging, physical memory mapping, and architecture interfaces.
- Integrates and hosts the core runtime layer on bare metal.

## Architecture-specific layer

- Minimal target-specific code and ABI details.
- Current momentum target: x86_64.
- Planned follow-up target: arm64.

## Toolchain direction

Current working direction is an LLVM-centric C-oriented systems stack:

- `clang` for compilation and cross-target builds.
- `ld.lld` with explicit linker scripts for section and memory layout control.
- `llvm-objcopy`/`llvm-objdump`/`llvm-readelf` for artifact inspection and conversion.
- QEMU for deterministic bare-metal test loops.

This choice is pragmatic and may evolve.

## Non-goals (current phase)

- Shipping a production operating system.
- Finalizing file/storage model early.
- Premature optimization before semantic correctness.
- Committing to a permanent system language before runtime semantics are validated.

## Open questions

- Which durable storage abstraction best matches immutable system semantics?
- Which GC family gives the best tradeoff for immutable value workloads in kernel context?
- What atom sharing and capability model best balances safety and usability?
- Which language/runtime surface best expresses HALT's value-centric programming model?

These questions are expected to change as experiments produce data.
