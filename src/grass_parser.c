/* $Id$ */
/*! \file
 * \brief ファイルを読み込んで grass_instruction_node * によるリストを作る。
 *
 * \date 2009-01-06
 * \author yoh2
 * $LastChangedBy$
 * $LastChangedDate$
 */

#include "grass_parser.h"
#include "grass_instruction.h"
#include "grass_value.h"
#include <wchar.h>
#include <string.h>
#include <gc.h>
#include <errno.h>

#include "static_assert.h"
#include <assert.h>

STATIC_ASSERT((char)L'W' == 'W');
STATIC_ASSERT((char)L'w' == 'w');
STATIC_ASSERT((char)L'v' == 'v');
STATIC_ASSERT('w' == 119);


/*! Grassソースのトークン。
 * 連続したW、連続したw、vのいずれか。
 */
struct grass_token
{
	/*!
	 * - L'W': 連続したW。
	 * - L'w': 連続したw。
	 * - L'v': v一文字。
	 * - WEOF: ファイル終端またはエラー。
	 * - その他: 不正。
	 */
	wint_t type;
	size_t n; /*!< 'W'または'w'がいくつ連続しているか。 type がそれ以外なら意味を持たない。 */
};


/*! ソース読み込みコンテキスト */
struct grass_read_context
{
	/*! ungot_token.type != L'\0' ならば押し戻されたトークンが入っている。 */
	struct grass_token ungot_token;
};

/*! ソース読み込みコンテキストの初期化子。 */
#define GRASS_READ_CONTEXT_INITIALIZER  { { 0 } }


/*! ソースを一文字読み込む。
 * W, w, v (含全角) 以外は読み飛ばし、全角は半角に変換されたものが返される。
 *
 * \param in 読み込み元。
 *
 * \retval WEOF     ファイル終端またはエラー。
 * \retval non-WEOF 読み込んだ文字。 L'W', L'w', L'v' のいずれかになる。
 */
static wchar_t
grass_getwc(FILE *in)
{
	/* 標準入力から読み込む場合、何度も^Dが必要にならないように。 */
	if(ferror(in) || feof(in))
	{
		return WEOF;
	}

	for(;;)
	{
		wint_t ch = getwc(in);

		switch(ch)
		{
		case L'W':
		case L'w':
		case L'v':
		case WEOF:
			break;

		case L'Ｗ':
			ch = L'W';
			break;

		case L'ｗ':
			ch = L'w';
			break;

		case L'ｖ':
			ch = L'v';
			break;

		default:
			continue;
		}

		return ch;
	}
}


/*!
 * トークンをひとつ読み込む。
 *
 * \param in      読み込み元。 NULL は不可。
 * \param context コンテキスト。 NULL は不可。
 * \param token   読み込まれたトークンが格納される。
 *                戻り値が非ゼロの時は不定。 NULL は不可。
 *
 * \retval zero     読み込みエラー。
 * \retval non-zero 読み込み成功。
 */
static int
grass_read_token(FILE *in, struct grass_read_context *context, struct grass_token *token)
{
	assert(in != NULL);
	assert(context != NULL);
	assert(token != NULL);

	if(context->ungot_token.type != L'\0')
	{
		*token = context->ungot_token;
		context->ungot_token.type = L'\0';

		switch(token->type)
		{
		case L'v':
			break;

		case WEOF:
			break;

		default:
			break;
		}

		return 1;
	}

	token->type = grass_getwc(in);
	switch(token->type)
	{
	case L'W':
	case L'w':
		token->n = 1;
		break;

	case L'v':
		return 1;

	default:
		assert(0); /* BUG! */
	case WEOF:
		return !ferror(in);
	}

	/* W or w */
	for(;;)
	{
		wint_t ch = grass_getwc(in);
		if(ch != token->type)
		{
			if(ch != WEOF)
			{
				ungetwc(ch, in);
			}
			break;
		}

		token->n++;
	}

	return !ferror(in);
}


void
grass_unget_token(struct grass_read_context *context, const struct grass_token *token)
{
	assert(context != NULL);
	assert(token != NULL);

	/* 既に何か押し戻されていてはならない。 */
	assert(context->ungot_token.type == L'\0');

	context->ungot_token = *token;
}


/*!
 * 最初のwが出現するかファイル終端までソースを読み飛ばす。
 *
 * \param in 読み込み元
 *
 * \retval zero     読み飛ばし成功。
 * \retval non-zero エラー。
 */
static int
grass_skip_until_first_w(FILE *in)
{
	for(;;)
	{
		switch(grass_getwc(in))
		{
		case L'w':
			ungetwc(L'w', in);
			return 1;

		case WEOF:
			return !ferror(in);

		default:
			break;
		}
	}
}


static struct grass_instruction_node *
grass_parse_application(FILE *in, struct grass_read_context *context,
                        size_t function_index, char **error_message)
{
	struct grass_instruction_node *app;
	struct grass_token token;

	assert(in != NULL);
	assert(context != NULL);
	assert(error_message != NULL);

	if(!grass_read_token(in, context, &token))
	{
		*error_message = strerror(errno);
		return NULL;
	}

	if(token.type != 'w')
	{
		*error_message = "parse error: unexpected character.";
		return NULL;
	}

	app = grass_create_application_node(function_index, token.n);
	if(app == NULL)
	{
		*error_message = strerror(errno);
		return NULL;
	}

	return app;
}


static struct grass_instruction_node *
grass_parse_abstraction(FILE *in, struct grass_read_context *context,
                        size_t num_args, char **error_message)
{
	struct grass_instruction_node *abs;
	struct grass_instruction_node *body = NULL;
	struct grass_token token;
	int done = 0;

	assert(in != NULL);
	assert(context != NULL);
	assert(error_message != NULL);

	while(!done)
	{
		struct grass_instruction_node *app;

		if(!grass_read_token(in, context, &token))
		{
			*error_message = strerror(errno);
			return NULL;
		}

		switch(token.type)
		{
		case L'W':
			app = grass_parse_application(in, context, token.n, error_message);
			if(app == NULL)
			{
				return NULL;
			}
			body = grass_append_instruction_list(body, app);
			break;

		default:
		case L'w':
			assert(0); /* BUG! */
			*error_message = "parse error: internal error.";
			return NULL;

		case L'v':
		case WEOF:
			done = 1;
			break;
		}
	}

	abs = grass_create_abstraction_node(num_args, body);
	if(abs == NULL)
	{
		*error_message = strerror(errno);
		return NULL;
	}

	return abs;
}


/*!
 * ソースを読み込んで、 grass_instruction_node によるリスト (code) を作る。
 *
 * \param in            読み込み元。 NULL は不可。
 * \param error_message エラーが発生した場合、エラーを説明する文字列が格納される。
 *                      静的な文字列領域か、GC_MALLOCされた領域が格納されるので、
 *                      free()してはならない。
 *                      strerror() の戻り値が格納される場合もあるので、文字列
 *                      の内容を変更してはならない。(この仕様、変えたいな)
 *                      読み込み成功の場合は *error_message に NULL が格納される。
 *                      NULL は不可。
 *
 * \return 作成されたコード。エラーの場合は NULL 。
 *         ソースが空の場合もNULLが返されるが、 *error_message によって
 *         エラーか否かを区別できる。
 *         もっとも、空のソースはGrassとして不正なので、エラーと
 *         区別する必要がないかも知れない。
 *
 * \todo もう少し気の利いたエラーメッセージを返すべき。
 */
struct grass_instruction_node *
grass_parse_source(FILE *in, char **error_message)
{
	struct grass_read_context context = GRASS_READ_CONTEXT_INITIALIZER;
	struct grass_instruction_node *code = NULL;
	char *dummy_error_message;

	assert(in != NULL);

	if(error_message == NULL)
	{
		error_message = &dummy_error_message;
	}
	*error_message = NULL;

	if(!grass_skip_until_first_w(in))
	{
		*error_message = strerror(errno);
		return NULL;
	}

	for(;;)
	{
		struct grass_token token;
		struct grass_instruction_node *node;

		if(!grass_read_token(in, &context, &token))
		{
			*error_message = strerror(errno);
			return NULL;
		}

		switch(token.type)
		{
		case L'W':
			node = grass_parse_application(in, &context, token.n, error_message);
			break;

		case L'w':
			node = grass_parse_abstraction(in, &context, token.n, error_message);
			break;

		case WEOF:
			return code;

		case L'v':
			continue;

		default:
			assert(0); /* BUG! */
			*error_message = "parse error: internal error.";
			return NULL;
		}

		if(node == NULL)
		{
			return NULL;
		}

		code = grass_append_instruction_list(code, node);
	}

	assert(0);
	*error_message = "parse error: internal error.";
	return NULL;
}
