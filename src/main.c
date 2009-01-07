/*
 * 注意点
 * 	- 入出力にはワイド文字を使う。
 * 	  ソース読み込み時の全角の扱いを簡単にするため、ワイド
 * 	  文字関数を利用。
 * 	  ソース読み込み元が stdin の場合を考えると、 In もワイド
 * 	  文字にしなければならない。 → ならば Out もワイド文字
 * 	  にすべき、ということで、プログラム全体がワイド文字を扱うことに。
 * 	- メモリ確保はBoehm GCを使う。
 */
#include "grass.h"
#include <stdio.h>
#include <wchar.h>
#include <locale.h>

void
dump_instruction_list(const struct grass_instruction_node *inst_list);

void
dump_value_list(const struct grass_value_node *value_list);

void
dump_instruction(const struct grass_instruction *inst)
{
	switch(inst->type)
	{
	case GRASS_IT_APPLICATION:
		wprintf(L"App(%d, %d)",
		        inst->content.app.func_index,
			inst->content.app.arg_index);
		break;

	case GRASS_IT_ABSTRACTION:
		wprintf(L"Abs(%d, ", inst->content.abs.num_args);
		dump_instruction_list(inst->content.abs.code);
		wprintf(L")");
	}
}


void
dump_instruction_list(const struct grass_instruction_node *inst_list)
{
	wprintf(L"(");
	while(inst_list != NULL)
	{
		dump_instruction(&inst_list->inst);
		wprintf(L" :: ");
		inst_list = inst_list->next;
	}
	wprintf(L"ε)");
}


void
dump_value(const struct grass_value *value)
{
	switch(value->type)
	{
	case GRASS_VT_CLOSURE:
		wprintf(L"{");
		dump_instruction_list(value->content.closure.code);
		wprintf(L", ");
		dump_value_list(value->content.closure.env);
		wprintf(L"}");
		break;

	case GRASS_VT_OUT:
		wprintf(L"Out");
		break;

	case GRASS_VT_IN:
		wprintf(L"In");
		break;

	case GRASS_VT_SUCC:
		wprintf(L"Succ");
		break;

	case GRASS_VT_NUMERIC:
		wprintf(L"Numeric[%d]", value->content.numeric.n);
		break;
	}
}


void
dump_value_list(const struct grass_value_node *value_list)
{
	wprintf(L"(");
	while(value_list != NULL)
	{
		dump_value(&value_list->value);
		wprintf(L" :: ");
		value_list = value_list->next;
	}
	wprintf(L"ε)");
}


void
dump_machine(const struct grass_machine *machine)
{
	wprintf(L"code: ");
	dump_instruction_list(machine->code);
	wprintf(L"\n");

	wprintf(L"env : ");
	dump_value_list(machine->env);
	wprintf(L"\n");

	wprintf(L"dump: ");
	dump_value_list(machine->dump);
	wprintf(L"\n");

	wprintf(L"\n");
}

/* とりあえず */
int main(int argc, char *argv[])
{
	struct grass_instruction_node *code;
	char *error_messsage;

	setlocale(LC_ALL, "");

	code = grass_parse_source(stdin, &error_messsage);
	if(code == NULL)
	{
		if(error_messsage == NULL)
		{
			wprintf(L"empty source.\n");
		}
		else
		{
			wprintf(L"%s\n", error_messsage);
		}
	}
	else
	{
		struct grass_machine *machine;
		char *msg;

		machine = grass_create_machine(code);

		/*
		wprintf(L"code = %p\n", code);
		wprintf(L"machine = %p\n\n", machine);
		*/

		while(!grass_machine_done(machine))
		{
			/*
			dump_machine(machine);
			wprintf(L"hit enter key.");
			getwchar();
			*/
			if(!grass_step_machine(machine, &msg))
			{
				wprintf(L"%s\n", msg);
				break;
			}
		}
	}
	return 0;
}
