#include "bptree.h"

struct bpt_record_t
{
	long v;
};

bpt_record_t* new_record(long v)
{
	bpt_record_t* brtp = (bpt_record_t*) my_calloc(sizeof(bpt_record_t));
	brtp->v = v;
}

/* Show the process of insert 100 records into bptree */
int
test1()
{
	int i;
	bpt_node* root = NULL;
	bpt_record_t* rec[100];
	for(i = 0; i < 100; i++)
		rec[i] = new_record(i);
	for(i = 0; i < 100; i++){
		bpt_insert(&root, i, rec[i]);
		bpt_print_tree(root);
	}
	for(i = 0; i < 100; i++)
		free(rec[i]);
	return 0;
}

/* Show the process of delete 100 nodes from bptree, from the biggest record to 
 * the smallest record. 
 */
int
test2()
{
	int i;
	bpt_node* root = NULL;
	bpt_record_t* rec[100];
	for(i = 0; i < 100; i++)
		rec[i] = new_record(i);
	for(i = 0 ;i < 100; i++)
		bpt_insert(&root, i, rec[i]);
	bpt_print_tree(root);
	for(i = 99; i >= 0; i--){
		bpt_delete(&root, i, rec[i]);
		bpt_print_tree(root);
	}
	for(i = 0; i < 100; i++)
		free(rec[i]);
	return 0;
}

/* Show the process of delete 100 nodes from bptree, from the smallest record to
 * the biggest record. */
int
test3()
{
	long i;
	bpt_node* root = NULL;
	bpt_record_t* rec[100];
	for(i = 0; i < 100; i++)
		rec[i] = new_record(i);
	for(i = 0 ;i < 100; i++)
		bpt_insert(&root, i, rec[i]);
	bpt_print_tree(root);
	for(i = 0; i < 100; i++){
		bpt_delete(&root, i, rec[i]);
		bpt_print_tree(root);
	}
	for(i = 0; i < 100; i++)
		free(rec[i]);
	return 0;
}

int 
main()
{
	//test1();
	//test2();
	test3();
	return 0;
}
