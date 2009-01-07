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
#include <wchar.h>
#include <errno.h>
#include <string.h>
#include <assert.h>





/*!
 * 初期環境を作成する。
 *
 * 初期環境: Out :: Succ :: w :: In :: ε
 *
 * \return 初期環境。失敗時は NULL 。
 */
static struct grass_value_node *
create_initial_environment(void)
{
	struct grass_value_node *initial_env = NULL;
	struct grass_value_node *node;

	node = grass_create_in_func_node();
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_env;
	initial_env = node;

	node = grass_create_numeric_node('w');
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_env;
	initial_env = node;

	node = grass_create_succ_func_node();
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_env;
	initial_env = node;

	node = grass_create_out_func_node();
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
	struct grass_value_node *initial_dump = NULL;
	struct grass_value_node *node;
	struct grass_instruction_node *code;

	/* (ε, ε) */
	node = grass_create_closure_node(NULL, NULL);
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_dump;
	initial_dump = node;

	/* (App(1, 1) :: ε, ε) */
	code = grass_create_application_node(1, 1);
	if(code == NULL)
	{
		return NULL;
	}
	node = grass_create_closure_node(code, NULL);
	if(node == NULL)
	{
		return NULL;
	}
	node->next = initial_dump;
	initial_dump = node;

	return initial_dump;
}


struct grass_machine *
grass_create_machine(struct grass_instruction_node *code)
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


int
grass_step_machine(struct grass_machine *machine, char **error_message)
{
	char *dummy_error_message;

	assert(machine != NULL);
	assert(!grass_machine_done(machine));

	if(error_message == NULL)
	{
		error_message = &dummy_error_message;
	}
	*error_message = NULL;

	if(machine->env == NULL)
	{
		*error_message = "runtime error: internal error.";
		return 0;
	}

	if(machine->code == NULL)
	{
		/* (ε, f::E, (C', E')::D) → (C', f::E', D) */
		struct grass_value_node *dump_top = machine->dump;

		assert(dump_top->value.type == GRASS_VT_CLOSURE);

		machine->code = dump_top->value.content.closure.code;
		machine->env->next = dump_top->value.content.closure.env;
		machine->dump = machine->dump->next;
	}
	else if(machine->code->inst.type == GRASS_IT_APPLICATION)
	{
		/* (App(m, n)::C, E, D) → (Cm, (Cn, En)::Em, (C, E)::D)
		 * 	where E = (C1, E1)::(C2, E2):: ... ::(Ci, Ei)::E' (i = m, n)
		 */
		struct grass_instruction_node *code_top = machine->code;
		struct grass_value_node *func_node;
		struct grass_value_node *arg_node;

		func_node = grass_get_nth_value_node(
		                  machine->env,
		                  code_top->inst.content.app.func_index);
		arg_node = grass_get_nth_value_node(
		                 machine->env,
		                 code_top->inst.content.app.arg_index);
		if((func_node == NULL) || (arg_node == NULL))
		{
			*error_message = "runtime error: stack out of range.";
			return 0;
		}

		switch(func_node->value.type)
		{
		case GRASS_VT_CLOSURE:
			{
				struct grass_value_node *env_node;
				struct grass_value_node *dump_node;

				env_node = grass_clone_value_node(arg_node);
				if(env_node == NULL)
				{
					*error_message = strerror(errno);
					return 0;
				}

				dump_node = grass_create_closure_node(
				                  machine->code->next,
				                  machine->env);
				if(dump_node == NULL)
				{
					*error_message = strerror(errno);
					return 0;
				}

				machine->code = func_node->value.content.closure.code;
				env_node->next = func_node->value.content.closure.env;
				machine->env = env_node;
				dump_node->next = machine->dump;
				machine->dump = dump_node;
			}
			break;

		case GRASS_VT_OUT:
			{
				struct grass_value_node *env_node;
				int n;

				if(arg_node->value.type != GRASS_VT_NUMERIC)
				{
					void dump_machine(const struct grass_machine *machine);
					wprintf(L"arg_node->value.type = %d\n", arg_node->value.type);
					dump_machine(machine);
					*error_message = "runtime error: non-numeric value could not applyed to Out.";
					return 0;
				}
				n = arg_node->value.content.numeric.n;

				/* TODO: 真っ当な int(char) → wchar_t 変換をかませる */
				putwchar(n);

				env_node = grass_create_numeric_node(n);
				if(env_node == NULL)
				{
					*error_message = strerror(errno);
					return 0;
				}
				machine->code = machine->code->next;
				env_node->next = machine->env;
				machine->env = env_node;
			}
			break;

		case GRASS_VT_IN:
			{
				struct grass_value_node *env_node;
				wint_t ch;

				ch = getwchar();
				if(ch == WEOF)
				{
					*error_message = "runtime error: unexpected EOF.";
					return 0;
				}
				/* TODO: 真っ当な wint_t(wchar_t) → int 変換をかませる */
				env_node = grass_create_numeric_node((int)ch);

				if(env_node == NULL)
				{
					*error_message = strerror(errno);
					return 0;
				}
				machine->code = machine->code->next;
				env_node->next = machine->env;
				machine->env = env_node;
			}
			break;

		case GRASS_VT_SUCC:
			{
				struct grass_value_node *env_node;

				if(arg_node->value.type != GRASS_VT_NUMERIC)
				{
					*error_message = "runtime error: non-numeric value could not applyed to Succ.";
					return 0;
				}

				env_node = grass_create_numeric_node((arg_node->value.content.numeric.n + 1) & 0xff);

				if(env_node == NULL)
				{
					*error_message = strerror(errno);
					return 0;
				}
				machine->code = machine->code->next;
				env_node->next = machine->env;
				machine->env = env_node;
			}
			break;

		case GRASS_VT_NUMERIC:
			{
				struct grass_value_node *bool_value;

				if(arg_node->value.type != GRASS_VT_NUMERIC)
				{
					*error_message = "runtime error: non-numeric value could not applyed to numeric value.";
					return 0;
				}
				//assert(!"TODO: implementation");

				if(func_node->value.content.numeric.n == arg_node->value.content.numeric.n)
				{
					bool_value = grass_create_true_node();
				}
				else
				{
					bool_value = grass_create_false_node();
				}

				if(bool_value == NULL)
				{
					*error_message = strerror(errno);
					return 0;
				}

				machine->code = machine->code->next;
				machine->env = grass_append_value_list(bool_value, machine->env);
			}
			break;
		}
	}
	else if(machine->code->inst.content.abs.num_args == 1)
	{
		/* (Abs(n, C')::C, E, D) → (C, (C', E)::E, D)
		 * 	if n = 1
		 */
		struct grass_instruction_node *code_top = machine->code;
		struct grass_value_node *closure_node;

		machine->code = code_top->next;

		closure_node = grass_create_closure_node(
		                     code_top->inst.content.abs.code,
		                     machine->env);
		if(closure_node == NULL)
		{
			*error_message = strerror(errno);
			return 0;
		}
		closure_node->next = machine->env;
		machine->env = closure_node;
	}
	else
	{
		/* (Abs(n, C')::C, E, D) → (C, (Abs(n-1, C')::ε, E)::E, D)
		 * 	if n > 1
		 */
		struct grass_instruction_node *code_top = machine->code;
		struct grass_instruction_node *abs_node; /* Abs(n-1, C')::ε */
		struct grass_value_node *closure_node;   /* (Abs(n-1, C')::ε, E) */

		machine->code = code_top->next;

		abs_node = grass_create_abstraction_node(
		                 code_top->inst.content.abs.num_args - 1,
		                 code_top->inst.content.abs.code);
		if(abs_node == NULL)
		{
			*error_message = strerror(errno);
			return 0;
		}
		closure_node = grass_create_closure_node(
		                     abs_node,
		                     machine->env);
		if(closure_node == NULL)
		{
			*error_message = strerror(errno);
			return 0;
		}
		closure_node->next = machine->env;
		machine->env = closure_node;
	}

	return 1;
}


int
grass_machine_done(const struct grass_machine *machine)
{
	assert(machine != NULL);

	return (machine->code == NULL) && (machine->dump == NULL);
}
