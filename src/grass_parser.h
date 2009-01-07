/* $Id$ */
/*! \file
 * \brief $B%U%!%$%k$rFI$_9~$s$G(B grass_instruction_node * $B$K$h$k%j%9%H$r:n$k!#(B
 *
 * \date 2009-01-06
 * \author yoh2
 * $LastChangedBy$
 * $LastChangedDate$
 */
#ifndef grass_parser_H_
#define grass_parser_H_

#include "grass_fwd.h"
#include <stdio.h>

struct grass_instruction_node *
grass_parse_source(FILE *in, char **error_message);

#endif /* grass_parser_H_ */
