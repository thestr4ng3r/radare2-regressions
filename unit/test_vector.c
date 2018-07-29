#include <r_util.h>
#include <r_vector.h>
#include "minunit.h"

// allocates a vector of len uint32_t values from 0 to len
// with capacity len + padding
static void init_test_vector(RVector *v, size_t len, size_t padding) {
	v->a = malloc ((len + padding) * 4);
	v->len = len;
	v->capacity = len + padding;
	v->elem_size = 4;

	uint32_t i;
	for (i = 0; i < len; i++) {
		((uint32_t *)v->a)[i] = i;
	}
}

// allocates a pvector of len pointers to uint32_t values from 0 to len
// with capacity len + padding
static void init_test_pvector(RVector *v, size_t len, size_t padding) {
	v->a = malloc ((len + padding) * sizeof (void *));
	v->len = len;
	v->capacity = len + padding;
	v->elem_size = sizeof (void *);

	uint32_t i;
	for (i = 0; i < len; i++) {
		uint32_t *e = malloc (sizeof (uint32_t));
		*e = i;
		((void **)v->a)[i] = e;
	}
}

static bool test_r_vector_old() {
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

	mu_end;
}


static bool test_vector_init() {
	RVector v;
	r_vector_init (&v, 42);
	mu_assert_eq_fmt (v.elem_size, 42UL, "init elem_size", "%lu");
	mu_assert_eq_fmt (v.len, 0UL, "init len", "%lu");
	mu_assert_eq_fmt (v.a, NULL, "init a", "%p");
	mu_assert_eq_fmt (v.capacity, 0UL, "init capacity", "%lu");
	mu_end;
}

static bool test_vector_new() {
	RVector *v = r_vector_new (42);
	mu_assert ("new", v);
	mu_assert_eq_fmt (v->elem_size, 42UL, "new elem_size", "%lu");
	mu_assert_eq_fmt (v->len, 0UL, "new len", "%lu");
	mu_assert_eq_fmt (v->a, NULL, "new a", "%p");
	mu_assert_eq_fmt (v->capacity, 0UL, "new capacity", "%lu");
	free (v);
	mu_end;
}

#define FREE_TEST_COUNT 10

static void elem_free_test(void *e, void *user) {
	uint32_t e_val = *((uint32_t *)e);
	int *acc = (int *)user;
	if (e_val < 0 || e_val > FREE_TEST_COUNT) {
		e_val = FREE_TEST_COUNT;
	}
	acc[e_val]++;
}

static bool test_vector_free() {
	RVector *v = r_vector_new (4);
	init_test_vector (v, FREE_TEST_COUNT, 0);

	int acc[FREE_TEST_COUNT+1] = {0};
	r_vector_free (v, elem_free_test, acc);

	// elem_free_test does acc[i]++ for element value i
	// => acc[0] through acc[FREE_TEST_COUNT-1] == 1
	// acc[FREE_TEST_COUNT] is for potentially invalid calls of elem_free_test

	uint32_t i;
	for (i=0; i<FREE_TEST_COUNT; i++) {
		mu_assert_eq (acc[i], 1, "free individual elements");
	}

	mu_assert_eq (acc[FREE_TEST_COUNT], 0, "invalid free calls");
	mu_end;
}

static bool test_vector_clear() {
	RVector v;
	init_test_vector (&v, FREE_TEST_COUNT, 0);

	int acc[FREE_TEST_COUNT+1] = {0};
	r_vector_clear (&v, elem_free_test, acc);

	// see test_vector_free

	uint32_t i;
	for (i = 0; i < FREE_TEST_COUNT; i++) {
		mu_assert_eq (acc[i], 1, "free individual elements");
	}

	mu_assert_eq (acc[FREE_TEST_COUNT], 0, "invalid free calls");
	mu_end;
}

static bool test_vector_clone() {
	RVector v;
	init_test_vector (&v, 5, 0);
	RVector *v1 = r_vector_clone (&v);
	r_vector_clear (&v, NULL, NULL);
	mu_assert ("r_vector_clone", v1);
	mu_assert_eq_fmt (v1->len, 5UL, "r_vector_clone => len", "%lu");
	mu_assert_eq_fmt (v1->capacity, 5UL, "r_vector_clone => capacity", "%lu");
	uint32_t i;
	for (i = 0; i < 5; i++) {
		mu_assert_eq (*((uint32_t *)r_vector_index_ptr (v1, i)), i, "r_vector_clone => content");
	}
	r_vector_free (v1, NULL, NULL);


	init_test_vector (&v, 5, 5);
	v1 = r_vector_clone (&v);
	r_vector_clear (&v, NULL, NULL);
	mu_assert ("r_vector_clone (+capacity)", v1);
	mu_assert_eq_fmt (v1->len, 5UL, "r_vector_clone (+capacity) => len", "%lu");
	mu_assert_eq_fmt (v1->capacity, 10UL, "r_vector_clone (+capacity) => capacity", "%lu");
	for (i = 0; i < 5; i++) {
		mu_assert_eq (*((uint32_t *)r_vector_index_ptr (v1, i)), i, "r_vector_clone => content");
	}
	// write over whole capacity to trigger potential errors with valgrind or asan
	for (i = 0; i < 10; i++) {
		*((uint32_t *)r_vector_index_ptr (v1, i)) = 1337;
	}
	r_vector_free (v1, NULL, NULL);

	mu_end;
}

static bool test_vector_empty() {
	RVector v;
	r_vector_init (&v, 1);
	bool empty = r_vector_empty (&v);
	mu_assert_eq (empty, true, "r_vector_init => r_vector_empty");
	uint8_t e = 0;
	r_vector_push (&v, &e);
	empty = r_vector_empty (&v);
	mu_assert_eq (empty, false, "r_vector_push => !r_vector_empty");
	r_vector_pop (&v, &e);
	empty = r_vector_empty (&v);
	mu_assert_eq (empty, true, "r_vector_pop => r_vector_empty");
	r_vector_clear (&v, NULL, NULL);

	RVector *vp = r_vector_new (42);
	empty = r_vector_empty (&v);
	mu_assert_eq (empty, true, "r_vector_new => r_vector_empty");
	r_vector_free (vp, NULL, NULL);

	mu_end;
}

static bool test_vector_delete_at() {
	RVector v;
	init_test_vector (&v, 5, 0);

	uint32_t e;
	r_vector_delete_at (&v, 2, &e);
	mu_assert_eq (e, 2, "r_vector_delete_at => into");
	mu_assert_eq_fmt (v.len, 4UL, "r_vector_delete_at => len", "%lu");

	mu_assert_eq (((uint32_t *)v.a)[0], 0, "r_vector_delete_at => remaining elements");
	mu_assert_eq (((uint32_t *)v.a)[1], 1, "r_vector_delete_at => remaining elements");
	mu_assert_eq (((uint32_t *)v.a)[2], 3, "r_vector_delete_at => remaining elements");
	mu_assert_eq (((uint32_t *)v.a)[3], 4, "r_vector_delete_at => remaining elements");

	r_vector_delete_at (&v, 3, &e);
	mu_assert_eq (e, 4, "r_vector_delete_at (end) => into");
	mu_assert_eq_fmt (v.len, 3UL, "r_vector_delete_at (end) => len", "%lu");

	mu_assert_eq (((uint32_t *)v.a)[0], 0, "r_vector_delete_at (end) => remaining elements");
	mu_assert_eq (((uint32_t *)v.a)[1], 1, "r_vector_delete_at (end) => remaining elements");
	mu_assert_eq (((uint32_t *)v.a)[2], 3, "r_vector_delete_at (end) => remaining elements");

	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_vector_insert() {
	RVector v;

	init_test_vector (&v, 4, 2);
	uint32_t e = 1337;
	e = *((uint32_t *)r_vector_insert (&v, 1, &e));
	mu_assert_eq_fmt (v.len, 5UL, "r_vector_insert => len", "%lu");
	mu_assert_eq (e, 1337, "r_vector_insert => content at returned ptr");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_insert => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 1337, "r_vector_insert => content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 2)), 1, "r_vector_insert => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 3)), 2, "r_vector_insert => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 4)), 3, "r_vector_insert => old content");
	r_vector_clear (&v, NULL, NULL);

	init_test_vector (&v, 4, 0);
	e = 1337;
	e = *((uint32_t *)r_vector_insert (&v, 1, &e));
	mu_assert ("r_vector_insert (resize) => capacity", v.capacity >= 5);
	mu_assert_eq_fmt (v.len, 5UL, "r_vector_insert => len", "%lu");
	mu_assert_eq (e, 1337, "r_vector_insert => content at returned ptr");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_insert (resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 1337, "r_vector_insert (resize) => content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 2)), 1, "r_vector_insert (resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 3)), 2, "r_vector_insert (resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 4)), 3, "r_vector_insert (resize) => old content");
	r_vector_clear (&v, NULL, NULL);

	init_test_vector (&v, 4, 2);
	e = 1337;
	e = *((uint32_t *)r_vector_insert (&v, 4, &e));
	mu_assert_eq_fmt (v.len, 5UL, "r_vector_insert (end) => len", "%lu");
	mu_assert_eq (e, 1337, "r_vector_insert (end) => content at returned ptr");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_insert (end) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 1, "r_vector_insert (end) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 2)), 2, "r_vector_insert (end) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 3)), 3, "r_vector_insert (end) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 4)), 1337, "r_vector_insert (end) => content");
	r_vector_clear (&v, NULL, NULL);

	init_test_vector (&v, 4, 0);
	e = 1337;
	e = *((uint32_t *)r_vector_insert (&v, 4, &e));
	mu_assert ("r_vector_insert (resize) => capacity", v.capacity >= 5);
	mu_assert_eq_fmt (v.len, 5UL, "r_vector_insert (end) => len", "%lu");
	mu_assert_eq (e, 1337, "r_vector_insert (end) => content at returned ptr");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_insert (end, resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 1, "r_vector_insert (end, resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 2)), 2, "r_vector_insert (end, resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 3)), 3, "r_vector_insert (end, resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 4)), 1337, "r_vector_insert (end, resize) => content");
	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_vector_insert_range() {
	RVector v;
	uint32_t range[] = { 0xC0, 0xFF, 0xEE };

	r_vector_init (&v, 4);
	uint32_t *p = (uint32_t *)r_vector_insert_range (&v, 0, range, 3);
	mu_assert_eq_fmt (p, r_vector_index_ptr (&v, 0), "r_vector_insert_range (empty) returned ptr", "%p");
	mu_assert_eq_fmt (v.len, 3UL, "r_vector_insert_range (empty) => len", "%lu");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0xC0, "r_vector_insert_range (empty) => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 0xFF, "r_vector_insert_range (empty) => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 2)), 0xEE, "r_vector_insert_range (empty) => new content");
	r_vector_clear (&v, NULL, NULL);

	init_test_vector (&v, 3, 3);
	p = (uint32_t *)r_vector_insert_range (&v, 2, range, 3);
	mu_assert_eq_fmt (p, r_vector_index_ptr (&v, 2), "r_vector_insert_range returned ptr", "%p");
	mu_assert_eq_fmt (v.len, 6UL, "r_vector_insert_range => len", "%lu");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_insert_range => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 1, "r_vector_insert_range => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 2)), 0xC0, "r_vector_insert_range => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 3)), 0xFF, "r_vector_insert_range => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 4)), 0xEE, "r_vector_insert_range => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 5)), 2, "r_vector_insert_range => old content");
	r_vector_clear (&v, NULL, NULL);

	init_test_vector (&v, 3, 3);
	p = (uint32_t *)r_vector_insert_range (&v, 3, range, 3);
	mu_assert_eq_fmt (p, r_vector_index_ptr (&v, 3), "r_vector_insert_range (end) returned ptr", "%p");
	mu_assert_eq_fmt (v.len, 6UL, "r_vector_insert_range (end) => len", "%lu");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_insert_range (end) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 1, "r_vector_insert_range (end) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 2)), 2, "r_vector_insert_range (end) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 3)), 0xC0, "r_vector_insert_range (end) => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 4)), 0xFF, "r_vector_insert_range (end) => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 5)), 0xEE, "r_vector_insert_range (end) => new content");
	r_vector_clear (&v, NULL, NULL);

	init_test_vector (&v, 3, 0);
	p = (uint32_t *)r_vector_insert_range (&v, 2, range, 3);
	mu_assert_eq_fmt (p, r_vector_index_ptr (&v, 2), "r_vector_insert_range (resize) returned ptr", "%p");
	mu_assert_eq_fmt (v.len, 6UL, "r_vector_insert_range (resize) => len", "%lu");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_insert_range (resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 1, "r_vector_insert_range (resize) => old content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 2)), 0xC0, "r_vector_insert_range (resize) => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 3)), 0xFF, "r_vector_insert_range (resize) => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 4)), 0xEE, "r_vector_insert_range (resize) => new content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 5)), 2, "r_vector_insert_range (resize) => old content");
	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_vector_pop() {
	RVector v;
	init_test_vector (&v, 3, 0);

	uint32_t e;
	r_vector_pop (&v, &e);
	mu_assert_eq (e, 2, "r_vector_pop into");
	mu_assert_eq_fmt (v.len, 2UL, "r_vector_pop => len", "%lu");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_pop => remaining content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 1, "r_vector_pop => remaining content");

	r_vector_pop (&v, &e);
	mu_assert_eq (e, 1, "r_vector_pop into");
	mu_assert_eq_fmt (v.len, 1UL, "r_vector_pop => len", "%lu");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 0, "r_vector_pop => remaining content");

	r_vector_pop (&v, &e);
	mu_assert_eq (e, 0, "r_vector_pop (last) into");
	mu_assert_eq_fmt (v.len, 0UL, "r_vector_pop (last) => len", "%lu");

	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_vector_pop_front() {
	RVector v;
	init_test_vector (&v, 3, 0);

	uint32_t e;
	r_vector_pop_front (&v, &e);
	mu_assert_eq (e, 0, "r_vector_pop_front into");
	mu_assert_eq_fmt (v.len, 2UL, "r_vector_pop_front => len", "%lu");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 1, "r_vector_pop_front => remaining content");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 1)), 2, "r_vector_pop_front => remaining content");

	r_vector_pop_front (&v, &e);
	mu_assert_eq (e, 1, "r_vector_pop_front into");
	mu_assert_eq_fmt (v.len, 1UL, "r_vector_pop_front => len", "%lu");
	mu_assert_eq (*((uint32_t *)r_vector_index_ptr (&v, 0)), 2, "r_vector_pop_front => remaining content");

	r_vector_pop_front (&v, &e);
	mu_assert_eq (e, 2, "r_vector_pop_front (last) into");
	mu_assert_eq_fmt (v.len, 0UL, "r_vector_pop_front (last) => len", "%lu");

	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_vector_push() {
	RVector v;
	r_vector_init (&v, 4);

	uint32_t e = 1337;
	e = *((uint32_t *)r_vector_push (&v, &e));
	mu_assert_eq_fmt (v.len, 1UL, "r_vector_push (empty) => len == 1", "%lu");
	mu_assert_eq (e, 1337, "r_vector_push (empty) => content at returned ptr");
	e = *((uint32_t *)r_vector_index_ptr (&v, 0));
	mu_assert_eq (e, 1337, "r_vector_push (empty) => content");

	e = 0xDEAD;
	e = *((uint32_t *)r_vector_push (&v, &e));
	mu_assert_eq_fmt (v.len, 2UL, "r_vector_push => len == 2", "%lu");
	mu_assert_eq (e, 0xDEAD, "r_vector_push (empty) => content at returned ptr");
	e = *((uint32_t *)r_vector_index_ptr (&v, 0));
	mu_assert_eq (e, 1337, "r_vector_push => old content");
	e = *((uint32_t *)r_vector_index_ptr (&v, 1));
	mu_assert_eq (e, 0xDEAD, "r_vector_push => content");

	e = 0xBEEF;
	e = *((uint32_t *)r_vector_push (&v, &e));
	mu_assert_eq_fmt (v.len, 3UL, "r_vector_push => len == 3", "%lu");
	mu_assert_eq (e, 0xBEEF, "r_vector_push (empty) => content at returned ptr");
	e = *((uint32_t *)r_vector_index_ptr (&v, 0));
	mu_assert_eq (e, 1337, "r_vector_push => old content");
	e = *((uint32_t *)r_vector_index_ptr (&v, 1));
	mu_assert_eq (e, 0xDEAD, "r_vector_push => old content");
	e = *((uint32_t *)r_vector_index_ptr (&v, 2));
	mu_assert_eq (e, 0xBEEF, "r_vector_push => content");

	r_vector_clear (&v, NULL, NULL);


	init_test_vector (&v, 5, 0);
	e = 1337;
	e = *((uint32_t *)r_vector_push (&v, &e));
	mu_assert ("r_vector_push (resize) => capacity", v.capacity >= 6);
	mu_assert_eq_fmt (v.len, 6UL, "r_vector_push (resize) => len", "%lu");
	mu_assert_eq (e, 1337, "r_vector_push (empty) => content at returned ptr");

	size_t i;
	for (i = 0; i < v.len - 1; i++) {
		e = *((uint32_t *)r_vector_index_ptr (&v, i));
		mu_assert_eq (e, (uint32_t)i, "r_vector_push (resize) => old content");
	}
	e = *((uint32_t *)r_vector_index_ptr (&v, 5));
	mu_assert_eq (e, 1337, "r_vector_push (resize) => content");

	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_vector_push_front() {
	RVector v;
	r_vector_init (&v, 4);

	uint32_t e = 1337;
	e = *((uint32_t *)r_vector_push_front (&v, &e));
	mu_assert_eq_fmt (v.len, 1UL, "r_vector_push_front (empty) => len == 1", "%lu");
	mu_assert_eq (e, 1337, "r_vector_push_front (empty) => content at returned ptr");
	e = *((uint32_t *)r_vector_index_ptr (&v, 0));
	mu_assert_eq (e, 1337, "r_vector_push (empty) => content");

	e = 0xDEAD;
	e = *((uint32_t *)r_vector_push_front (&v, &e));
	mu_assert_eq_fmt (v.len, 2UL, "r_vector_push_front => len == 2", "%lu");
	mu_assert_eq (e, 0xDEAD, "r_vector_push_front (empty) => content at returned ptr");
	e = *((uint32_t *)r_vector_index_ptr (&v, 0));
	mu_assert_eq (e, 0xDEAD, "r_vector_push_front => content");
	e = *((uint32_t *)r_vector_index_ptr (&v, 1));
	mu_assert_eq (e, 1337, "r_vector_push_front => old content");

	e = 0xBEEF;
	e = *((uint32_t *)r_vector_push_front (&v, &e));
	mu_assert_eq_fmt (v.len, 3UL, "r_vector_push_front => len == 3", "%lu");
	mu_assert_eq (e, 0xBEEF, "r_vector_push_front (empty) => content at returned ptr");
	e = *((uint32_t *)r_vector_index_ptr (&v, 0));
	mu_assert_eq (e, 0xBEEF, "r_vector_push_front => content");
	e = *((uint32_t *)r_vector_index_ptr (&v, 1));
	mu_assert_eq (e, 0xDEAD, "r_vector_push_front => old content");
	e = *((uint32_t *)r_vector_index_ptr (&v, 2));
	mu_assert_eq (e, 1337, "r_vector_push_front => old content");

	r_vector_clear (&v, NULL, NULL);


	init_test_vector (&v, 5, 0);
	e = 1337;
	e = *((uint32_t *)r_vector_push_front (&v, &e));
	mu_assert ("r_vector_push_front (resize) => capacity", v.capacity >= 6);
	mu_assert_eq_fmt (v.len, 6UL, "r_vector_push_front (resize) => len", "%lu");
	mu_assert_eq (e, 1337, "r_vector_push_front (empty) => content at returned ptr");

	size_t i;
	for (i = 1; i < v.len; i++) {
		e = *((uint32_t *)r_vector_index_ptr (&v, i));
		mu_assert_eq (e, (uint32_t)i - 1, "r_vector_push (resize) => old content");
	}
	e = *((uint32_t *)r_vector_index_ptr (&v, 0));
	mu_assert_eq (e, 1337, "r_vector_push (resize) => content");

	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_vector_reserve() {
	RVector v;
	r_vector_init (&v, 4);

	r_vector_reserve (&v, 42);
	mu_assert_eq_fmt (v.capacity, 42UL, "r_vector_reserve (empty) => capacity", "%lu");
	mu_assert ("r_vector_reserve (empty) => a", v.a);
	size_t i;
	for (i = 0; i < v.capacity; i++) {
		*((uint32_t *)r_vector_index_ptr (&v, i)) = 1337;
	}
	v.len = 20;

	r_vector_reserve (&v, 100);
	mu_assert_eq_fmt (v.capacity, 100UL, "r_vector_reserve => capacity", "%lu");
	mu_assert ("r_vector_reserve => a", v.a);
	for (i = 0; i < v.capacity; i++) {
		*((uint32_t *)r_vector_index_ptr (&v, i)) = 1337;
	}

	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_vector_shrink() {
	RVector v;
	init_test_vector (&v, 5, 5);
	void *a = r_vector_shrink (&v);
	mu_assert_eq_fmt (a, v.a, "r_vector_shrink ret", "%p");
	mu_assert_eq_fmt (v.len, 5UL, "r_vector_shrink => len", "%lu");
	mu_assert_eq_fmt (v.capacity, 5UL, "r_vector_shrink => capacity", "%lu");
	r_vector_clear (&v, NULL, NULL);

	init_test_vector (&v, 5, 0);
	a = r_vector_shrink (&v);
	mu_assert_eq_fmt (a, v.a, "r_vector_shrink (already minimal) ret", "%p");
	mu_assert_eq_fmt (v.len, 5UL, "r_vector_shrink (already minimal) => len", "%lu");
	mu_assert_eq_fmt (v.capacity, 5UL, "r_vector_shrink (already minimal) => capacity", "%lu");
	r_vector_clear (&v, NULL, NULL);

	mu_end;
}

static bool test_pvector_init() {
	RVector v;
	r_pvector_init (&v);
	mu_assert_eq_fmt (v.elem_size, sizeof (void *), "elem_size", "%lu");
	mu_assert_eq_fmt (v.len, 0UL, "len", "%lu");
	mu_assert_eq_fmt (v.a, NULL, "a", "%p");
	mu_assert_eq_fmt (v.capacity, 0UL, "capacity", "%lu");
	mu_end;
}

static bool test_pvector_new() {
	RVector *v = r_pvector_new ();
	mu_assert_eq_fmt (v->elem_size, sizeof (void *), "elem_size", "%lu");
	mu_assert_eq_fmt (v->len, 0UL, "len", "%lu");
	mu_assert_eq_fmt (v->a, NULL, "a", "%p");
	mu_assert_eq_fmt (v->capacity, 0UL, "capacity", "%lu");
	free (v);
	mu_end;
}

static bool test_pvector_free() {
	// run with asan or valgrind
	RVector *v = R_NEW (RVector);
	init_test_pvector (v, 5, 5);
	mu_assert_eq_fmt (v->len, 5UL, "initial len", "%lu");
	mu_assert ("initial a", v->a);
	mu_assert_eq_fmt (v->capacity, 10UL, "initial capacity", "%lu");
	r_pvector_free (v, free);
	mu_end;
}

static bool test_pvector_clear() {
	// run with asan or valgrind
	RVector v;
	init_test_pvector (&v, 5, 5);
	mu_assert_eq_fmt (v.len, 5UL, "initial len", "%lu");
	mu_assert ("initial a", v.a);
	mu_assert_eq_fmt (v.capacity, 10UL, "initial capacity", "%lu");
	r_pvector_clear (&v, free);
	mu_assert_eq_fmt (v.len, 0UL, "len", "%lu");
	mu_assert_eq_fmt (v.a, NULL, "a", "%p");
	mu_assert_eq_fmt (v.capacity, 0UL, "capacity", "%lu");
	mu_end;
}

static int all_tests() {
	mu_run_test (test_r_vector_old);
	mu_run_test (test_vector_init);
	mu_run_test (test_vector_new);
	mu_run_test (test_vector_empty);
	mu_run_test (test_vector_free);
	mu_run_test (test_vector_clone);
	mu_run_test (test_vector_clear);
	mu_run_test (test_vector_delete_at);
	mu_run_test (test_vector_insert);
	mu_run_test (test_vector_insert_range);
	mu_run_test (test_vector_pop);
	mu_run_test (test_vector_pop_front);
	mu_run_test (test_vector_push);
	mu_run_test (test_vector_push_front);
	mu_run_test (test_vector_reserve);
	mu_run_test (test_vector_shrink);

	mu_run_test (test_pvector_init);
	mu_run_test (test_pvector_new);
	mu_run_test (test_pvector_free);
	mu_run_test (test_pvector_clear);

	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests();
}
