%{
#include <stdio.h>
#include <math.h>
#include "y.tab.h"
%}

%option yylineno
letter	[a-zA-Z]
alphanum [a-zA-Z0-9]
num [0-9]

%%
(\+)|(\*)|(\\)|(-) {printf("%s", yytext); 
			 return ARITH;}
"if" {printf("%s", yytext); 
			 return IF;}
"else" {printf("%s", yytext); 
			 return ELSE;}
"while" {printf("%s", yytext); 
			 return WHILE;}
"return" {printf("%s", yytext); 
			 return RETURN;}
"int" {printf("%s", yytext); 
			 return INT;}
"void" {printf("%s", yytext); 
			 return VOID;}
"extern" {printf("%s", yytext); 
			 return EXTERN;}
"print" {printf("%s", yytext); 
			return PRINT;}
"read" {printf("%s", yytext); 
			 return READ;}
"func" {printf("%s", yytext); 
			 return FUNC;}
{letter}{alphanum}* {printf("%s", yytext); 
						return WORD;}
{num}+ {printf("%s", yytext); 
			 return NUM;}
>|<|==|<=|>= {printf("%s", yytext); return OP;}//gets operators
"{" {printf("%s", yytext); 
		return yytext[0];}
"}" {printf("%s", yytext); 
		return yytext[0];}
"("	{printf("%s", yytext); 
		return yytext[0];}
")"	{printf("%s", yytext); 
       return yytext[0];}
";" {printf("%s", yytext); 
       return yytext[0];}
"=" {printf("%s", yytext);
	   return yytext[0];}
.|[ \t\n] {printf("%s", yytext);} //ignores whitespace
%%

int yywrap(){
	return(1);
}

/*int main(){
	yylex();
	return(0);
}*/
