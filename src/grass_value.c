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
#include "grass_instruction.h"
#include "grass_machine.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gc.h>
#include <assert.h>


struct grass_value_node *
grass_create_value_node(const struct grass_value *value)
{
	struct grass_value_node *new_node;

	assert(value != NULL);

	new_node = (struct grass_value_node *)GC_MALLOC(sizeof(new_node[0]));
	if(new_node == NULL)
	{
		return NULL;
	}

	new_node->value = *value;
	new_node->next = NULL;

	return new_node;
}


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


static struct grass_value_node *
grass_create_true_node(void)
{
	/*
	 * 関数
	 *
	 * Abs(2, App(3, 2)::ε)
	 * -- 第一引数のひとつ前に存在する関数を第1引数に適用したものを返す。
	 *
	 * と、環境
	 *
	 * (ε, ε)::ε
	 * -- 第1引数をそのまま返す1引数関数 (Abs(1, ε)) と空の環境との
	 *    組をクロージャ化したもの ((ε, ε)) のみが積まれた環境
	 *
	 * との組をクロージャ化したもの、つまり
	 *
	 * (Abs(1, App(3, 2)::ε), (ε, ε)::ε)
	 *
	 * を作成する。
	 */
	struct grass_instruction_node *abs_code;
	struct grass_instruction_node *true_code;
	struct grass_value_node *env_node;

	abs_code = grass_create_application_node(3, 2);
	if(abs_code == NULL)
	{
		return NULL;
	}

	true_code = grass_create_abstraction_node(1, abs_code);
	if(true_code == NULL)
	{
		return NULL;
	}

	env_node = grass_create_closure_node(NULL, NULL);
	if(env_node == NULL)
	{
		return NULL;
	}

	return grass_create_closure_node(true_code, env_node);
}

static struct grass_value_node *
grass_create_false_node(void)
{
	/*
	 * 関数
	 *
	 * Abs(2, ε)  -- 何もしない2引数関数 → 戻り値が第2引数になる
	 *
	 * と、空の環境との組をクロージャ化したもの、つまり
	 *
	 * (Abs(1, ε)::ε, ε)
	 *
	 * を作成する。
	 */
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

static int
grass_apply_to_closure(struct grass_machine *machine,
                       const struct grass_value *func,
                       const struct grass_value *arg,
                       char **error_message)
{
	struct grass_value_node *env_node;
	struct grass_value_node *dump_node;

	assert(machine != NULL);
	assert(func != NULL);
	assert(arg != NULL);
	assert(error_message != NULL);

	assert(func->type == GRASS_VT_CLOSURE);

	/* (App(m, n)::C, E, D) → (Cm, (Cn, En)::Em, (C, E)::D)
	 * 	where E = (C1, E1)::(C2, E2):: ... ::(Ci, Ei)::E' (i = m, n)
	 */

	env_node = grass_create_value_node(arg);
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

	machine->code = func->content.closure.code;
	env_node->next = func->content.closure.env;
	machine->env = env_node;
	dump_node->next = machine->dump;
	machine->dump = dump_node;

	return 1;
}

static int
grass_apply_to_out(struct grass_machine *machine,
                   const struct grass_value *func,
                   const struct grass_value *arg,
                   char **error_message)
{
	struct grass_value_node *env_node;
	int n;

	assert(machine != NULL);
	assert(func != NULL);
	assert(arg != NULL);
	assert(error_message != NULL);

	assert(func->type == GRASS_VT_OUT);

	if(arg->type != GRASS_VT_NUMERIC)
	{
		*error_message = "runtime error: non-numeric value could not applyed to Out.";
		return 0;
	}
	n = arg->content.numeric.n;

	putchar(n);

	env_node = grass_create_numeric_node(n);
	if(env_node == NULL)
	{
		*error_message = strerror(errno);
		return 0;
	}
	machine->code = machine->code->next;
	env_node->next = machine->env;
	machine->env = env_node;

	return 1;
}


static int
grass_apply_to_in(struct grass_machine *machine,
                  const struct grass_value *func,
                  const struct grass_value *arg,
                  char **error_message)
{
	struct grass_value_node *env_node;
	int ch;

	assert(machine != NULL);
	assert(func != NULL);
	assert(arg != NULL);
	assert(error_message != NULL);

	assert(func->type == GRASS_VT_IN);

	ch = getchar();
	if(ch == EOF)
	{
		*error_message = "runtime error: unexpected EOF.";
		return 0;
	}
	env_node = grass_create_numeric_node(ch);

	if(env_node == NULL)
	{
		*error_message = strerror(errno);
		return 0;
	}
	machine->code = machine->code->next;
	env_node->next = machine->env;
	machine->env = env_node;

	return 1;
}


static int
grass_apply_to_succ(struct grass_machine *machine,
                    const struct grass_value *func,
                    const struct grass_value *arg,
                    char **error_message)
{
	struct grass_value_node *env_node;

	assert(machine != NULL);
	assert(func != NULL);
	assert(arg != NULL);
	assert(error_message != NULL);

	assert(func->type == GRASS_VT_SUCC);

	if(arg->type != GRASS_VT_NUMERIC)
	{
		*error_message = "runtime error: non-numeric value could not applyed to Succ.";
		return 0;
	}

	env_node = grass_create_numeric_node((arg->content.numeric.n + 1) & 0xff);

	if(env_node == NULL)
	{
		*error_message = strerror(errno);
		return 0;
	}
	machine->code = machine->code->next;
	env_node->next = machine->env;
	machine->env = env_node;

	return 1;
}


static int
grass_apply_to_numeric(struct grass_machine *machine,
                       const struct grass_value *func,
                       const struct grass_value *arg,
                       char **error_message)
{
	struct grass_value_node *env_node;

	assert(machine != NULL);
	assert(func != NULL);
	assert(arg != NULL);
	assert(error_message != NULL);

	assert(func->type == GRASS_VT_NUMERIC);

	if(arg->type != GRASS_VT_NUMERIC)
	{
		*error_message = "runtime error: non-numeric value could not applyed to numeric value.";
		return 0;
	}
	//assert(!"TODO: implementation");

	if(func->content.numeric.n == arg->content.numeric.n)
	{
		env_node = grass_create_true_node();
	}
	else
	{
		env_node = grass_create_false_node();
	}

	if(env_node == NULL)
	{
		*error_message = strerror(errno);
		return 0;
	}
	machine->code = machine->code->next;
	env_node->next = machine->env;
	machine->env = env_node;

	return 1;
}

/*!
 * \a func に \a arg を適用し、抽象機械の状態を更新する。
 */
int
grass_apply(struct grass_machine *machine,
            const struct grass_value *func,
            const struct grass_value *arg,
            char **error_message)
{
	assert(machine != NULL);
	assert(func != NULL);
	assert(arg != NULL);
	assert(error_message != NULL);

	switch(func->type)
	{
	case GRASS_VT_CLOSURE:
		return grass_apply_to_closure(machine, func, arg, error_message);

	case GRASS_VT_OUT:
		return grass_apply_to_out(machine, func, arg, error_message);

	case GRASS_VT_IN:
		return grass_apply_to_in(machine, func, arg, error_message);

	case GRASS_VT_SUCC:
		return grass_apply_to_succ(machine, func, arg, error_message);

	case GRASS_VT_NUMERIC:
		return grass_apply_to_numeric(machine, func, arg, error_message);

	default:
		assert(0); /* BUG! */
		return 0;
	}
}
