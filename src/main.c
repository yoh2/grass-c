/*! $Id$ */
/*! \file
 * \brief grassプログラムエントリーポイント。
 *
 * \date 2009-01-06
 * \author yoh2
 *
 * $LastChangedDate$
 * $LastChangedBy$
 */
/*
 * 注意点
 * 	- Grassソースの入力には、現在のロケールに合わせたものを入力すること。
 * 	- メモリ確保はBoehm GCを使う。
 */
#include "grass.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <assert.h>

struct prog_options
{
	int dump;
	int trace;
	int step;
	int no_exec;

	int help;
	int help_to_stderr;
};

/*
 * コマンドライン:
 *
 *	--dump,   -d 実行前にソース解析結果をダンプする。
 *	--trace,  -t Grassマシンの状態をダンプしつつ実行する。ど大量の
 *	             結果が表示されるので注意。
 *	--step,   -s ステップ実行を行う。
 *	--noexec, -n ソースを読み込むだけで、実行を行わない。
 *	--help,   -h 使い方を出力して終了する。
 */
static void
get_options(int argc, char *argv[], struct prog_options *opts)
{
	struct option longopts[] = {
		{ "dump",   no_argument, NULL, 'd' },
		{ "trace",  no_argument, NULL, 't' },
		{ "step",   no_argument, NULL, 's' },
		{ "noexec", no_argument, NULL, 'n' },
		{ "help",   no_argument, NULL, 'h' },

		{ 0 }
	};
	int done = 0;

	/* 初期値 */
	opts->dump = 0;
	opts->trace = 0;
	opts->step = 0;
	opts->no_exec = 0;
	opts->help = 0;
	opts->help_to_stderr = 0;

	do
	{
		switch(getopt_long(argc, argv, "dtsnh", longopts, NULL))
		{
		case 'd': /* dump */
			opts->dump = 1;
			break;

		case 't': /* trace */
			opts->trace = 1;
			break;

		case 's': /* step */
			opts->step = 1;
			break;

		case 'n': /* noexec */
			opts->no_exec = 1;
			break;

		case 'h': /* help */
			opts->help = 1;
			break;

		case -1:
			done = 1;
			break;

		case '?':
			opts->help = 1;
			opts->help_to_stderr = 1;
			done = 1;
			break;

		default:
			assert(0); /* BUG! */
			break;
		}
	}while(!done);
}


/*
 * \retval 0 正常終了
 */
int main(int argc, char *argv[])
{
	struct grass_instruction_node *code;
	char *error_messsage;
	struct prog_options options;

	setlocale(LC_ALL, "");

	get_options(argc, argv, &options);

	if(options.help)
	{
		/* TODO: ヘルプ表示 */
		return (options.help_to_stderr? 1: 0);
	}


	code = grass_parse_source(stdin, &error_messsage);
	if(code == NULL)
	{
		if(error_messsage == NULL)
		{
			printf("empty source.\n");
		}
		else
		{
			printf("%s\n", error_messsage);
		}

		return 1;
	}

	if(options.dump)
	{
		grass_dump_instruction_list(code);
		puts("");
	}

	if(!options.no_exec)
	{
		struct grass_machine *machine;
		char *msg;

		machine = grass_create_machine(code);

		while(!grass_machine_done(machine))
		{
			if(options.trace)
			{
				grass_dump_machine(machine);
			}
			if(options.step)
			{
				printf("hit enter key.");
				fflush(stdout);
				getchar();
			}
			if(!grass_step_machine(machine, &msg))
			{
				printf("%s\n", msg);
				return 1;
			}
		}
	}
	return 0;
}
