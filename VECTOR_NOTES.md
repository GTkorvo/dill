# DILL_Q: 128-bit vector support

Design notes for the NEON/SSE vector extension (branch `neon-vector-ops`).
Motivating client: fused derived-expression loops in ADIOS2 (element-wise
float/double kernels), where dill's scalar output loses to compiler-vectorized
library code.

## Type

One new atomic type, `DILL_Q`: an opaque 128-bit value living in an FP/vector
register. Lane interpretation (4 x float vs 2 x double) is carried by the
*operation*, not the type, mirroring how the hardware works. `DILL_Q` is
appended to the type enum after `DILL_EC` and before `DILL_ERR`, which keeps
every existing type index stable. All per-arch `type_size`/`type_align`
tables get entries (16/16); on arches without vector encoders the entries are
inert, since nothing creates `DILL_Q` vregs there.

Constraint checked: `iclass_loadstore` packs the type into 4 bits of
`insn_code` (`& 0xf`, 0x10 = store, 0x20 = bswap). `DILL_Q` = 14 still fits.

## Operations

Data movement rides the existing fixed jump-table entries, keyed by type:

- `dill_ldq/ldqi/stq/stqi` — 16-byte load/store, lane-agnostic (base.ops
  `&loadstore` gains letter `q`)
- `dill_movq` — full-register move (`&mov` gains `q`)
- no `dill_setq` (no 128-bit immediates; use scalar set + splat), no bswap

New arith families appended at the END of the a3/a2 index spaces (arch .ops
files fill slots by name, so unimplementing arches just leave NULLs):

- a3: `vadd vsub vmul vdiv` x `f d`  (operands all DILL_Q)
- a3: `vfma` x `f d` — ACCUMULATING: `dest += src1*src2` (NEON FMLA, fused
  single-rounding, matches C fma()/fmaf()).  Dill's only read-modify-write
  op: liveness and the optimizers treat dest as also-used (see
  `is_accum_insn` in virtual.c — insn_uses/build_bb_body add the dest use;
  const-prop, mov back-prop, and same-except-dest elimination are barred).
- a2: `vneg vsqrt` x `f d`           (operands all DILL_Q)
- a2: `vsplat` x `f d`               (dest DILL_Q, src scalar DILL_F/D; DUP)
- a2: `sqrt` x `f d`                 (SCALAR sqrt — hardware has it, spares
                                      the libm call TryFuse pays today)

Capability query: `dill_has_vector_ops(dill_stream s)` — checks the *native*
target's jump table (in virtual mode `s->j` is the always-filled virtual
table, so the query must look through to the emit-time table). Clients fall
back to scalar loops when false.

## Virtual mode

- virtual.ops: record-only entries for the new families (reuse
  dill_varith3/varith2 recorders); NO vm emulation cases initially — the
  libffi/vm path reports the ops unsupported.
- Register allocation: `DILL_Q` joins `DILL_F/DILL_D` in the fpregs class
  (all `case DILL_F:` switches in virtual.c). Spill slots come from
  `dill_local(c, DILL_Q)` = 16 bytes via the type tables; spill/reload go
  through `j->storei/loadi(DILL_Q)` = full-width q/xmm moves.

### Callee-saved hazard (the one subtle rule)

AAPCS64 preserves only the LOW 64 bits of v8-v15 across calls. arm64's
`var_f` allocation pool is exactly v8-v15, so a `DILL_Q` value assigned there
would silently lose its top lanes at any `dill_scall`. Rule: `DILL_Q` vregs
allocate from the tmp (caller-saved) FP pool only, and are spilled around
calls like any tmp value. (x86_64 SysV: all xmm caller-saved, no issue;
Win64 preserves full xmm6-15, no issue.)

## arm64 encoders (first target)

All fixed 32-bit words, fields OR'd in, same style as existing emitters:

- `FADD/FSUB/FMUL/FDIV Vd.<T>, Vn.<T>, Vm.<T>` (T = 4S/2D)
- `FMLA Vd.<T>, Vn.<T>, Vm.<T>` (accumulating)
- `FNEG/FSQRT Vd.<T>, Vn.<T>`
- `DUP Vd.4S, Vn.S[0]` / `DUP Vd.2D, Vn.D[0]` (splat)
- `FSQRT Sd,Sn / Dd,Dn` (scalar)
- `LDR/STR Qt, [Xn, #imm]` (+ unscaled/reg-offset fallbacks, unaligned OK)
- mov: `ORR Vd.16B, Vn.16B, Vn.16B`

Disassembly verification comes free via the binutils disassembler hookup.

## Status (2026-08-17)

Implemented and passing on arm64/macOS: base.ops/virtual.ops plumbing, RA
integration, NEON encoders, `vtests/vec.c` (binops, unops, splat, scalar
sqrt, fused-magnitude loop with scalar remainder, vfma straight-line + a
loop-carried dot-product accumulator, 20-vector spill across a call). 19/19
ctest, ASAN-clean, 120x soak clean. pregen-source regenerated
(and now includes dill_arm64.c, previously missing).

Gotcha worth remembering: `dill_scall*`'s arg string drives its OWN vararg
push (register numbers passed as varargs after the string). Manually calling
dill_push_init/push_arg* before dill_scall* and omitting the varargs is UB
that corrupts the stream (cost a day of heisenbug chasing in the test).
ADIOS's TryFuse WIP makes exactly this mistake with its libm wrapper calls;
fix it when porting.

## Explicitly out of scope (for now)

- x86_64 encoders (next step after arm64 proves out; SSE2 baseline:
  movups/addps/subps/mulps/divps/sqrtps/shufps)
- AVX/SVE wider-than-128 widths, gathers, masked ops, integer lanes,
  vector byteswap, vector calls (SLEEF), vm-mode emulation
