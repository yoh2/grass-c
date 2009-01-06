/* $Id$ */
/*! \file
 * \brief 値関連の定義。
 *
 * \date 2009-01-06
 * \author yoh2
 * $LastChangedBy$
 * $LastChangedDate$
 */

#include "grass_value.h"
#include <gc.h>
#include <assert.h>


struct grass_value_node *
grass_create_closure_node(struct grass_instruction_node *code, struct grass_value_node *env)
{
	struct grass_value_node *new_node
		= (struct grass_value_node *)GC_MALLOC(sizeof(new_node[0]));
	if(new_node == NULL)
	{
		return NULL;
	}

	new_node->value.type = GRASS_VT_CLOSURE;
	new_node->value.content.closure.code = code;
	new_node->value.content.closure.env = env;
	new_node->next = NULL;

	return new_node;
}

struct grass_value_node *
grass_create_out_func_node(void)
{
	struct grass_value_node *new_node
		= (struct grass_value_node *)GC_MALLOC(sizeof(new_node[0]));
	if(new_node == NULL)
	{
		return NULL;
	}

	new_node->value.type = GRASS_VT_OUT;
	new_node->next = NULL;

	return new_node;
}


struct grass_value_node *
grass_create_in_func_node(void)
{
	struct grass_value_node *new_node
		= (struct grass_value_node *)GC_MALLOC(sizeof(new_node[0]));
	if(new_node == NULL)
	{
		return NULL;
	}

	new_node->value.type = GRASS_VT_IN;
	new_node->next = NULL;

	return new_node;
}


struct grass_value_node *
grass_create_succ_func_node(void)
{
	struct grass_value_node *new_node
		= (struct grass_value_node *)GC_MALLOC(sizeof(new_node[0]));
	if(new_node == NULL)
	{
		return NULL;
	}

	new_node->value.type = GRASS_VT_SUCC;
	new_node->next = NULL;

	return new_node;
}


struct grass_value_node *
grass_create_numeric_node(int n)
{
	struct grass_value_node *new_node
		= (struct grass_value_node *)GC_MALLOC(sizeof(new_node[0]));
	if(new_node == NULL)
	{
		return NULL;
	}

	assert((0 <= n) && (n <= 255));

	new_node->value.type = GRASS_VT_NUMERIC;
	new_node->value.content.numeric.n = n;
	new_node->next = NULL;

	return new_node;
}


struct grass_value_node *
grass_create_true_node(void)
{
	struct grass_instruction_node *true_code;
	struct grass_instruction_node *abs_code;

	abs_code = grass_create_application_node(2, 3);
	if(abs_code == NULL)
	{
		return NULL;
	}

	true_code = grass_create_abstraction_node(1, abs_code);
	if(true_code == NULL)
	{
		return NULL;
	}

	return grass_create_closure_node(true_code, NULL);
}

struct grass_value_node *
grass_create_false_node(void)
{
	struct grass_instruction_node *false_code = NULL;

	false_code = grass_create_abstraction_node(1, NULL);
	if(false_code == NULL)
	{
		return NULL;
	}

	return grass_create_closure_node(false_code, NULL);
}

struct grass_value_node *
grass_get_nth_value_node(struct grass_value_node *node, size_t n)
{
	assert(n > 0);

	while((--n > 0) && (node != NULL))
	{
		node = node->next;
	}

	return node;
}


struct grass_value_node *
grass_append_value_list(struct grass_value_node *list1, struct grass_value_node *list2)
{
	if(list1 == NULL)
	{
		return list2;
	}
	else
	{
		struct grass_value_node *list1_tail;
		for(list1_tail = list1; list1_tail->next != NULL; list1_tail = list1_tail->next)
			;
		list1_tail->next = list2;
		return list1;
	}
}
