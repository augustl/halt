# HALT Roadmap

This roadmap describes the current execution strategy for HALT.

HALT is a research project. The objective is not feature count, but validating or falsifying architecture ideas around immutable state, atoms, and concurrency.

## Guiding strategy

- Keep bare-metal work central.
- Use library-level tests to accelerate iteration and confidence.
- Build core runtime logic so it works against a preallocated contiguous memory arena.
- Prefer short milestones with explicit pass/fail criteria.

## Milestone sequence

## M0 - Bootstrap reliability

Goal: make kernel bring-up repeatable and observable.

- Improve serial/panic diagnostics.
- Keep a deterministic QEMU boot loop.
- Define stage markers for early boot.

Done when:

- Boot logs clearly show progress through startup stages.
- Failures are diagnosable without guessing.

## M1 - Bare-metal memory foundations

Goal: establish trusted low-level memory primitives.

- Parse firmware/boot memory map.
- Implement a physical frame allocator.
- Establish page-table management basics.

Done when:

- Allocation and mapping operations are repeatable under QEMU.
- Basic negative tests fail cleanly.

## M2 - Core immutable runtime (library level)

Goal: validate HALT semantics independent of platform plumbing.

- Implement value model over a fixed contiguous arena.
- Implement atom abstraction (`load`, `swap`, compare-and-swap style updates).
- Add invariants around snapshot consistency and atomic updates.

Done when:

- Tests show readers never observe torn state.
- Atom update semantics are deterministic under concurrent stress tests.

## M3 - Allocation and GC experiments

Goal: get working memory management before optimization.

- Start with simple allocator strategy suitable for experiments.
- Implement a first GC strategy that works with immutable values.
- Benchmark pauses/allocation behavior on synthetic workloads.

Done when:

- GC can reclaim memory in repeatable test scenarios.
- Tradeoffs are documented.

## M4 - Process/task model prototype

Goal: test process semantics implied by immutable values.

- Define process representation around atoms + immutable value graphs.
- Prototype cheap fork semantics conceptually, then implement minimal form.
- Clarify ownership/protection model for shared atoms.

Done when:

- Process behavior is demonstrable with small, deterministic tests.
- Fork and sharing semantics are documented with examples.

## M5 - Kernel integration of runtime semantics

Goal: move validated runtime mechanisms onto bare metal incrementally.

- Integrate arena-based runtime into kernel memory layer.
- Preserve clear boundaries between arch-specific code and core runtime.
- Add in-kernel self-tests runnable under QEMU.

Done when:

- Core immutable state operations work in-kernel under QEMU.
- Regression tests cover core invariants.

## M6 - Architecture expansion

Goal: expand confidence and portability after core semantics are stable.

- Harden x86_64 path.
- Add arm64 target support.
- Compare behavior across architectures.

Done when:

- Same runtime invariants hold on both architectures.
- Portability issues are documented with mitigation strategy.

## Tooling direction

Current default direction:

- LLVM-based stack (`clang`, `ld.lld`, `llvm-objcopy`, `llvm-objdump`, `llvm-readelf`).
- QEMU for integration testing and early automation.
- Hosted/unit-style tests for runtime semantics where bare-metal feedback loops are too slow.

These are implementation choices, not immutable project principles.

## Planning cadence

- Keep milestones short (roughly 1-2 weeks each).
- Every milestone should end with a concrete artifact: logs, test output, benchmark, or design note.
- Update this roadmap as assumptions change.
