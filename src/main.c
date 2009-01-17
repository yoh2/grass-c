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


/*! プログラムのオプション */
struct prog_options
{
	int dump;    /*!< dumpオプションに対応。 */
	int trace;   /*!< traceオプションに対応。 */
	int step;    /*!< stopオプションに対応。 */
	int no_exec; /*!< noexecオプションに対応。 */

	const char *infile; /*!< 入力(ソース)ファイル。無指定ならNULL。 */

	int help;    /*!< helpオプションが指定されるか、不正オプションがあった場合に1。 */
	int help_to_stderr; /*!< ヘルプを stderr に出力するか。0の場合は stdout になる。 */
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
get_options(int argc, char *argv[], struct prog_options *options)
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
	options->dump = 0;
	options->trace = 0;
	options->step = 0;
	options->no_exec = 0;
	options->infile = NULL;
	options->help = 0;
	options->help_to_stderr = 0;

	do
	{
		switch(getopt_long(argc, argv, "dtsnh", longopts, NULL))
		{
		case 'd': /* dump */
			options->dump = 1;
			break;

		case 't': /* trace */
			options->trace = 1;
			break;

		case 's': /* step */
			options->step = 1;
			break;

		case 'n': /* noexec */
			options->no_exec = 1;
			break;

		case 'h': /* help */
			options->help = 1;
			break;

		case -1:
			done = 1;
			break;

		case '?':
			options->help = 1;
			options->help_to_stderr = 1;

			done = 1;
			break;

		default:
			assert(0); /* BUG! */
			break;
		}
	}while(!done);

	if(optind < argc - 1)
	{
		/* ファイル指定が複数あるのはNG。 */
		options->help = 1;
		options->help_to_stderr = 1;
	}
	else if(optind < argc)
	{
		options->infile = argv[optind];
	}
}


/*!
 * 使い方を表示する。
 *
 * \param out   使い方表示先。
 * \param prog  プログラム名。
 */
static void
print_usage(FILE *out, const char *prog)
{
	/* 頭回らん。gdgdな文かも。 */
	fprintf(out,
		"usage: %s [options..] [infile]\n"
		"\n"
		"  -d, --dump    dump parsed result.\n"
		"  -t, --trace   run with Grass machine's state output step-by-step.\n"
		"                (note: lots of texts will be output.)\n"
		"  -s, --step    run in stepping mode.\n"
		"  -n, --noexec  parse only. odn't run the program.\n"
		"  -h, --help    display this help and exit.\n"
		,
		prog
	);
}


/*!
 * \param options  実行オプション。
 * \param in       ソース読み込み元。
 *
 * \return そのまま main() の戻り値になる。
 */
static int
run(const struct prog_options *options, FILE *in)
{
	struct grass_instruction_node *code;
	char *error_messsage;

	code = grass_parse_source(in, &error_messsage);
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

	if(options->dump)
	{
		grass_dump_instruction_list(code);
		puts("");
	}

	if(!options->no_exec)
	{
		struct grass_machine *machine;
		char *msg;

		machine = grass_create_machine(code);

		while(!grass_machine_done(machine))
		{
			if(options->trace)
			{
				grass_dump_machine(machine);
			}
			if(options->step)
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


/*!
 * \retval 0 正常終了
 * \retval 1 何らかのエラー発生
 */
int main(int argc, char *argv[])
{
	struct prog_options options;
	FILE *in;

	setlocale(LC_ALL, "");

	get_options(argc, argv, &options);

	if(options.help)
	{
		print_usage(options.help_to_stderr? stderr: stdout, argv[0]);
		return (options.help_to_stderr? 1: 0);
	}

	if(options.infile == NULL)
	{
		in = stdin;
	}
	else
	{
		in = fopen(options.infile, "r");
		if(in == NULL)
		{
			perror(options.infile);
			return 1;
		}
	}

	return run(&options, in);
}
