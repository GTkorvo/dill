#include "stdio.h"
#include "dill.h"

static int verbose = 0;

int gg(int a, int b) {
    printf("In gg  a=%d, b=%d\n", a, b);
     return a+b;
}
int ff(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    printf("In ff  a=%d, b=%d, c=%d, d=%d, e=%d, f=%d, g=%d, h=%d, i=%d, j=%d\n",
	   a, b, c, d, e, f, g, h, i, j);
     return a+b+c+d+e+f+g+h+i+j;
}

void a () {
    dill_stream c = dill_create_stream();
     dill_reg a,b,p3,d,e,f,g,h,i,j,w,z,cnt;
/*     dill_reg func;*/
     int L1;
     int (*ip)();

     dill_start_simple_proc(c, "a_gen", DILL_I);
     L1 = dill_alloc_label(c);
/*     func = dill_getreg(c, DILL_P);*/
     cnt = dill_getreg(c, DILL_P);
     a = dill_getreg(c, DILL_I);
     b = dill_getreg(c, DILL_I);
     p3 = dill_getreg(c, DILL_I);
     d = dill_getreg(c, DILL_I);
     e = dill_getreg(c, DILL_I);
     f = dill_getreg(c, DILL_I);
     g = dill_getreg(c, DILL_I);
     h = dill_getreg(c, DILL_I);
     i = dill_getreg(c, DILL_I);
     j = dill_getreg(c, DILL_I);
     z = dill_getreg(c, DILL_I);
     w = dill_getreg(c, DILL_I);

/*     dill_setp(c, func, ff);*/
     dill_seti(c, a, 1);
     dill_seti(c, b, 2);
     dill_seti(c, p3, 3);
     dill_seti(c, d, 4);
     dill_seti(c, e, 5);
     dill_seti(c, f, 6);
     dill_seti(c, g, 7);
     dill_seti(c, h, 8);
     dill_seti(c, i, 9);
     dill_seti(c, j, 0);
     dill_seti(c, z, 0);
     dill_seti(c, cnt, 0);
     dill_mark_label(c, L1);
     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argi(c, a);
	 dill_push_argi(c, b);
	 dill_push_argi(c, p3);
	 dill_push_argi(c, d);
	 dill_push_argi(c, e);
	 dill_push_argi(c, f);
	 dill_push_argi(c, g);
	 dill_push_argi(c, h);
	 dill_push_argi(c, i);
	 dill_push_argi(c, j);
     } else {
	 dill_push_argi(c, j);
	 dill_push_argi(c, i);
	 dill_push_argi(c, h);
	 dill_push_argi(c, g);
	 dill_push_argi(c, f);
	 dill_push_argi(c, e);
	 dill_push_argi(c, d);
	 dill_push_argi(c, p3);
	 dill_push_argi(c, b);
	 dill_push_argi(c, a);
     }	 
     w = dill_calli(c, (void*)ff, "ff");
     dill_addi(c, z,z,w);
     dill_addii(c, cnt,cnt,1);
     dill_bltii(c, cnt,2,L1);
     dill_addi(c, z, z, a);
     dill_addi(c, z, z, b);
     dill_addi(c, z, z, p3);
     dill_addi(c, z, z, d);
     dill_addi(c, z, z, e);
     dill_addi(c, z, z, f);
     dill_addi(c, z, z, g);
     dill_addi(c, z, z, h);
     dill_addi(c, z, z, i);
     dill_addi(c, z, z, j);
     dill_reti(c, z);			/* (9*10/2)*3 = 135 */
     ip = (int(*)())dill_finalize(c);

     if (verbose) dill_dump(c);

     printf("**135=%d\n", (*ip)());
     dill_free_context(c);
}

void b () {
     dill_reg f;
     void *(*pp)();
     
     dill_stream c = dill_create_stream();

     dill_start_proc(c, "b_gen", DILL_I, "%i%i");
     f = dill_getreg(c, DILL_P);
     dill_setp(c, f, gg);
     dill_retp(c, f);
     pp = (void *(*)())dill_finalize(c);

     if (verbose) dill_dump(c);

     printf("**3=%d\n", (*(int (*)(int, int))(long)((*pp)()))(1,2));
     dill_free_context(c);
}

void c () {
     dill_reg a,b,f;
     int (*ip)();
     
     dill_stream c = dill_create_stream();

     dill_start_simple_proc(c, "c_gen", DILL_I);
     a = dill_getreg(c, DILL_I);
     b = dill_getreg(c, DILL_I);
     f = dill_getreg(c, DILL_I);

     dill_setp(c, f, gg);
     dill_seti(c, a, 1);
     dill_seti(c, b, 2);  
     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argi(c, a);
	 dill_push_argi(c, b);
     } else {
	 dill_push_argi(c, b);
	 dill_push_argi(c, a);
     }
     a = dill_callri(c, f);
     dill_reti(c, a);
     ip = (int(*)())dill_finalize(c);

     if (verbose) dill_dump(c);

     printf("**3=%d\n", (*ip)());
     dill_free_context(c);
}

void d () {
     dill_reg a,b;
     int (*ip)();
     
     dill_stream c = dill_create_stream();

     dill_start_simple_proc(c, "d_gen", DILL_I);
     a = dill_getreg(c, DILL_I);
     b = dill_getreg(c, DILL_I);

     dill_seti(c, a, 1);
     dill_seti(c, b, 2);  
     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argi(c, a);
	 dill_push_argi(c, b);
     } else {
	 dill_push_argi(c, b);
	 dill_push_argi(c, a);
     }
     a = dill_calli(c, (void*)gg, "gg");
     dill_reti(c, a);
     ip = (int(*)())dill_finalize(c);

     if (verbose) dill_dump(c);

     printf("**3=%d\n", (*ip)());
     dill_free_context(c);
}

void e () {
     dill_reg a,b,p3,d,e,f,g,h,i,j,k,l;
     int (*ip)(int (*)(int,int),int,int,int,int,int,int,int,int,int);
     
     dill_stream c = dill_create_stream();

     dill_start_proc(c, "e_gen", DILL_I, "%p%i%i%i%i%i%i%i%i%i");
     f = dill_vparam(c, 0);
     a = dill_vparam(c, 1);
     b = dill_vparam(c ,2);
     p3 = dill_vparam(c, 3);
     d = dill_vparam(c, 4);
     e = dill_vparam(c, 5);
     g = dill_vparam(c, 6);
     h = dill_vparam(c, 7);
     i = dill_vparam(c, 8);
     j = dill_vparam(c, 9);
     k = dill_getreg(c, DILL_I);
     l = dill_getreg(c, DILL_I);

     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argi(c, a);
	 dill_push_argi(c, b);
     } else {
	 dill_push_argi(c, b);
	 dill_push_argi(c, a);
     }
     k = dill_callri(c, f);
     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argi(c, p3);
	 dill_push_argi(c, d);
     } else {
	 dill_push_argi(c, d);
	 dill_push_argi(c, p3);
     }
     d = dill_callri(c, f);
     dill_addi(c, k,k,d);
     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argi(c, e);
	 dill_push_argi(c, g);
     } else {
	 dill_push_argi(c, g);
	 dill_push_argi(c, e);
     }
     g = dill_callri(c, f);
     dill_addi(c, k,k,g);
     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argi(c, h);
	 dill_push_argi(c, i);
     } else {
	 dill_push_argi(c, i);
	 dill_push_argi(c, h);
     }
     i = dill_callri(c, f);
     dill_addi(c, k,k,i);
     dill_addi(c, k,k,j);
     dill_addii(c, l,k,3);
     dill_reti(c, l);
     ip = (int(*)())dill_finalize(c);

     if (verbose) dill_dump(c);

     printf("**48=%d\n", (*ip)(gg,1,2,3,4,5,6,7,8,9));
     dill_free_context(c);
}

void f () {
     dill_reg a, b, d;
     double dp = 3.14159;
     int ip = 5;
     char *pp = "hello!";
     void *(*proc)();
     
     dill_stream c = dill_create_stream();

     dill_start_proc(c, "f_gen", DILL_I, "");
     a = dill_getreg(c, DILL_D);
     b = dill_getreg(c, DILL_I);
     d = dill_getreg(c, DILL_P);
     dill_setd(c, a, dp);
     dill_seti(c, b, ip);
     dill_setp(c, d, pp);
     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argpi(c, (void*)"values are %d, %g, %s\n");
	 dill_push_argi(c, b);
	 dill_push_argd(c, a);
	 dill_push_argp(c, d);
     } else {
	 dill_push_argp(c, d);
	 dill_push_argd(c, a);
	 dill_push_argi(c, b);
	 dill_push_argpi(c, (void*)"values are %d, %g, %s\n");
     }
     a = dill_calli(c, (void*)printf, "printf");
     dill_reti(c, a);
     proc = (void *(*)())dill_finalize(c);

     if (verbose) dill_dump(c);

     printf("expect: values are %d, %g, %s\n", ip, dp, pp);
     proc();
     dill_free_context(c);
}

#ifdef NOTDEF
void g () {
     dill_reg a, b, d, e, f, g, h, i;
     double da = 3.1;
     double db = 4.1;
     double dd = 5.1;
     double de = 6.1;
     double df = 7.1;
     double dg = 8.1;
     double dh = 9.1;
     double di = 10.1;
     void *(*proc)();
     
     dill_stream c = dill_create_stream();

     dill_start_proc(c, "g_gen", "");
     a = dill_getreg(c, DILL_D);
     b = dill_getreg(c, DILL_D);
     d = dill_getreg(c, DILL_D);
     e = dill_getreg(c, DILL_D);
     f = dill_getreg(c, DILL_D);
     g = dill_getreg(c, DILL_D);
     h = dill_getreg(c, DILL_D);
     i = dill_getreg(c, DILL_D);
     dill_setd(c, a, da);
     dill_setd(c, b, db);
     dill_setd(c, d, dd);
     dill_setd(c, e, de);
     dill_setd(c, f, df);
     dill_setd(c, g, dg);
     dill_setd(c, h, dh);
     dill_setd(c, i, di);
     dill_push_init(c);
     if (!dill_do_reverse_vararg_push(c)) {
	 dill_push_argpi(c, (void*)"values are %g, %g, %g, %g, %g, %g, %g, %g, %g\n");
	 dill_push_argd(c, a);
	 dill_push_argd(c, b);
	 dill_push_argd(c, d);
	 dill_push_argd(c, e);
	 dill_push_argd(c, f);
	 dill_push_argd(c, g);
	 dill_push_argd(c, h);
	 dill_push_argd(c, i);
     }
     a = dill_calli(c, (void*)printf, "printf");
     dill_reti(c, a);
     proc = (void *(*)())dill_finalize(c);

     if (verbose) dill_dump(c);

     printf("expect: values are %g, %g, %g, %g, %g, %g, %g, %g\n", da, db, dd, de, df, dg, dh, di);
     proc();
     dill_free_context(c);
}
#endif

int 
main(int argc, char **argv)
{
    if (argc > 1) verbose++;
    printf("########## A\n");
    a();
    printf("########## B\n");
    b();
    printf("########## C\n");
    c();
    printf("########## D\n");
    d();
    printf("########## E\n");
    e();
    printf("########## F\n");
    f();
    printf("########## end\n");
    return 0;
}
