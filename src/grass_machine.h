/* $Id$ */
/*! \file
 * \brief Grass抽象機械
 *
 * \date 2009-01-06
 * \author yoh2
 * $LastChangedBy$
 * $LastChangedDate$
 */

#ifndef grass_machine_H_
#define grass_machine_H_

#include "grass_instruction.h"
#include "grass_value.h"

struct grass_machine
{
	struct grass_instruction_node *code;
	struct grass_value_node *env;
	struct grass_value_node *dump;
};


/* 初期状態のGrass抽象機械を作成する。 */
struct grass_machine *
create_grass_machine(struct grass_instruction_node *code);

#endif /* grass_machine_H_ */
