/* $Id$ */
/*! \file
 * \brief コンパイル時に評価されるアサーション。
 *
 * \date 2009-01-06
 * \author yoh2
 * $LastChangedBy$
 * $LastChangedDate$
 */
#ifndef static_assert_H_
#define static_assert_H_

/*!
 * \a exp がゼロの時にコンパイルエラーになるマクロ。
 *
 * エラー時のメッセージは、大抵、「長さが負の配列が宣言された」
 * といった感じのものになる。
 *
 * 関数宣言を使って実装しているので、これを書ける位置は、
 * 	- グローバルスコープ。
 * 	- 関数定義中のブロック先頭。
 * のいずれかになる。
 * (C99なら、関数定義中のどこでも書ける)
 *
 * \param exp 正当性をチェックしたい式。当然、
 */
#define STATIC_ASSERT(exp)  void static_assert_dummy_func_(int [][(exp)?1:-1]);

#endif /* static_assert_H_ */
