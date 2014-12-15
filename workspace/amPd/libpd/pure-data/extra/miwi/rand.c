#include "m_pd.h"
#include <time.h>

static t_class *rand_class;

typedef struct _rand {
	t_object x_obj;
	t_float x_f;
	int x_thym;
	unsigned int x_state; // roll-over odometer
} t_rand;

static int rand_addthym(void) {
	int thym = time(0) % 31536000; // seconds in a year
	return thym + !(thym%2); // odd numbers only
}

static int rand_timeseed(int thym) {
	static unsigned int rand_nextseed = 1489853723;
	rand_nextseed = rand_nextseed * thym + 938284287;
	return (rand_nextseed & 0x7fffffff);
}

static void rand_seed(t_rand *x, t_symbol *s, int argc, t_atom *argv) {
	x->x_state = x->x_thym =
		(!argc ? rand_addthym() : atom_getfloat(argv));
}

static void rand_peek(t_rand *x, t_symbol *s) {
	post("%s%s%u (%d)", s->s_name, (*s->s_name ? ": " : ""),
		x->x_state, x->x_thym);
}

static void rand_bang(t_rand *x) {
	int n=x->x_f, nval;
	int range = (n < 1 ? 1 : n);
	x->x_state = x->x_state * 472940017 + 832416023;
	nval = ((double)range) * ((double)x->x_state) / 4294967296;
	outlet_float(x->x_obj.ob_outlet, nval);
}

static void *rand_new(t_floatarg f) {
	t_rand *x = (t_rand *)pd_new(rand_class);
	x->x_f = f;
	x->x_thym = rand_addthym();
	x->x_state = rand_timeseed(x->x_thym);
	floatinlet_new(&x->x_obj, &x->x_f);
	outlet_new(&x->x_obj, &s_float);
	return (x);
}

void rand_setup(void) {
	rand_class = class_new(gensym("rand"),
		(t_newmethod)rand_new, 0,
		sizeof(t_rand), 0,
		A_DEFFLOAT, 0);

	class_addbang(rand_class, rand_bang);
	class_addmethod(rand_class, (t_method)rand_seed,
		gensym("seed"), A_GIMME, 0);
	class_addmethod(rand_class, (t_method)rand_peek,
		gensym("peek"), A_DEFSYM, 0);
}
