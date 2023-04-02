#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "vma.h"

#define MAX_STRING_SIZE 64

linked_list_t* ll_create(unsigned int data_size)
{
    linked_list_t *list = malloc(sizeof(linked_list_t));
    DIE(!list, "error!");
    
    list->data_size = data_size;
    list->size = 0;
    list->head = NULL;

    return list;
}

void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    if (!list) {
        printf("List not created");
        return;
    }

    ll_node_t* crt = list->head;
    ll_node_t *new = malloc(sizeof(ll_node_t));
    DIE(!new, "error!");
    new->data = malloc(list->data_size);
    DIE(!new->data, "error!");
    memcpy(new->data, new_data, list->data_size);

    if(n == 0 || !list->head) {
        new->next = list->head;
        list->head = new;
    } else {
        while (crt->next && n > 1) {
            crt = crt->next;
            n--;
        }
        new->next = crt->next;
        crt->next = new;    
    }
    list->size++;
}

ll_node_t* ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t* prev = list->head;
    ll_node_t* del = prev->next;;
    
    if (!list) {
        printf("List not created");
        return NULL;
    }

    if(n == 0) {
        list->head = list->head->next;
        return prev;
    } else {
        while(del->next && n > 1) {
            del = del->next;
            prev = prev->next;
            n--;
        }
        prev->next = del->next;;
    }
    list->size--;
    return del;
}

unsigned int ll_get_size(linked_list_t* list)
{
    if (!list) {
        printf("List not created");
        return -1;
    }
    return list->size;
}

void ll_free(linked_list_t** pp_list)
{
    if (!(*pp_list)) {
        printf("List not created");
        return;
    }
    ll_node_t *crt = (*pp_list)->head, *del;
	while (crt) {
		del = crt;
		crt = crt->next;
        free(del->data);
		free(del);
	}
    (*pp_list)->size = 0;
    (*pp_list)->head = NULL;
    free(*pp_list);
	*pp_list = NULL;
}

void ll_print_int(linked_list_t* list)
{
    if (!list) {
        printf("List not created");
        return;
    }
    ll_node_t *crt = list->head;
    int *value;
    while (crt) {
        value = (int*)crt->data;
        printf("%d ", *value);
        crt = crt->next;
    }
    printf("\n");
}

void ll_print_string(linked_list_t* list)
{
    if (!list) {
        printf("List not created");
        return;
    }
    ll_node_t *crt = list->head;
    char* str;
    while (crt) {
        str = (char*)crt->data;
        printf("%s ", str);
        crt = crt->next;
    }
    printf("\n");
}
