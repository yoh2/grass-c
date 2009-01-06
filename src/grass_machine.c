/* $Id$ */
/*! \file
 * \brief Grass抽象機械
 *
 * \date 2009-01-06
 * \author yoh2
 * $LastChangedBy$
 * $LastChangedDate$
 */

#include "grass_machine.h"
#include "grass_value.h"
#include <gc.h>
#include <assert.h>



static struct grass_value_node *
create_initial_environment(void)
{
	struct grass_value_node *initial_env = NULL;
	struct grass_value_node *node;


	node = gras_create_out_func_node();
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_env;
	initial_env = node;

	node = gras_create_numeric_node('w');
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_env;
	initial_env = node;

	node = gras_create_succ_func_node();
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_env;
	initial_env = node;

	node = gras_create_out_func_node();
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_env;
	initial_env = node;

	return initial_env;
}


static struct grass_value_node *
create_initial_dump(void)
{
	// TODO: implementation
	return NULL;
}


struct grass_machine *
create_grass_machine(struct grass_instruction_node *code)
{
	struct grass_machine *new_machine;

	new_machine = (struct grass_machine *)GC_MALLOC(sizeof(*new_machine));
	if(new_machine == NULL)
	{
		return NULL;
	}

	new_machine->code = code;
	new_machine->env = create_initial_environment();
	new_machine->dump = create_initial_dump();

	if((new_machine->env == NULL) || (new_machine->dump == NULL))
	{
		/* GC_FREEしておくべき？ */
		return NULL;
	}

	return new_machine;
}
