# HALT - Design Goals and Contributor Guidance

HALT is a research project exploring what an operating system could look like if all values are immutable.

This document summarizes the project philosophy and contribution guidance, based on the original project notes and blog posts.

## Project Status

HALT is intentionally experimental and early-stage. The current repository mostly contains low-level bootstrapping work (Rust `no_std`, basic entrypoint, VGA text output, and interrupt table setup), not a full implementation of the full architecture vision yet.

## Core Design Goals

- Treat state as a sequence of immutable snapshots rather than mutable memory.
- Keep mutation isolated to controlled indirection points ("atoms"), not in-place updates of values.
- Make safe concurrency the default model by eliminating shared mutable data as a programming primitive.
- Prefer value-oriented semantics over place-oriented semantics (no pointer arithmetic in the system language model).
- Explore architecture ideas first, practicality second: HALT is a research vehicle, not a production OS roadmap.

## Research posture

- Optimize for learning per unit time, not feature count.
- Keep experiments small and falsifiable.
- Prefer deleting weak ideas quickly over polishing speculative implementations.
- Record tradeoffs and failed approaches so the project accumulates knowledge.

## State Model: Immutable Values + Atoms

HALT’s intended state model follows a Clojure-like design:

- Values are immutable: bytes, collections, and higher-level structures are never changed in place.
- Atoms are mutable references: an atom points to a current immutable value.
- Updating state means atomically swapping an atom to a new value (with CAS-style semantics when needed).
- Readers always observe complete immutable values, never partially-written state.

This provides deterministic snapshots, safer sharing, and simpler reasoning under concurrency.

## Memory and Process Model

The architecture aims to make immutability a first-class OS primitive:

- Immutable values can be shared across processes safely without defensive copying.
- Process fork can become very cheap, because immutable data does not need copy-on-write for safety.
- Protection and synchronization concerns center on atoms/reference ownership, not raw memory mutation.
- The long-term language/runtime direction avoids exposed pointers to preserve value semantics.

## Garbage Collection as a Kernel-Level Concern

HALT assumes garbage collection is fundamental in an immutable system:

- The collector can exploit immutability for safer relocation/defragmentation strategies.
- In principle, values can be copied and internal references swapped without exposing torn state.
- There is interest in GC and representation optimizations that can happen with reduced global pauses.

## Concurrency and UI Origins

HALT grew out of earlier work on truly concurrent immutable UI rendering:

- Model UI (and, later, system state) as immutable data observed by dedicated worker threads.
- Allow many concurrent writers through transactional or atomic update semantics.
- Favor consistency and composability over manual lock choreography.

This origin explains the strong emphasis on snapshot-based reasoning and parallelism.

## Performance Philosophy

HALT does not optimize around conventional mutable-memory assumptions first. Instead, it asks:

- What hardware/software co-design opportunities appear when immutability is guaranteed?
- Can cache behavior, sharing, and synchronization become simpler under immutable data?
- Which traditional OS costs (defensive copying, COW edge cases, lock-heavy coordination) can be reduced?

Performance matters, but the research goal is to discover new tradeoffs, not copy existing OS patterns.

## Storage and Language Direction (Exploratory)

The original direction includes open questions rather than fixed decisions:

- Durable storage may be a file system, append-only store, or key/value model with immutable semantics.
- The system language is intended to be Lisp-like and value-centric.
- Hot deploy/code updates are easier to reason about when code is immutable too.

These are active research areas, not finalized design commitments.

## Working technical direction (current)

These are current defaults, not permanent commitments:

- Prioritize bare-metal progress, with library-level tests as fast validation.
- Keep core runtime logic portable and testable with a preallocated contiguous memory arena.
- Target x86_64 first for momentum and tooling maturity; add arm64 once core semantics stabilize.
- Favor a lean systems stack over complex language machinery.
- Prefer LLVM-based tooling for cross-target work (clang/lld/llvm-*), while keeping build artifacts explicit and inspectable.

## Implementation Strategy

A pragmatic staging strategy was proposed:

1. Build enough runtime/VM mechanisms to test core immutable ideas quickly.
2. Validate allocator + GC + process semantics in a constrained environment.
3. Move toward bare metal where it adds learning value and architectural clarity.

The repository currently reflects this "bootstrap first, prove concepts incrementally" approach.

## What Contributors Should Optimize For

When adding work to HALT, optimize for:

- preserving immutable/value semantics,
- making concurrency simpler and safer,
- producing clear experiments that validate or falsify core hypotheses,
- documenting tradeoffs and failed approaches.

HALT is successful when it increases understanding, even if experiments are discarded.

## Documentation boundaries

- Keep this file focused on principles and contributor behavior.
- Put milestone plans and sequencing in `docs/ROADMAP.md`.
- Put implementation model details in `docs/ARCHITECTURE.md`.
