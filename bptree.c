/* Copyright(c) Brayden Zhang
 * Mail: pczhang2010@gmail.com
 */
#include <assert.h>

#include "bptree.h"

int
bpt_empty(bpt_node* root)
{
	return root == NULL;
}

bpt_num_of_key(bpt_node* n)
{
	if(bpt_is_leaf(n))
		return n->num_of_rec;
	return n->num_of_rec - 1;
}

bpt_node* 
bpt_create_leaf_node()
{
	bpt_node* n = (bpt_node*) my_calloc(sizeof(bpt_node));
	n->t = LEAF;
	n->num_of_rec = 0;
	n->p = NULL;
	return n;
}

bpt_node*
bpt_create_index_node()
{
	bpt_node* n = bpt_create_leaf_node();
	n->t = INDEX;
	return n;
}

void 
bpt_init(bpt_node** root)
{
	*root = bpt_create_leaf_node();
}

int
bpt_is_leaf(bpt_node* p)
{
	assert(p->t == LEAF || p->t == INDEX);
	return p->t == LEAF;
}

int 
bpt_is_root(bpt_node* l)
{
	return l->p == NULL;
}

int
bpt_is_full(bpt_node* l)
{
	assert(l->num_of_rec <= BPT_MAX_REC_NO);
	return l->num_of_rec == BPT_MAX_REC_NO;
}

void
bpt_delete_node(bpt_node** n)
{
	free(*n);
	*n = NULL;
}

/* Get the record index from node's parent, eg, if record is the first record of
 * its parent, then return 0.
 */
int 
bpt_locate_in_parent(bpt_node* l)
{
	int ind;
	bpt_node* p = l->p;
	for(ind = 0; ind < p->num_of_rec; ind++)
		if(p->recs.c_arr[ind] == l)
			return ind;
	assert(0); // Should never reach here
}

/* 1. If node is root and is not leaf, it must have num_of_rec >=2;
 * 2. If node is root and is leaf, it can have num_of_rec == 0 or == 1 ;
 * 3. If node is not root, it must have num_of_rec >= ceil(BPT_MAX_REC_NO / 2).
 */
int
bpt_is_enough(bpt_node* n)
{
	return bpt_is_root(n) 
			? bpt_is_leaf(n)
				? 1 
				: n->num_of_rec >= 2 
			: n->num_of_rec >= ceil(BPT_MAX_REC_NO / 2);
}

/** For the searching key K, return the leaf node N 
 * TODO: verify if duplicated keys exists, is current behavior correct?
 */ 
bpt_node*
bpt_query(bpt_node* root, long k)
{
	bpt_node* n = root;
	while(! bpt_is_leaf(n)){
		if(bpt_is_root(n))
			assert(n->num_of_rec >=2);
		else assert(n->num_of_rec >= (BPT_MAX_REC_NO + 1) / 2);

		int ind = get_1st_ge(n->key, bpt_num_of_key(n), k);

		if(ind == bpt_num_of_key(n)) 
			/* k is the biggest, search the last child */
			n = n->recs.c_arr[bpt_num_of_key(n)];

		/* Now k <= n->key[ind] */
		else if(k == n->key[ind])
			n = n->recs.c_arr[ind + 1];
		else n = n->recs.c_arr[ind];
	}

	/* We are in the leaf node now */
	return n;
}

/* In leaf node, insert key in l[key_ind]; 
 * insert record in l[rec_ind] 
 */
void
bpt_insert_in_leaf_at(bpt_node* l, int key_ind, int rec_ind, 
		long k, bpt_record_t *v)
{
	assert(key_ind >= 0 && rec_ind >= 0 && ! bpt_is_full(l));

	int i;
	for(i = l->num_of_rec; i > key_ind; i--)//make room for the new key
		l->key[i] = l->key[i - 1];
	l->key[key_ind] = k;

	for(i = l->num_of_rec; i > rec_ind; i--)//make room for the new record
		l->recs.l_rec.r_arr[i] = l->recs.l_rec.r_arr[i - 1];
	l->recs.l_rec.r_arr[rec_ind] = v;

	l->num_of_rec++;
}

/* Insert (k, v) into leaf node */ 
void
bpt_insert_in_leaf(bpt_node* l, long k, bpt_record_t* v)
{
	assert(!bpt_is_full(l));
	
	/* (k, v) should be added in the location of the first key which is 
	 * greater or equal to k; if k is greater than all the keys, (k, v) will
	 * be insert after the last key 
	 * */
	int ind = get_1st_ge(l->key, l->num_of_rec, k);
	bpt_insert_in_leaf_at(l, ind, ind, k, v);  
}

/* In index node, insert key in n[key_ind]; 
 * insert new child in n[rec_ind] 
 */
void
bpt_insert_in_index_at(bpt_node* n, int key_ind, int rec_ind, 
		long k, bpt_node* l)
{
	assert(key_ind >= 0 && rec_ind >= 0 && ! bpt_is_full(n));
	int i;

	for(i = bpt_num_of_key(n); i > key_ind; i--)//make room for the new key
		n->key[i] = n->key[i - 1];
	n->key[key_ind] = k;

	for(i = n->num_of_rec; i > rec_ind; i--) //make room for the new child
		n->recs.c_arr[i] = n->recs.c_arr[i - 1];
	n->recs.c_arr[rec_ind] = l;

	n->num_of_rec++; 
	l->p = n;
}

/* New pair (split_key, l1) need to be added into root, but root node is full, 
 * so need to split the root node. The new root will have two child, the old 
 * root node and the new node, separated by the split_key. 
 */
void
bpt_split_root(bpt_node** root, long split_key, bpt_node* l1)
{
	bpt_node* l = *root;
	assert(bpt_is_root(l));

	/* Split the old root node, r is the new root node */
	bpt_node* r = bpt_create_index_node();
	r->num_of_rec = 2;
	r->key[0] = split_key;
	r->recs.c_arr[0] = l;
	r->recs.c_arr[1] = l1;

	*root = r;

	l->p = r;
	l1->p = r;
}

void 
bpt_split_parent(bpt_node** root, bpt_node* p, int key_ind, int child_ind, 
		long k, bpt_node* l);

/* The new pair (split_key, l1) need to be insert into node l's parent, right 
 * after node l. Please note if parent node is full, we need to split the parent
 * node.
 */
void
bpt_insert_in_parent(bpt_node** root, bpt_node* l, 
		long split_key, bpt_node* l1) 
{
	if(bpt_is_root(l)){
		bpt_split_root(root, split_key, l1);
		return;
	} 

	bpt_node* p = l->p;
	int ind = bpt_locate_in_parent(l);
	if(! bpt_is_full(p))
		bpt_insert_in_index_at(p, ind, ind + 1, split_key, l1);
	else{
		bpt_split_parent(root, p, ind, ind + 1, split_key, l1);
	}
}

/* New pair (k, l) need to be added into parent node, key_ind and child_ind is 
 * the location to insert the new key and new child into the key array and child
 * array. but parent node is full,so need to split the parent node. 
 */
void
bpt_split_parent(bpt_node** root, bpt_node* p, int key_ind, int child_ind, 
		long k, bpt_node* l)
{
	assert(bpt_is_full(p));

	/* Temporary storage */
	long ind_arr[p->num_of_rec];
	bpt_node* rec_arr[p->num_of_rec + 1];

	/* Copy from the old node to temporary storage */
	memcpy(ind_arr, p->key, bpt_num_of_key(p) * sizeof(long));
	memcpy(rec_arr, p->recs.c_arr, p->num_of_rec * sizeof(bpt_node*));

	/* Add the new (k, l) to temporary storage */
	int i;
	for(i = bpt_num_of_key(p); i > key_ind; i--) //make room for new key
		ind_arr[i] = ind_arr[i - 1];
	for(i = p->num_of_rec; i > child_ind; i--) //make room for new child
		rec_arr[i] = rec_arr[i - 1];
	ind_arr[key_ind] = k;
	rec_arr[child_ind] = l;

	int num = (p->num_of_rec + 1) / 2; // Num to move to original node
	int num1 = p->num_of_rec + 1 - num; // Num to move to new node 

	/* Move from temporary to orginal node */
	memcpy(p->key, ind_arr, (num - 1) * sizeof(long));
	memcpy(p->recs.c_arr, rec_arr, num * sizeof(bpt_node*));
	p->num_of_rec = num;

	/* Create new index node */
	bpt_node* p1 = bpt_create_index_node();

	/* Move from temporary to new node */
	memcpy(p1->key, ind_arr + num,  num1 * sizeof(long));
	memcpy(p1->recs.c_arr,rec_arr + num, num1 * sizeof(bpt_node*));
       	p1->num_of_rec = num1;      

	/* Split key for original node(p) and new node(p1) in p->p */
	long split_key = ind_arr[num - 1]; 

	/* Insert the new splitted node into parent */
	if(k < split_key)
		l->p = p;

	/* The new node's children should change their parents now */
	for(i = 0; i < p1->num_of_rec; i++)
		p1->recs.c_arr[i]->p = p1;

	/* The new node should be adopted by its parent now */
	bpt_insert_in_parent(root, p, split_key, p1);
}

void bpt_split_leaf(bpt_node** root, bpt_node* l, long k, bpt_record_t* v);

/* Insert pair (k, v) into the B-Plus-Tree */
void 
bpt_insert(bpt_node** root, long k, bpt_record_t* v)
{
	bpt_node* l;
	if(bpt_empty(*root)){
		bpt_init(root);
		l = *root;
	}else l = bpt_query(*root, k);
	
	/* Now l is the leaf node */
	if(! bpt_is_full(l))
		bpt_insert_in_leaf(l, k, v);
	else bpt_split_leaf(root, l, k, v);
}

/* New pair (k, l) need to be added into leaf node, but this leaf node is full,
 * so need to split it. 
 */
void
bpt_split_leaf(bpt_node** root, bpt_node* l, long k, bpt_record_t* v)
{
	assert(bpt_is_full(l));

	/* Temporary storage */
	long ind_arr[l->num_of_rec + 1];
	bpt_record_t* rec_arr[l->num_of_rec + 1];

	/* Move all items of orginal node to temporary */
	memcpy(ind_arr, l->key, l->num_of_rec * sizeof(long));
	memcpy(rec_arr, l->recs.l_rec.r_arr, 
			l->num_of_rec * sizeof(bpt_record_t*));

	/* Insert the new (k, v) to temporary */
	int ind = get_1st_ge(l->key, l->num_of_rec, k);
	int i;
	for(i = l->num_of_rec; i > ind; i--){
		/* make room for the new key and record */
		ind_arr[i] = ind_arr[i - 1];
		rec_arr[i] = rec_arr[i - 1];
	}
	ind_arr[ind] = k;
	rec_arr[ind] = v;
	
	/* Num to move to original node */
	int num = (l->num_of_rec + 1) / 2;
	/* Num to move to new node */
	int num1 = l->num_of_rec + 1 - num; 

	/* Move from temporary to original node */
	memcpy(l->key, ind_arr, num * sizeof(long));
	memcpy(l->recs.l_rec.r_arr, rec_arr, num * sizeof(bpt_record_t*));
	l->num_of_rec = num;

	/* Create the new leaf node */
	bpt_node* l1 = bpt_create_leaf_node();
	
	/* TODO: maintain the link list of the leaf node */
	//TAILQ_INSERT_AFTER(&rec_list_head, l, l1, recs.l_rec.n);

	/* Move from temporary to new node */
	memcpy(l1->key, ind_arr + num,  num1 * sizeof(long));
	memcpy(l1->recs.l_rec.r_arr, rec_arr + num, 
			num1 * sizeof(bpt_record_t*));
       	l1->num_of_rec = num1;      

	/* Add the new splitted node into parent;
	 * use the first key of the new node as the split key
	 */
	bpt_insert_in_parent(root, l, l1->key[0], l1);
}

void
bpt_replace_root_with_child(bpt_node** root)
{
	bpt_node* r = (*root)->recs.c_arr[0];
	r->p = NULL;
	free(*root);
	*root = r;
}

/* Given a node n, return the close sibling of n. The returned sibling node is 
 * hold by parameter n1, the returned split key between the two node is hold by
 * parameter k. The order of n and it's sibling makes no differences here.
 */
void 
bpt_get_close_sibling(bpt_node* n, bpt_node** n1, long* k)
{
	bpt_node* p = n->p;
	assert(p && p->num_of_rec > 1);

	int ind = bpt_locate_in_parent(n);
	int direction = (ind == 0) ? 1 : -1; // indicats index +1 or -1
	if(ind == 0){
		*n1 = p->recs.c_arr[ind + 1];
		*k = p->key[ind];
	}else{
		*n1 = p->recs.c_arr[ind - 1];
		*k = p->key[ind - 1];
	}
}	


void bpt_delete_entry(bpt_node** root, bpt_node* n, void* v);

/* Merge the second index node into the first index node. split_key is the 
 * split key between the two node. The second index node will be freed.
 */
void
bpt_merge_index(bpt_node** root, bpt_node* n, long split_key, bpt_node** n1)
{
	bpt_node *n11 = *n1;

	/* add the split key into node */
	n->key[bpt_num_of_key(n)] = split_key;

	/* Copy keys of the second index node into the first index node */
	memcpy(n->key + n->num_of_rec, n11->key, 
			n11->num_of_rec * sizeof(long));

	/* Copy children of the second index node into the first index node */
	memcpy(n->recs.c_arr + n->num_of_rec, n11->recs.c_arr, 
			n11->num_of_rec * sizeof(bpt_node*));

	n->num_of_rec += n11->num_of_rec;

	/* NOTE!! For the children of to-be-removed index node, 
	 * they should change their parent. */
	int i;
	for(i = 0; i < n11->num_of_rec; i++)
		n11->recs.c_arr[i]->p = n;

	/* Need to remove it from its parent. */
	bpt_delete_entry(root, n->p, *n1);

	/* Free the node memory */
	bpt_delete_node(n1);
}

/* Merge the second leaf node into the first leaf node. The second leaf node 
 * will be freed. 
 */
void
bpt_merge_leaf(bpt_node** root, bpt_node* n, bpt_node** n1)
{
	bpt_node *n11 = *n1;
	
	/* Copy keys of the second leaf node into the first leaf node */
	memcpy(n->key + n->num_of_rec, n11->key, 
			n11->num_of_rec * sizeof(long));

	/* Copy records of the second leaf node into the first leaf node */
	memcpy(n->recs.l_rec.r_arr + n->num_of_rec, n11->recs.l_rec.r_arr, 
			n11->num_of_rec * sizeof(bpt_record_t*));

	n->num_of_rec += n11->num_of_rec;

	/* Need to remove the second node from its parent */
	bpt_delete_entry(root, n->p, *n1);

	/* Free the node memory */
	bpt_delete_node(n1);
}

/* In the index node, delete key from the key array at key_ind; delete child 
 * from child array at rec_ind. Note the key should be the split key of the i
 * child, which means, key_ind == rec_ind or key_ind == rec_ind - 1
 */
void
bpt_delete_in_index_at(bpt_node* n, int key_ind, int rec_ind)
{
	assert(key_ind >= 0 && key_ind < bpt_num_of_key(n) 
		&& rec_ind >= 0	&& rec_ind < n->num_of_rec);
	/* Key is the split key of the child */
	assert(key_ind == rec_ind || key_ind == rec_ind - 1);

	int i;
	for(i = key_ind; i < bpt_num_of_key(n) - 1; i++)
		n->key[i] = n->key[i + 1];
	for(i = rec_ind; i < n->num_of_rec - 1; i++)
		n->recs.c_arr[i] = n->recs.c_arr[i + 1];

	n->num_of_rec--;
}

/* In the leaf node, delete key from the key array at ind; delete child from 
 * child array at ind. 
 */
void
bpt_delete_in_leaf_at(bpt_node* n, int ind)
{
	assert(ind >= 0 && ind < n->num_of_rec);

	int i;
	for(i = ind; i < n->num_of_rec - 1; i++){
		n->key[i] = n->key[i + 1];
		n->recs.l_rec.r_arr[i] = n->recs.l_rec.r_arr[i + 1];
	}

	n->num_of_rec--;
}

/* Replace key of the node's key array at specific index */
void
bpt_replace_key_in_parent(bpt_node* p, int ind, long k)
{
	p->key[ind] = k;
}

/* Borrow on record from the second leaf node to the first leaf node. The second
 * node is the previous node of the first node.
 */
void
bpt_borrow_from_pre_leaf(bpt_node* n, bpt_node* n1)
{
	long k = n1->key[bpt_num_of_key(n1) - 1];
	bpt_record_t* v = n1->recs.l_rec.r_arr[n1->num_of_rec - 1];
	bpt_insert_in_leaf_at(n, 0, 0, k, v);
	bpt_delete_in_leaf_at(n1, n1->num_of_rec - 1);

	/* Get the split key index of n and n1 */
	int ind = bpt_locate_in_parent(n1);
	bpt_replace_key_in_parent(n1->p, ind, n->key[0]);
}

/* Borrow on record from the second leaf node to the first leaf node. The second
 * node is the next node of the first node.
 */
void
bpt_borrow_from_post_leaf(bpt_node* n, bpt_node* n1)
{
	int num = n->num_of_rec;
	bpt_insert_in_leaf_at(n, num, num, n1->key[0], n1->recs.l_rec.r_arr[0]);
	bpt_delete_in_leaf_at(n1, 0);

	/* Get the split key index of n and n1 */
	int ind = bpt_locate_in_parent(n);
	bpt_replace_key_in_parent(n->p, ind, n1->key[0]);
}

/* Borrow on record from the second index node to the first index node. The 
 * second node is the previous node of the first node.
 */
void
bpt_borrow_from_pre_index(bpt_node* n, long k, bpt_node* n1)
{
	long new_key = n1->key[bpt_num_of_key(n1) - 1];
	bpt_insert_in_index_at(n, 0, 0, 
			k, n1->recs.c_arr[n1->num_of_rec - 1]);
	bpt_delete_in_index_at(n1, bpt_num_of_key(n1) - 1, n1->num_of_rec - 1);

	/* Get the split key index of n and n1 */
	int ind = bpt_locate_in_parent(n1);
	bpt_replace_key_in_parent(n1->p, ind, new_key);
}

/* Borrow on record from the second index node to the first index node. The 
 * second node is the next node of the first node.
 */
void
bpt_borrow_from_post_index(bpt_node* n, long k, bpt_node* n1)
{
	long new_key = n1->key[0];
	bpt_insert_in_index_at(n, bpt_num_of_key(n), n->num_of_rec, 
			k, n1->recs.c_arr[0]);
	bpt_delete_in_index_at(n1, 0, 0);

	/* Get the split key index of n and n1 */
	int ind = bpt_locate_in_parent(n);
	bpt_replace_key_in_parent(n->p, ind, new_key);
}

/* Return the index of record in a leaf node */
int
bpt_locate_in_leaf(bpt_node* n, bpt_record_t* v)
{
	/* TODO: replace with binary search */
	int ind;
	for(ind = 0; ind < n->num_of_rec; ind++)
		if(n->recs.l_rec.r_arr[ind] == v)
			return ind;
	assert(0); // Should never reach here
}	

void
bpt_delete_in_leaf(bpt_node* n, bpt_record_t* v)
{
	int ind = bpt_locate_in_leaf(n, v);
	bpt_delete_in_leaf_at(n, ind);
}

void
bpt_delete_in_index(bpt_node* n, bpt_node* v)
{
	int ind = bpt_locate_in_parent(v);
	bpt_delete_in_index_at(n, ind - 1, ind);
}

/* Only for delete the entry in node. Not ajust the tree yet */
void
bpt_delete_in_node(bpt_node* n, void* v)
{
	if(bpt_is_leaf(n))
		bpt_delete_in_leaf(n, v);
	else bpt_delete_in_index(n, v);
}

void
bpt_delete(bpt_node** root, long k, bpt_record_t* v)
{
	bpt_node* n = bpt_query(*root, k);
	/* n is the leaf node now. Delete record(v) from n */
	bpt_delete_entry(root, n, v);
}

/* Delete one entry from leaf node or index node; 
 * make ajustment to maintain bptree
 */
void
bpt_delete_entry(bpt_node** root, bpt_node* n, void* v)
{
	/* Only delete from node, no ajust yet. */
	bpt_delete_in_node(n, v);

	if(bpt_is_root(n)){
	       if(! bpt_is_enough(n) && !bpt_is_leaf(n)) 
			/* So root is not leaf node and only has one child */
			bpt_replace_root_with_child(root);
	}else if(! bpt_is_enough(n)){
	        /* Not enough record in the node now, so need ajustment. */	
		long k;
		bpt_node* n1;

		bpt_get_close_sibling(n, &n1, &k);

		if((n->num_of_rec + n1->num_of_rec) <= BPT_MAX_REC_NO){
			/* merge node and its close sibling */
			if(n->key[0] > n1->key[0])
				swap_pointer((void**)&n, (void**)&n1);

			if(bpt_is_leaf(n))
				bpt_merge_leaf(root, n, &n1);
			else bpt_merge_index(root, n, k, &n1);
		}else{
			/* borrow one entry from its close sibling */
			if(n->key[0] > n1->key[0]){
				if(bpt_is_leaf(n))
					bpt_borrow_from_pre_leaf(n,n1);
				else bpt_borrow_from_pre_index(n, k, n1);
			}else{
				if(bpt_is_leaf(n))
					bpt_borrow_from_post_leaf(n, n1);
				else bpt_borrow_from_post_index(n, k, n1);
			}
		}
	}
}


void 
print_level(int level)
{
	int i = 0;
	for(;i < level;i++)
		printf("\t");
}

void 
bpt_print_leaf_node(bpt_node* node, int level)
{
	int i, j;
	print_level(level);
	printf("##BEGIN LEAF NODE\n");
	for(i = 0; i < node->num_of_rec; i++){
		print_level(level);
		/* TODO: may add a callback so user could define how the leaf 
		 * node data will be printed. Currently print the pointer.
		 */
		printf("key:%ld, record:%ld\n", 
			node->key[i], node->recs.l_rec.r_arr[i]);
	}
	print_level(level);
	printf("##END LEAF NODE\n");
}

void 
bpt_print_node(bpt_node* root, int level)
{
	if(bpt_is_leaf(root))
		bpt_print_leaf_node(root, level);
	else{
		print_level(level);
		printf("##BEGIN INDEX NODE##\n");
		int i, j;
		/** print the first child **/
		bpt_print_node(root->recs.c_arr[0], level + 1);

		for(i = 0; i < bpt_num_of_key(root); i++){
			/** print the key and next child **/
			print_level(level);
			printf("key:%ld\n", root->key[i]);
			bpt_print_node(root->recs.c_arr[i+1], level + 1);
		}
		print_level(level);
		printf("##END INDEX NODE##\n");
	}
}

void
bpt_print_tree(bpt_node* root)
{
	printf("###########BEGIN PRINT TREE##########\n");
	bpt_print_node(root, 0);
	printf("#############END PRINT TREE##########\n\n");
}
