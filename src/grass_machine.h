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

#include "grass_fwd.h"

struct grass_machine
{
	struct grass_instruction_node *code;
	struct grass_value_node *env;
	struct grass_value_node *dump;
};


/* 初期状態のGrass抽象機械を作成する。 */
struct grass_machine *
grass_create_machine(struct grass_instruction_node *code);

int
grass_step_machine(struct grass_machine *machine, char **error_message);

int
grass_machine_done(const struct grass_machine *machine);

void
grass_dump_machine(const struct grass_machine *machine);
#endif /* grass_machine_H_ */
