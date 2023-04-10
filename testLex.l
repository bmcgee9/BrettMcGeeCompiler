%{
#include <stdio.h>
#include <math.h>
%}

letter	[a-zA-Z]
alphanum [a-zA-Z0-9]
num [0-9]

%%
(\+)|(\*)|(\\)|(-) {printf("%s", yytext); }
"if" {printf("%s", yytext); }
"else" {printf("%s", yytext); }
"while" {printf("%s", yytext); }
"return" {printf("%s", yytext);}
"int" {printf("%s", yytext);}
"void" {printf("%s", yytext);}
"extern" {printf("%s", yytext);}
"print" {printf("%s", yytext);}
"read" {printf("%s", yytext);}
{letter}{alphanum}* {printf("%s", yytext);}
{num}+ {printf("%s", yytext);}
>|<|==|<=|>= {printf("%s", yytext);}
"{" {printf("%s", yytext); }
"}" {printf("%s", yytext); }
"("	{printf("%s", yytext);}
")"	{printf("%s", yytext);}
";" {printf("%s", yytext);}
.|[ \t\n]
%%

int yywrap(){
	return(1);
}

int main(){
	yylex();
	return(0);
}
