#include "pse/module.h"
#include "parser.tab.h"
#include "scanner.h"

module *
new_module_from_stdin()
{
	module *mod = (module *)malloc(sizeof(module));
	mod->src = stdin;
	return mod;
}

module *
new_module_from_file(const char *filename)
{
	module *mod = (module *)malloc(sizeof(module));
	mod->src = fopen(filename, "r");
	return mod;
}

module *
new_module_from_string(char *src)
{
	module *mod = (module *)malloc(sizeof(module));
	mod->src = fmemopen(src, strlen(src) + 1, "r");
	return mod;
}

void delete_module(module *mod)
{
	if (mod->root != NULL)
	{
		delete_sexp_node(mod->root);
	}
	fclose(mod->src);
	free(mod);
}

int parse_module(module *mod)
{
	yyscan_t sc;
	int res;

	module_yylex_init(&sc);
	module_yyset_in(mod->src, sc);
	res = module_yyparse(sc, mod);
	module_yylex_destroy(sc);

	if (res == 0)
	{
		print_node_sexp(mod->root);
	}

	return res;
}

module *
raw_parser(char *src)
{
	module *extra;
	core_yyscan_t scanner;

	scanner = module_scanner_create(src);

	extra = (module *)malloc(sizeof(module));
	extra->src = fmemopen(src, strlen(src) + 1, "r");

	extra->yyresult = module_yyparse(scanner, extra);

	module_scanner_destroy(scanner);
	return extra;
}
