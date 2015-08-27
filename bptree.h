/* Copyright(c) Brayden Zhang.
 * Mail: pczhang2010@gmail.com
 */

#ifndef _BPT_H
#define _BPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/queue.h>

#include "bpt_utils.h"

/* Definition of a B_Plus_Tree(referenced from Database System Concept):
 * 0. Assume M > 2;
 * 1. Non-leaf root node has [2, M] children; leaf root node can have only
 *    one child or zero child;
 * 2. Non-root nodes can have [roof(M/2), M] children;
 * 3. Leaf nodes are in the same level. They store keys(as K[i]) of the 
 *    file records and the pointers(as P[i]) to the file records. 
 *    For i < j, K[i] <= K[j]. 
 *    All the leaf nodes are linked to each other, ordered by the key value.
 * 4. For nodes which are not leaf, they store keys(as K[i]) and
 *    pointers(as P[i]) to children. For i < j, K[i] <= K[j]. 
 *    Say the node contains N keys and N+1 children:
 *    For child pointed by P[0], all its keys should < K[0];
 *    For child pointed by P[N], all its keys should >= K[N-1];
 *    For child pointed by P[i](0 < i < N), all its keys should >= K[i-1] and
 *    < K[i].
 */

/* The max number of children in index node;
 * Also the max number of records in leaf node. 
 * TODO: separate the two numbers for index node and leaf node.
 */ 
#define BPT_MAX_REC_NO 4 

typedef enum { LEAF, INDEX } bpt_node_t;

/* The record struct stored in the leaf node */
typedef struct bpt_record_t bpt_record_t;
/* The record struct should be defined in client code. */
struct bpt_record_t;

typedef struct __bpt_node bpt_node;

struct __bpt_node
{
	/* Type of this node, should always be LEAF or INDEX */
	bpt_node_t t;

	/* Parent node of this node. For root node, parent is NULL */
	bpt_node* p;
	
	/* Current record number in this node.
	 * For index node, num_of_rec == 'key number' -1 == 'children number';
	 * for leaf node, num_of_rec == 'key number' == 'record number'
	 */
	int num_of_rec;

	/* Array for the keys. */
	long key[BPT_MAX_REC_NO];

	union
	{
		/* Array for the children of index node */
		bpt_node* c_arr[BPT_MAX_REC_NO + 1];

		struct
		{
			/* Array for the records of leaf node */
			bpt_record_t* r_arr[BPT_MAX_REC_NO];

			/* Pointer to next/previous leaf node */
			TAILQ_ENTRY (bpt_node) n;
		} l_rec;
	} recs;
};

/* bptree struct is not used yet. Currently we access bptree using root node.
 * TODO: use bptree struct to access the B-Plus-Tree.
 */
typedef struct __bptree bptree;
struct __bptree
{
	/* root node of the tree */
	bpt_node* root;

	/* Head of the link list of leaf nodes */
	TAILQ_HEAD (rec_list, bpt_node) rec_list_head;

	/* Function to insert a (k, v) pair into bptree */
	void (*insert) (bpt_node** root, long k, bpt_record_t* v);

	/* Function to remove a (k, v) pair from bptree */
	void (*remove) (bpt_node** root, long k, bpt_record_t* v);

	/* Function to print the bptree */
	void (*print) (bpt_node** root);

	/* Function to init the bptree */
	void (*init) (struct __bptree bpt);

};

#endif /* end of _BPT_H */
