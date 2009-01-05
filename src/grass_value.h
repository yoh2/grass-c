/* $Id$ */
/*! \file
 * \brief 値関連の定義。
 */
#ifndef grass_value_H_
#define grass_value_H_

#include "grass_instruction.h"

/* 値関連の型 */

struct grass_value_node;

/*!
 * クロージャ。
 * コードと環境の組。
 */
struct grass_closure
{
	struct grass_instruction_node *code;
	struct grass_value_node *env;
};

/*!
 * 数値型。
 */
struct grass_numeric
{
	int n;
};

enum grass_value_type
{
	GRASS_VT_CLOSURE, /*!< \brief クロージャ */
	GRASS_VT_OUT,     /*!< \brief Outプリミティブ */
	GRASS_VT_IN,      /*!< \brief Inプリミティブ */
	GRASS_VT_SUCC,    /*!< \brief Succプリミティブ */
	GRASS_VT_NUMERIC  /*!< \brief 数値型 (兼同値判定関数) */
};


struct grass_value
{
	enum grass_value_type type;
	union /* Out, In, Succ には特に保存しておく値なし。 */
	{
		struct grass_closure closure;
		struct grass_numeric numeric;
	} content;
};


struct grass_value_node
{
	struct grass_value value;
	struct grass_value_node *next;
};


struct grass_value_node *
grass_create_closure_node(struct grass_instruction_node *code, struct grass_value_node *env);

struct grass_value_node *
gras_create_out_func_node(void);

struct grass_value_node *
gras_create_in_func_node(void);

struct grass_value_node *
gras_create_succ_func_node(void);

struct grass_value_node *
gras_create_numeric_node(int n);



#endif /* grass_value_H_ */
