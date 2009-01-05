/* $Id$ */
/*! \file
 * \brief コード関連の定義。
 */
#ifndef grass_instruction_H_
#define grass_instruction_H_
/* コード関連の型 */

#include <stddef.h>

struct grass_instruction_node;

struct grass_application
{
	size_t func_index;
	size_t arg_index;
};

struct grass_abstraction
{
	size_t num_args;
	struct grass_instruction_node *code;
};


enum grass_instruction_type
{
	GRASS_IT_APPLICATION, /*!< \brief 関数適用 */
	GRASS_IT_ABSTRACTION  /*!< \brief ラムダ抽象 (関数定義) */
};

struct grass_instruction
{
	enum grass_instruction_type type;
	union
	{
		struct grass_application app;
		struct grass_abstraction abs;
	} content;
};


struct grass_instruction_node
{
	struct grass_instruction inst;
	struct grass_instruction_node *next;
};


/*! \brief 関数適用のノードを作成する。 */
struct grass_instruction_node *
grass_create_application_node(size_t func_index, size_t arg_index);

/*! \brief 関数定義のノードを作成する。 */
struct grass_instruction_node *
grass_create_abstraction_node(size_t num_args, struct grass_instruction_node *code);

#endif /* grass_instruction_H_ */
