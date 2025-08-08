%{
#include "parser.tab.h"
%}

%option reentrant bison-bridge noyywrap
%option prefix="module_yy"
%option extra-type="module_yy_extra"


id     [a-zA-Z][a-zA-Z_0-9]*
int    [0-9]+
blank  [ \t]
string \"[^\"\n]*\"


%{
#define update_location() \
    do \
    { \
        yyextra.first_loc = yytext - yyextra.scan_buf; \
        yyextra.last_loc = yyextra.first_loc + strlen(yytext) - 1; \
    } while (0)
#define get_location() (yyextra.first_loc)
#define Return update_location(); return

#define scan_errmsg(msg) _scan_errmsg(msg, &yyextra)
static int _scan_errmsg(const char *msg, const module_yy_extra *extra);
#define scan_errposition() _scan_errposition(yyextra.last_loc, &yyextra)
static int _scan_errposition(const int location, const module_yy_extra *extra);
extern int module_scanner_errmsg(const char *msg, core_yyscan_t *scanner);
extern int module_scanner_errposition(const int location, core_yyscan_t *scanner);
%}

%%

{blank}+   ;
[\n]+      ;

"("        { Return TOK_LPAREN; }
")"        { Return TOK_RPAREN; }

{int}      { yylval->NUMBER = atol(yytext); Return NUMBER; }
{id}       { yylval->IDENTIFIER = yytext; Return IDENTIFIER; }
{string}   { yylval->STRING = yytext; Return STRING; }

<<EOF>>    return TOK_EOF;

%%

static int 
_scan_errmsg(const char *msg, const module_yy_extra *extra)
{
    int ret, loop;
    const char *t = extra->scan_buf + extra->first_loc;
    char * p = (char *)t;

    while(p > extra->scan_buf)
    {
        if (*p == '\n') 
            break;
        p--;
    }
    
    loop = ((p != extra->scan_buf) ? t - ++p : t - p);

    if (t[0] == YY_END_OF_BUFFER_CHAR)
    {
        ret = fprintf(stderr, "\n%s at end of input\n", msg);
    }
    else
    {
        ret = fprintf(stderr, "\n\033[31merror: \033[0mlocation of %s was here : \n\t%s\n"
                              "\t", msg, p);

        for (int i = 0; i < loop; i++, fprintf(stderr, " ")); 

        for (int i = 0; i <= extra->last_loc - extra->first_loc; 
            i++, fprintf(stderr, "\033[31m↑"))
            ;
        fprintf(stderr, "\033[31m - Error Here!\n\033[0m"); 
    }
    return ret;
}

static int 
_scan_errposition(const int location, const module_yy_extra *extra)
{
    int pos;

    // no-op if location is unknown
    if (location < 0)
        return 0;

    // convert byte offset to number of characters
    pos = mblen(extra->scan_buf, location) + 1;

    return pos;
}

core_yyscan_t 
module_scanner_create(const char *s)
{
    unsigned long int len;
    char *buf;
    yyscan_t yyscanner;
    module_yy_extra extra;
    int ret;

    // The last two YY_END_OF_BUFFER_CHAR are required by flex.
    if (s)
        len = strlen(s);
    else
        len = 0;
    buf = malloc((len + 2) * sizeof(char));
    strncpy(buf, s, len);
    buf[len] = YY_END_OF_BUFFER_CHAR;
    buf[len + 1] = YY_END_OF_BUFFER_CHAR;

    ret = module_yylex_init(&yyscanner);
    if (ret)
        fprintf(stderr, "module_yylex_init() failed: %m");

    extra.high_surrogate = 0;
    extra.start_cond = INITIAL;
    extra.scan_buf = buf;
    extra.last_loc = 0;
    extra.first_loc = 0;

    module_yyset_extra(extra, yyscanner);
    module_yy_scan_buffer(buf, len + 2, yyscanner);
    return yyscanner;
}

void 
module_scanner_destroy(core_yyscan_t scanner)
{
    module_yylex_destroy(scanner);
}

int 
module_scanner_errmsg(const char *msg, core_yyscan_t *scanner)
{
    module_yy_extra extra;

    extra = module_yyget_extra(scanner);

    return _scan_errmsg(msg, &extra);
}

int 
module_scanner_errposition(const int location, core_yyscan_t *scanner)
{
    module_yy_extra extra;

    extra = module_yyget_extra(scanner);

    return _scan_errposition(location, &extra);
}
