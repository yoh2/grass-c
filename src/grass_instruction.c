/* $Id$ */
/*! \file
 * \brief コード関連の定義。
 *
 * \date 2009-01-06
 * \author yoh2
 * $LastChangedBy$
 * $LastChangedDate$
 */
#include "grass_instruction.h"
#include <gc.h>
#include <assert.h>


/*! 
 * 関数適用のノードを作成する。
 * ノードはBoehm GCによって作成されるので、明示的な開放の必要なし。
 *
 * \param func_index 関数のインデックス。
 * \param arg_index 引数のインデックス。
 *
 * \return 関数適用のノード。失敗時はNULL。
 */
struct grass_instruction_node *
grass_create_application_node(size_t func_index, size_t arg_index)
{
	struct grass_instruction_node *new_node
		= (struct grass_instruction_node *)GC_MALLOC(sizeof(new_node[0]));
	if(new_node == NULL)
	{
		return NULL;
	}

	new_node->inst.type = GRASS_IT_APPLICATION;
	new_node->inst.content.app.func_index = func_index;
	new_node->inst.content.app.arg_index = arg_index;
	new_node->next = NULL;

	return new_node;
}


/*! 
 * 関数定義のノードを作成する。
 * ノードはBoehm GCによって作成されるので、明示的な開放の必要なし。
 *
 * \param func_index 関数のインデックス。
 * \param arg_index 引数のインデックス。
 *
 * \return 関数適用のノード。失敗時はNULL。
 */
struct grass_instruction_node *
grass_create_abstraction_node(size_t num_args, struct grass_instruction_node *code)
{
	struct grass_instruction_node *new_node
		= (struct grass_instruction_node *)GC_MALLOC(sizeof(new_node[0]));
	if(new_node == NULL)
	{
		return NULL;
	}

	new_node->inst.type = GRASS_IT_ABSTRACTION;
	new_node->inst.content.abs.num_args = num_args;
	new_node->inst.content.abs.code = code;
	new_node->next = NULL;

	return new_node;
}


struct grass_instruction_node *
grass_append_instruction_list(struct grass_instruction_node *list1, struct grass_instruction_node *list2)
{
	if(list1 == NULL)
	{
		return list2;
	}
	else
	{
		struct grass_instruction_node *list1_tail;
		for(list1_tail = list1; list1_tail->next != NULL; list1_tail = list1_tail->next)
			;
		list1_tail->next = list2;
		return list1;
	}
}

