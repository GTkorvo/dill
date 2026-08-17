/* Tests for the DILL_Q 128-bit vector ops (virtual mode, register allocation).
 *
 * On targets without vector encoders, dill_has_vector_ops() is false and the
 * test passes vacuously (prints "no vector support").
 */
#include "../config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dill.h"

static int verbose = 0;
static int failed = 0;

static void
check_f(const char *name, const float *got, const float *want, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        if (got[i] != want[i]) {
            printf("%s: elem %d got %g, expected %g\n", name, i, got[i],
                   want[i]);
            failed = 1;
            return;
        }
    }
    if (verbose)
        printf("%s OK\n", name);
}

static void
check_d(const char *name, const double *got, const double *want, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        if (got[i] != want[i]) {
            printf("%s: elem %d got %g, expected %g\n", name, i, got[i],
                   want[i]);
            failed = 1;
            return;
        }
    }
    if (verbose)
        printf("%s OK\n", name);
}

typedef void (*binop_func)(void *, void *, void *);
typedef void (*emit_func)(dill_stream, int, int, int);

/* Build void(*)(void *a, void *b, void *out): one 128-bit binary vector op. */
static binop_func
gen_binop(dill_stream s, dill_exec_handle *hp, emit_func emit)
{
    dill_reg pa, pb, po, va, vb, vr;
    dill_start_proc(s, "vbin", DILL_V, "%p%p%p");
    pa = dill_vparam(s, 0);
    pb = dill_vparam(s, 1);
    po = dill_vparam(s, 2);
    va = dill_getreg(s, DILL_Q);
    vb = dill_getreg(s, DILL_Q);
    vr = dill_getreg(s, DILL_Q);
    dill_ldqi(s, va, pa, 0);
    dill_ldqi(s, vb, pb, 0);
    emit(s, vr, va, vb);
    dill_stqi(s, vr, po, 0);
    dill_retii(s, 0);
    *hp = dill_finalize(s);
    return (binop_func)dill_get_fp(*hp);
}

static void e_vaddf(dill_stream s, int d, int a, int b) { dill_vaddf(s, d, a, b); }
static void e_vsubf(dill_stream s, int d, int a, int b) { dill_vsubf(s, d, a, b); }
static void e_vmulf(dill_stream s, int d, int a, int b) { dill_vmulf(s, d, a, b); }
static void e_vdivf(dill_stream s, int d, int a, int b) { dill_vdivf(s, d, a, b); }
static void e_vaddd(dill_stream s, int d, int a, int b) { dill_vaddd(s, d, a, b); }
static void e_vsubd(dill_stream s, int d, int a, int b) { dill_vsubd(s, d, a, b); }
static void e_vmuld(dill_stream s, int d, int a, int b) { dill_vmuld(s, d, a, b); }
static void e_vdivd(dill_stream s, int d, int a, int b) { dill_vdivd(s, d, a, b); }

static void
test_binops(void)
{
    float fa[4] = {1.5f, -2.0f, 3.25f, 100.0f};
    float fb[4] = {0.5f, 4.0f, -1.25f, 8.0f};
    float fo[4], fw[4];
    double da[2] = {1.5, -200.0};
    double db[2] = {0.25, 8.0};
    double dout[2], dw[2];
    struct {
        const char *name;
        emit_func emit;
        char op;
        int dbl;
    } cases[] = {
        {"vaddf", e_vaddf, '+', 0}, {"vsubf", e_vsubf, '-', 0},
        {"vmulf", e_vmulf, '*', 0}, {"vdivf", e_vdivf, '/', 0},
        {"vaddd", e_vaddd, '+', 1}, {"vsubd", e_vsubd, '-', 1},
        {"vmuld", e_vmuld, '*', 1}, {"vdivd", e_vdivd, '/', 1},
    };
    size_t c;
    for (c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        dill_stream s = dill_create_stream();
        dill_exec_handle h;
        binop_func f = gen_binop(s, &h, cases[c].emit);
        int i;
        if (cases[c].dbl) {
            f(da, db, dout);
            for (i = 0; i < 2; i++) {
                switch (cases[c].op) {
                case '+': dw[i] = da[i] + db[i]; break;
                case '-': dw[i] = da[i] - db[i]; break;
                case '*': dw[i] = da[i] * db[i]; break;
                case '/': dw[i] = da[i] / db[i]; break;
                }
            }
            check_d(cases[c].name, dout, dw, 2);
        } else {
            f(fa, fb, fo);
            for (i = 0; i < 4; i++) {
                switch (cases[c].op) {
                case '+': fw[i] = fa[i] + fb[i]; break;
                case '-': fw[i] = fa[i] - fb[i]; break;
                case '*': fw[i] = fa[i] * fb[i]; break;
                case '/': fw[i] = fa[i] / fb[i]; break;
                }
            }
            check_f(cases[c].name, fo, fw, 4);
        }
        dill_free_handle(h);
        dill_free_stream(s);
    }
}

static void
test_unops(void)
{
    float fa[4] = {4.0f, 9.0f, 2.25f, 100.0f};
    float fo[4], fw[4];
    double da[2] = {16.0, 0.0625};
    double dout[2], dw[2];
    int i;

    {
        dill_stream s = dill_create_stream();
        dill_exec_handle h;
        dill_reg pa, po, va, vr;
        void (*f)(void *, void *);
        dill_start_proc(s, "vun", DILL_V, "%p%p");
        pa = dill_vparam(s, 0);
        po = dill_vparam(s, 1);
        va = dill_getreg(s, DILL_Q);
        vr = dill_getreg(s, DILL_Q);
        dill_ldqi(s, va, pa, 0);
        dill_vsqrtf(s, vr, va);
        dill_vnegf(s, vr, vr);
        dill_stqi(s, vr, po, 0);
        dill_retii(s, 0);
        h = dill_finalize(s);
        f = (void (*)(void *, void *))dill_get_fp(h);
        f(fa, fo);
        for (i = 0; i < 4; i++)
            fw[i] = -sqrtf(fa[i]);
        check_f("vsqrtf+vnegf", fo, fw, 4);
        dill_free_handle(h);
        dill_free_stream(s);
    }
    {
        dill_stream s = dill_create_stream();
        dill_exec_handle h;
        dill_reg pa, po, va, vr;
        void (*f)(void *, void *);
        dill_start_proc(s, "vund", DILL_V, "%p%p");
        pa = dill_vparam(s, 0);
        po = dill_vparam(s, 1);
        va = dill_getreg(s, DILL_Q);
        vr = dill_getreg(s, DILL_Q);
        dill_ldqi(s, va, pa, 0);
        dill_vsqrtd(s, vr, va);
        dill_vnegd(s, vr, vr);
        dill_stqi(s, vr, po, 0);
        dill_retii(s, 0);
        h = dill_finalize(s);
        f = (void (*)(void *, void *))dill_get_fp(h);
        f(da, dout);
        for (i = 0; i < 2; i++)
            dw[i] = -sqrt(da[i]);
        check_d("vsqrtd+vnegd", dout, dw, 2);
        dill_free_handle(h);
        dill_free_stream(s);
    }
}

static void
test_splat(void)
{
    /* out[i] = in[i] * k, k broadcast from a scalar parameter register */
    float fa[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float fo[4], fw[4];
    double da[2] = {1.0, -2.0};
    double dout[2], dw[2];
    int i;
    {
        dill_stream s = dill_create_stream();
        dill_exec_handle h;
        dill_reg pa, po, k, vk, va;
        void (*f)(void *, void *, float);
        dill_start_proc(s, "vsplf", DILL_V, "%p%p%f");
        pa = dill_vparam(s, 0);
        po = dill_vparam(s, 1);
        k = dill_vparam(s, 2);
        vk = dill_getreg(s, DILL_Q);
        va = dill_getreg(s, DILL_Q);
        dill_vsplatf(s, vk, k);
        dill_ldqi(s, va, pa, 0);
        dill_vmulf(s, va, va, vk);
        dill_stqi(s, va, po, 0);
        dill_retii(s, 0);
        h = dill_finalize(s);
        f = (void (*)(void *, void *, float))dill_get_fp(h);
        f(fa, fo, 2.5f);
        for (i = 0; i < 4; i++)
            fw[i] = fa[i] * 2.5f;
        check_f("vsplatf", fo, fw, 4);
        dill_free_handle(h);
        dill_free_stream(s);
    }
    {
        dill_stream s = dill_create_stream();
        dill_exec_handle h;
        dill_reg pa, po, k, vk, va;
        void (*f)(void *, void *, double);
        dill_start_proc(s, "vspld", DILL_V, "%p%p%d");
        pa = dill_vparam(s, 0);
        po = dill_vparam(s, 1);
        k = dill_vparam(s, 2);
        vk = dill_getreg(s, DILL_Q);
        va = dill_getreg(s, DILL_Q);
        dill_vsplatd(s, vk, k);
        dill_ldqi(s, va, pa, 0);
        dill_vmuld(s, va, va, vk);
        dill_stqi(s, va, po, 0);
        dill_retii(s, 0);
        h = dill_finalize(s);
        f = (void (*)(void *, void *, double))dill_get_fp(h);
        f(da, dout, 3.0);
        for (i = 0; i < 2; i++)
            dw[i] = da[i] * 3.0;
        check_d("vsplatd", dout, dw, 2);
        dill_free_handle(h);
        dill_free_stream(s);
    }
}

static void
test_scalar_sqrt(void)
{
    dill_stream s = dill_create_stream();
    dill_exec_handle h;
    dill_reg p, r;
    double (*f)(double);
    dill_start_proc(s, "ssqrt", DILL_D, "%d");
    p = dill_vparam(s, 0);
    r = dill_getreg(s, DILL_D);
    dill_sqrtd(s, r, p);
    dill_retd(s, r);
    h = dill_finalize(s);
    f = (double (*)(double))dill_get_fp(h);
    if (f(1764.0) != 42.0) {
        printf("scalar sqrtd: got %g, expected 42\n", f(1764.0));
        failed = 1;
    } else if (verbose) {
        printf("sqrtd OK\n");
    }
    dill_free_handle(h);
    dill_free_stream(s);
}

/* The fused-expression shape: out[i] = sqrt(a[i]^2 + b[i]^2 + c[i]^2) over a
 * vector body with a scalar remainder loop.  Exercises loops, per-iteration
 * addressing, and vector register use across basic blocks. */
static void
test_mag_loop(void)
{
    enum { N = 1027 }; /* not a multiple of 4: remainder loop must run */
    static float a[N], b[N], c[N], out[N], want[N];
    dill_stream s = dill_create_stream();
    dill_exec_handle h;
    dill_reg pa, pb, pc, po, n, i, off, nvec, mask;
    dill_reg va, vb, vc, vr;
    dill_reg fa, fb, fc, fr;
    void (*f)(void *, void *, void *, void *, size_t);
    int loop_top, loop_end, tail_top, tail_end;
    int k;

    for (k = 0; k < N; k++) {
        a[k] = (float)(k % 37) * 0.5f;
        b[k] = (float)(k % 11) - 5.0f;
        c[k] = (float)(k % 7) * 1.25f;
        want[k] = sqrtf(a[k] * a[k] + b[k] * b[k] + c[k] * c[k]);
    }

    dill_start_proc(s, "mag", DILL_V, "%p%p%p%p%ul");
    pa = dill_vparam(s, 0);
    pb = dill_vparam(s, 1);
    pc = dill_vparam(s, 2);
    po = dill_vparam(s, 3);
    n = dill_vparam(s, 4);

    i = dill_getreg(s, DILL_UL);
    off = dill_getreg(s, DILL_UL);
    nvec = dill_getreg(s, DILL_UL);
    mask = dill_getreg(s, DILL_UL);
    va = dill_getreg(s, DILL_Q);
    vb = dill_getreg(s, DILL_Q);
    vc = dill_getreg(s, DILL_Q);
    vr = dill_getreg(s, DILL_Q);

    /* nvec = n & ~3 (whole vectors) */
    dill_setul(s, mask, ~(size_t)3);
    dill_andul(s, nvec, n, mask);
    dill_setul(s, i, 0);

    loop_top = dill_alloc_label(s, "loop_top");
    loop_end = dill_alloc_label(s, "loop_end");
    dill_mark_label(s, loop_top);
    dill_bgeul(s, i, nvec, loop_end);

    dill_mululi(s, off, i, sizeof(float));
    dill_ldq(s, va, pa, off);
    dill_ldq(s, vb, pb, off);
    dill_ldq(s, vc, pc, off);
    dill_vmulf(s, va, va, va);
    dill_vmulf(s, vb, vb, vb);
    dill_vmulf(s, vc, vc, vc);
    dill_vaddf(s, vr, va, vb);
    dill_vaddf(s, vr, vr, vc);
    dill_vsqrtf(s, vr, vr);
    dill_stq(s, vr, po, off);

    dill_adduli(s, i, i, 4);
    dill_jv(s, loop_top);
    dill_mark_label(s, loop_end);

    /* scalar remainder */
    fa = dill_getreg(s, DILL_F);
    fb = dill_getreg(s, DILL_F);
    fc = dill_getreg(s, DILL_F);
    fr = dill_getreg(s, DILL_F);
    tail_top = dill_alloc_label(s, "tail_top");
    tail_end = dill_alloc_label(s, "tail_end");
    dill_mark_label(s, tail_top);
    dill_bgeul(s, i, n, tail_end);
    dill_mululi(s, off, i, sizeof(float));
    dill_ldf(s, fa, pa, off);
    dill_ldf(s, fb, pb, off);
    dill_ldf(s, fc, pc, off);
    dill_mulf(s, fa, fa, fa);
    dill_mulf(s, fb, fb, fb);
    dill_mulf(s, fc, fc, fc);
    dill_addf(s, fr, fa, fb);
    dill_addf(s, fr, fr, fc);
    dill_sqrtf(s, fr, fr);
    dill_stf(s, fr, po, off);
    dill_adduli(s, i, i, 1);
    dill_jv(s, tail_top);
    dill_mark_label(s, tail_end);
    dill_retii(s, 0);

    h = dill_finalize(s);
    f = (void (*)(void *, void *, void *, void *, size_t))dill_get_fp(h);
    f(a, b, c, out, (size_t)N);
    check_f("mag_loop", out, want, N);
    dill_free_handle(h);
    dill_free_stream(s);
}

/* Enough simultaneously-live vectors to force 16-byte spills, plus a call in
 * the middle so live vectors cross a basic-block/call boundary. */
static double
clobber(double x)
{
    return x * 0.0;
}

static void
test_spill_and_call(void)
{
    enum { NV = 20 };
    float in[4 * NV], out[4];
    float want[4] = {0, 0, 0, 0};
    dill_stream s = dill_create_stream();
    dill_exec_handle h;
    dill_reg pi, po, acc, tmp;
    dill_reg v[NV];
    void (*f)(void *, void *);
    int k, j;

    for (k = 0; k < 4 * NV; k++)
        in[k] = (float)(k + 1) * 0.25f;
    for (j = 0; j < 4; j++)
        for (k = 0; k < NV; k++)
            want[j] += in[4 * k + j];

    dill_start_proc(s, "spill", DILL_V, "%p%p");
    pi = dill_vparam(s, 0);
    po = dill_vparam(s, 1);
    for (k = 0; k < NV; k++) {
        v[k] = dill_getreg(s, DILL_Q);
        dill_ldqi(s, v[k], pi, 16 * k);
    }
    /* a call: every vector above must survive it */
    {
        dill_reg darg = dill_getreg(s, DILL_D);
        dill_setd(s, darg, 1.0);
        dill_scalld(s, (void *)clobber, "clobber", "%d", darg);
    }
    acc = dill_getreg(s, DILL_Q);
    tmp = dill_getreg(s, DILL_Q);
    dill_movq(s, acc, v[0]);
    for (k = 1; k < NV; k++) {
        dill_vaddf(s, tmp, acc, v[k]);
        dill_movq(s, acc, tmp);
    }
    dill_stqi(s, acc, po, 0);
    dill_retii(s, 0);

    h = dill_finalize(s);
    f = (void (*)(void *, void *))dill_get_fp(h);
    f(in, out);
    check_f("spill_and_call", out, want, 4);
    dill_free_handle(h);
    dill_free_stream(s);
}

int
main(int argc, char **argv)
{
    dill_stream probe;
    int has_vec;
    if (argc > 1 && strcmp(argv[1], "-v") == 0)
        verbose++;

    probe = dill_create_stream();
    has_vec = dill_has_vector_ops(probe);
    dill_free_stream(probe);
    if (!has_vec) {
        printf("no vector support on this target; test skipped\n");
        printf("success!\n");
        return 0;
    }

    test_binops();
    test_unops();
    test_splat();
    test_scalar_sqrt();
    test_mag_loop();
    test_spill_and_call();

    if (!failed)
        printf("success!\n");
    return failed;
}
