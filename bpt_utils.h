/* 
 * Copyright(c) Brayden Zhang
 * Mail: pczhang2010@gmail.com
 */

#ifndef _BPT_UTILS_H
#define _BPT_UTILS_H

static inline void swap_pointer (void** a, void** b);
static inline void* my_calloc (int size);
static inline int get_1st_ge (long a[], int len, long k);

void
swap_pointer(void** a, void** b)
{
	void* c = *a;
	*a = *b;
	*b = c;
}

void*
my_calloc(int size)
{
	void* p = calloc(1, size);
	if (p == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(-1);
	}
	return p;
}

/* Give an integer k, return the least-bigger index i in integer array, which 
 * makes a[i-1] < k <= a[i].
 * Note: if i == len, then k is bigger than all array elements.
 */
int
get_1st_ge(long a[], int len, long k)
{
	/* TODO: replaced with binary search */
	int i;
	for (i = 0; i < len; i++)
		if (a[i] >= k)
			return i;
	return len;
}

#endif /* End of _BPT_UTILS_H */
