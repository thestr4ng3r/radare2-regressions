#include <r_util.h>
#include <r_vector.h>
#include "minunit.h"

bool test_r_vector() {
	ptrdiff_t i;
	void **it;
	void *val;
	RVector *v = r_pvector_new ();
	it = r_pvector_push (v, (void *)1337);
	mu_assert_eq_fmt (*it, (void *)1337, "first push", "%p");
	for (i = 0; i < 10; i++) {
		r_pvector_push (v, (void *)i);
		mu_assert_eq_fmt (v->len, i + 2, "push more", "%lu");
	}
	val = r_pvector_pop_front (v);
	mu_assert_eq_fmt (val, (void *)1337, "pop_front", "%p");
	mu_assert ("contains", r_pvector_contains (v, (void *)9));

	val = r_pvector_delete_at (v, 9);
	mu_assert_eq_fmt (val, (void *)9, "delete_at", "%p");
	mu_assert ("!contains after delete_at", !r_pvector_contains (v, (void *)9));

	i = 0;
	r_pvector_foreach (v, it) {
		void *e = (void *)i++;
		mu_assert_eq_fmt (*it, e, "remaining elements", "%p");
	}

	r_vector_shrink (v);
	mu_assert_eq_fmt (v->len, 9UL, "len after shrink", "%lu");
	RVector *v1 = r_vector_clone (v);
	r_pvector_clear (v, NULL);
	mu_assert ("empty after clear", v->capacity == 0 && v->len == 0);
	mu_assert_eq_fmt (v1->len, 9UL, "unaffected source after clone", "%lu");

	r_pvector_free (v, NULL);
	r_pvector_free (v1, NULL);

	RVector s;
	r_pvector_init (&s);
	r_vector_reserve (&s, 10);
	r_pvector_clear (&s, NULL);

	r_vector_reserve (&s, 10);
	r_pvector_push (&s, (void *)-1);
	mu_assert ("stack allocated vector init", s.len == 1 && s.capacity == 10);
	for (i = 0; i < 20; i++) {
		r_pvector_push (&s, (void *)i);
	}
	r_vector_reserve (&s, 10);
	r_pvector_clear (&s, NULL);

	{
		void *a[] = {(void*)0, (void*)2, (void*)4, (void*)6, (void*)8};
		RVector s;
		r_pvector_init (&s);
		size_t l;
		r_vector_insert_range (&s, 0, a + 2, 3);
		r_vector_insert_range (&s, 0, a, 2);

#define CMP(x, y) x - y
		r_pvector_lower_bound (&s, (void *)4, l, CMP);
		mu_assert_eq_fmt (r_pvector_at (&s, l), (void *)4, "lower_bound", "%p");
		r_pvector_lower_bound (&s, (void *)5, l, CMP);
		mu_assert_eq_fmt (r_pvector_at (&s, l), (void *)6, "lower_bound 2", "%p");
		r_pvector_lower_bound (&s, (void *)6, l, CMP);
		mu_assert_eq_fmt (r_pvector_at (&s, l), (void *)6, "lower_bound 3", "%p");
		r_pvector_lower_bound (&s, (void *)9, l, CMP);
		mu_assert_eq_fmt (l, s.len, "lower_bound 3", "%lu");

		r_pvector_upper_bound (&s, (void *)4, l, CMP);
		mu_assert_eq_fmt (r_pvector_at (&s, l), (void *)6, "upper_bound", "%p");
		r_pvector_upper_bound (&s, (void *)5, l, CMP);

		mu_assert_eq_fmt (r_pvector_at (&s, l), (void *)6, "upper_bound 2", "%p");
		r_pvector_upper_bound (&s, (void *)6, l, CMP);
		mu_assert_eq_fmt (r_pvector_at (&s, l), (void *)8, "upper_bound 3", "%p");
#undef CMP

		r_pvector_clear (&s, NULL);

		r_pvector_push (&s, strdup ("Charmander"));
		r_pvector_push (&s, strdup ("Squirtle"));
		r_pvector_push (&s, strdup ("Bulbasaur"));
		r_pvector_push (&s, strdup ("Meowth"));
		r_pvector_push (&s, strdup ("Caterpie"));
		r_pvector_sort (&s, (RPVectorComparator) strcmp);

		r_pvector_lower_bound (&s, "Meow", l, strcmp);
		mu_assert_streq ((char *)r_pvector_at (&s, l), "Meowth", "sort, lower_bound");

		r_pvector_clear (&s, free);
	}

	//return 0;
	mu_end;
}

int all_tests() {
	mu_run_test(test_r_vector);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests();
}
