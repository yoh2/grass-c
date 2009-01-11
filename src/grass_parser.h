/* $Id$ */
/*! \file
 * \brief ファイルを読み込んで grass_instruction_node * によるリストを作る。
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
