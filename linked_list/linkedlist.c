#include "linkedlist.h"

int linked_list_init(linked_list_t** ll) 
{
	if(ll == NULL){
		printf("linked list pointer error \n");
		return -1;
	}
	/* do program */
	/* allocate linked list */
	/* initialize list head */
	/* initialize mutex lock for BLINKED_LIST version */
	(*ll) = malloc(sizeof(linked_list_t));
	(*ll)->list_head = malloc (sizeof(node_t));
	(*ll)->list_head->next = NULL;
#ifdef BLINKED_LIST
	int rc = pthread_mutex_init(&((*ll)->list_lock), NULL);
    assert(rc == 0);
#endif
	srand((unsigned) time (NULL));

	return 0;
}


void linked_list_destroy(linked_list_t* ll)
{
	/* do program */ 
	/* free all nodes in the linked list */
	/* free the linked list */ 
	node_t *next_n = ll->list_head;
	while(next_n != NULL){
		next_n = next_n->next;
		free(next_n);
	}
	free(ll->list_head);
	free(ll);
	return;
}


long linked_list_get (long key, linked_list_t* ll) 
{
	/* do program */
	/* if key is found, return value */
	/* if key is not found, return -1 */
	node_t *next_n = ll->list_head->next;
	while(next_n != NULL){
		if (next_n->key == key)
			return next_n->value;
		next_n = next_n->next;
	}
	return -1;
}

long linked_list_put (long key, long value, linked_list_t* ll)
{
	/* do program */
	/* if succeeds, return 0 */
	/* if fails, return -1 */
#ifdef BLINKED_LIST
	pthread_mutex_lock(&ll->list_lock);// lock 
	node_t *newNode = malloc(sizeof(node_t));
	newNode->next = ll->list_head->next;
	newNode->key = key;
	newNode->value = value;
	newNode->level = 0;
	ll->list_head->next = newNode;
	pthread_mutex_unlock(&ll->list_lock);// unlock
#else
	node_t *newNode = malloc(sizeof(node_t));
	newNode->key = key;
	newNode->value = value;
	newNode->level = 0;
	node_t *val =  ll->list_head->next;
	while(CAS(&(ll->list_head->next), val, newNode) == 0){
		val =  ll->list_head->next;
	}
	newNode->next = val;
#endif
	return 0;
}
