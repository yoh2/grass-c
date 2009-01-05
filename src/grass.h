#ifndef grass_H_
#define grass_H_

#include "grass_instruction.h"
#include "grass_value.h"


struct grass_machine
{
	struct grass_instruction_node *code;
	struct grass_value *env;
	struct grass_value *dump;
};

#endif /* grass_H_ */
