/* $Id$ */
/*! \file
 * \brief 値関連の定義。
 *
 * \date 2009-01-06
 * \author yoh2
 * $LastChangedBy$
 * $LastChangedDate$
 */
#ifndef grass_value_H_
#define grass_value_H_

#include <stddef.h>
#include "grass_fwd.h"

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
 * 関数としては、自身が持つ数値と引数が持つ数値が等しいかの判定を行う。
 */
struct grass_numeric
{
	int n;
};


/*! 値の種類。 */
enum grass_value_type
{
	GRASS_VT_CLOSURE, /*!< \brief クロージャ */
	GRASS_VT_OUT,     /*!< \brief Outプリミティブ */
	GRASS_VT_IN,      /*!< \brief Inプリミティブ */
	GRASS_VT_SUCC,    /*!< \brief Succプリミティブ */
	GRASS_VT_NUMERIC  /*!< \brief 数値型 (兼同値判定関数) */
};


/*!
 * すべての値関連の型をひとまとめにしたもの。
 */
struct grass_value
{
	enum grass_value_type type;
	union /* Out, In, Succ には特に保存しておく値なし。 */
	{
		struct grass_closure closure; /*!< \brief クロージャ */
		struct grass_numeric numeric; /*!< \brief 数値 */
	} content;
};


/*!
 * 値型のリストを構成するノード。
 */
struct grass_value_node
{
	struct grass_value value;
	struct grass_value_node *next;
};


/*!
 * \brief 値型のノードを複製する。
 */
struct grass_value_node *
grass_create_value_node(const struct grass_value *node);

/*!
 * \brief 内容としてクロージャを持つノードを作成する。
 */
struct grass_value_node *
grass_create_closure_node(struct grass_instruction_node *code, struct grass_value_node *env);

/*!
 * \brief 内容としてOutプリミティブを持つノードを作成する。
 */
struct grass_value_node *
grass_create_out_func_node(void);

/*!
 * \brief 内容としてInプリミティブを持つノードを作成する。
 */
struct grass_value_node *
grass_create_in_func_node(void);

/*!
 * \brief 内容としてSuccプリミティブを持つノードを作成する。
 */
struct grass_value_node *
grass_create_succ_func_node(void);

/*!
 * \brief 内容として数値型を持つノードを作成する。
 */
struct grass_value_node *
grass_create_numeric_node(int n);

/*!
 * \brief 値リストの \a n 番目のノードを取得する。
 */
struct grass_value_node *
grass_get_nth_value_node(struct grass_value_node *node, size_t n);

/*!
 * \brief \a list1 と \a list2 を結合する。
 */
struct grass_value_node *
grass_append_value_list(struct grass_value_node *list1, struct grass_value_node *list2);

/*!
 * \a func に \a arg を適用し、抽象機械の状態を更新する。
 */
int
grass_apply(struct grass_machine *machine,
            const struct grass_value *func,
            const struct grass_value *arg,
            char **error_message);


/*! \brief 値のリスト(環境 or Dump)を出力する。 */
void
grass_dump_value_list(const struct grass_value_node *value_list);

#endif /* grass_value_H_ */
