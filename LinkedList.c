#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "vma.h"

#define MAX_STRING_SIZE 64

doubly_linked_list_t*
dll_create(unsigned int data_size)
{
	doubly_linked_list_t *list = malloc(sizeof(doubly_linked_list_t));
	DIE(!list, "malloc fail!");
	list->data_size = data_size;
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	return list;
}

dll_node_t*
dll_get_nth_node(doubly_linked_list_t* list, unsigned int n)
{
	if (!list) {
		printf("List not created");
		return NULL;
	}
	if (list->size == 0) {
		printf("List is empty!");
		return NULL;
	}

	dll_node_t *crt = list->head;
	if (n > list->size)
		return list->tail;
	
	while (crt->next && n != 0) {
		crt = crt->next;
		n--;
	}
	return crt;
}

void
dll_add_nth_node(doubly_linked_list_t* list, unsigned int n, const void* new_data)
{
	if (!list) {
	    printf("List not created");
        return;
    }
    dll_node_t* crt;
    dll_node_t *new = malloc(sizeof(dll_node_t));
    DIE(!new, "malloc fail!");
    new->data = malloc(list->data_size);
    DIE(!new->data, "malloc fail!");
    memcpy(new->data, new_data, list->data_size);

    if (!list->head) {
        new->next = NULL;
		new->prev = NULL;
        list->head = new;
		list->tail = new;
    } else if (n == 0) {
		new->next = list->head;
		new->prev = NULL;
		list->head->prev = new;
		list->head = new;
	} else if (n >= list->size) {
		new->next = NULL;
		new->prev = list->tail;
		list->tail->next = new;
		list->tail = new;
	} else {
		crt = list->head;
		while (crt->next && n > 1) {
			crt = crt->next;
			n--;
		}
		new->next = crt->next;
		new->prev = crt;
		crt->next->prev = new;
		crt->next = new;
	}   
    list->size++;
}

dll_node_t*
dll_remove_nth_node(doubly_linked_list_t* list, unsigned int n)
{
    if (!list) {
        printf("List not created");
        return NULL;
	}
	if (list->size == 0) {
		printf("List is empty");
		return NULL;
	}
	dll_node_t* crt;	
	if (n == 0) {
		crt = list->head;
		list->head->next->prev = NULL; 
		list->head = list->head->next;
	} else if (n >= list->size - 1) {
		crt = list->tail;
		list->tail->prev->next = NULL;
		list->tail = list->tail->prev;
	} else {
		crt = list->head;
		while(crt->next && n > 0) {
			crt = crt->next;
			n--;
		} 
		crt->prev->next = crt->next;
		crt->next->prev = crt->prev;
	}
    list->size--;
    return crt;
}

unsigned int
dll_get_size(doubly_linked_list_t* list)
{
	 if (!list) {
        printf("List not created");
        return -1;
    }
    return list->size;
}

void
dll_free(doubly_linked_list_t** pp_list)
{
	 if (!(*pp_list)) {
        printf("List not created");
        return;
    }
    dll_node_t *crt = (*pp_list)->head, *del;
	while ((*pp_list)->size) {
		del = crt;
		crt = crt->next;
        free(del->data);
		free(del);
		(*pp_list)->size--;
	}
    (*pp_list)->size = 0;
    (*pp_list)->head = NULL;
    free(*pp_list);
	*pp_list = NULL;
}

void
dll_print_int(doubly_linked_list_t* list)
{
    if (!list) {
        printf("List not created");
        return;
    }
    dll_node_t *crt = list->head;
    int *value;
    while (crt) {
        value = (int*)crt->data;
        printf("%d ", *value);
        crt = crt->next;
    }
    printf("\n");
}

void
dll_print_string(doubly_linked_list_t* list)
{
    if (!list) {
        printf("List not created");
        return;
    }
    dll_node_t *crt = list->head;
    char* str;
    while (crt) {
        str = (char*)crt->data;
        printf("%s ", str);
        crt = crt->next;
    }
    printf("\n");
}
