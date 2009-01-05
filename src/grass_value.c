/* $Id$ */
/*! \file
 * \brief 値関連の定義。
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
gras_create_out_func_node(void)
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
gras_create_in_func_node(void)
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
gras_create_succ_func_node(void)
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
gras_create_numeric_node(int n)
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


