#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

Token *tokens;	// single linked list of tokens
Token *lastTk;		// the last token in list

int line=1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code)
{
    Token *tk=safeAlloc(sizeof(Token));
    tk->code=code;
    tk->line=line;
    tk->next=NULL;
    if(lastTk)
    {
        lastTk->next=tk;
    }
    else
    {
        tokens=tk;
    }
    lastTk=tk;
    return tk;
}


//primeste pointerii begin si end si extrage intr-o zona alocata
// dinamic subsirul dintre cei doi pointeri [begin, end]
char *extractString(const char *begin,const char *end)
{
    if (begin == NULL || end == NULL || begin >= end)
    {
        err("invalid pointers");
        return NULL;
    }

    size_t length = end - begin;

    // alocam memorie dinamic pentru sirul extras
    char *extracted_text = (char*)safeAlloc((size_t)length+1);

    // copiem carcterele din intervalul [begin, end)
    size_t i = 0;
    for(i = 0; i < length; i++)
    {
        extracted_text[i] = begin[i];
    }

    // adaugam terminatorul de sir de caracter
    extracted_text[length] = '\0';

    return extracted_text;
}

// de completat la
Token *tokenize(const char *pch)
{
    const char *start;
    Token *tk;
    for(;;)
    {
        switch(*pch)
        {
        case ' ':
        case '\t':
            pch++;
            break;
        case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
            if(pch[1]=='\n')pch++;
        // fallthrough to \n
        case '\n':
            line++;
            pch++;
            break;
        case '\0':
            addTk(END);
            return tokens;
        case ',':
            addTk(COMMA);
            pch++;
            break;
        case ';':
            addTk(SEMICOLON);
            pch++;
            break;
        case '(':
            addTk(LPAR);
            pch++;
            break;
        case ')':
            addTk(RPAR);
            pch++;
            break;
        case '[':
            addTk(LBRACKET);
            pch++;
            break;
        case ']':
            addTk(RBRACKET);
            pch++;
            break;
        case '{':
            addTk(LACC);
            pch++;
            break;
        case '}':
            addTk(RACC);
            pch++;
            break;
        case '+':
            addTk(ADD);
            pch++;
            break;
        case '-':
            addTk(SUB);
            pch++;
            break;
        case '*':
            addTk(MUL);
            pch++;
            break;
        case '/':
            if(pch[1]=='/')
            {
                while(*(pch)!='\n')
                {
                    //printf("%c", *pch);
                    pch++;
                }
                //printf("\n");

            }
            else
            {
                addTk(DIV);
                pch++;
            }

            break;
        case '.':
            if(isdigit(pch[1])==0)
            {
                addTk(DOT);
                pch++;
            }
            break;
        case '&':
            if(pch[1] == '&')
            {
                addTk(AND);
                pch+=2;
            }
            else
            {
                err("invalid symbol: %c (%d)",*pch,*pch);
            }
            break;
        case '|':
            if(pch[1] == '|')
            {
                addTk(OR);
                pch+=2;
            }
            else
            {
                err("invalid symbol: %c (%d)",*pch,*pch);
            }
            break;
        case '!':
            if(pch[1] == '=')
            {
                addTk(NOTEQ);
                pch+=2;
            }
            else
            {
                addTk(NOT);
                pch++;
            }
            break;
        case '=':
            if(pch[1] == '=')
            {
                addTk(EQUAL);
                pch+=2;
            }
            else
            {
                addTk(ASSIGN);
                pch++;
            }
            break;
        case '<':
            if(pch[1] == '=')
            {
                addTk(LESSEQ);
                pch+=2;
            }
            else
            {
                addTk(LESS);
                pch++;
            }
            break;
        case '>':
            if(pch[1] == '=')
            {
                addTk(GREATEREQ);
                pch+=2;
            }
            else
            {
                addTk(GREATER);
                pch++;
            }
            break;
        case '\'':
            if(pch[1]!='\'' && pch[2]=='\'')
            {
                tk=addTk(CHAR);
                tk->c=pch[1];
                pch+=3;
            }
            break;

        default:
            if(isalpha(*pch)||*pch=='_')
            {
                for(start=pch++; isalnum(*pch) || *pch=='_'; pch++) {}
                char *text=extractString(start, pch);
                if(strcmp(text,"char")==0)addTk(TYPE_CHAR);
                else if(strcmp(text,"double")==0)addTk(TYPE_DOUBLE);
                else if(strcmp(text,"else")==0)addTk(ELSE);
                else if(strcmp(text,"if")==0)addTk(IF);
                else if(strcmp(text,"int")==0)addTk(TYPE_INT);
                else if(strcmp(text,"return")==0)addTk(RETURN);
                else if(strcmp(text,"struct")==0)addTk(STRUCT);
                else if(strcmp(text,"void")==0)addTk(VOID);
                else if(strcmp(text,"while")==0)addTk(WHILE);
                else
                {
                    tk=addTk(ID);
                    tk->text=text;
                }
            }
            else if(isdigit(*pch))
            {

                for(start=pch++; isdigit(*pch) || *pch=='.' || *pch=='E' || *pch=='e' ||
                   ((*pch=='e' || *pch=='E') && (*(pch+1)=='-' || (*(pch+1)=='+' || isdigit(*(pch+1)) ) )) ||
                   ((*(pch-1)=='e' || *(pch-1)=='E') && (*pch=='-' || (*pch=='+' || isdigit(*pch) ) )); pch++) {
                       if(*pch=='.' && !(isdigit(*(pch+1)))) {
                            err("dupa . nu e cifra");
                       }
                }
                char*text = extractString(start, pch);

                double val = atof(text); // converteste textul extras in double
                // daca are . si se termina cu cifra atunci este double
                if(strchr(text, '.') || strchr(text, 'E') || strchr(text, 'e') || strchr(text, '-'))
                {
                    tk=addTk(DOUBLE);
                    tk->d=val;
                    tk->text=text;
                }
                else
                {
                    tk=addTk(INT);
                    tk->i=(int)val; // face cast la int in cazul in care nu avem caracterul .
                }
            }
            else if(*pch =='\"')
            {
                for(start=pch++; *pch!='\"'; pch++) {
                    if(*pch=='\0' || *pch=='\r' || *pch=='\n')err("invalid character inside string");
                }
                char *text=extractString(start, pch);
                text[strlen(text)]='"';
                text[strlen(text)+1]='\0';
                pch++;

                // elimin caracterul " din string
                char *s, *d;
                for(s = d = text; *s!='\0'; s++)
                {
                    *d = *s;
                    if(*d != '"') d++;
                }
                *d='\0';

                tk=addTk(STRING);
                tk->text=text;
            }
            else err("invalid number: %c (%d)",*pch,*pch);
        }
    }
}


void showTokens(const Token *tokens)
{
    for(const Token *tk=tokens; tk; tk=tk->next)
    {
        switch(tk->code)
        {
        //identifiers
        case ID:
            printf("%s", tk->text);
            break;
        //keywords
        case TYPE_CHAR:
            printf("%s","char");
            break;
        case TYPE_DOUBLE:
            printf("%s","double");
            break;
        case IF:
            printf("%s","if");
            break;
        case ELSE:
            printf("%s","else");
            break;
        case TYPE_INT:
            printf("%s","int");
            break;
        case RETURN:
            printf("%s","return");
            break;
        case STRUCT:
            printf("%s","struct");
            break;
        case VOID:
            printf("%s","void");
            break;
        case WHILE:
            printf("%s","while");
            break;
        //constants
        case INT:
            printf("%d", tk->i);
            break;
        case DOUBLE:
            printf("%0.3f", tk->d);
            break;
        case CHAR:
            printf("'%c'",tk->c);
            break;
        case STRING:
            printf("%s",tk->text);
            break;
        //delimiters
        case COMMA:
            printf("%s",",");
            break;
        case SEMICOLON:
            printf("%s",";");
            break;
        case LPAR:
            printf("%s","(");
            break;
        case RPAR:
            printf("%s",")");
            break;
        case LBRACKET:
            printf("%s","[");
            break;
        case RBRACKET:
            printf("%s","]");
            break;
        case LACC:
            printf("%s","{");
            break;
        case RACC:
            printf("%s","}");
            break;
        //operators
        case ADD:
            printf("%s","+");
            break;
        case SUB:
            printf("%s","-");
            break;
        case MUL:
            printf("%s","*");
            break;
        case DIV:
            printf("%s","/");
            break;
        case DOT:
            printf("%s",".");
            break;
        case AND:
            printf("%s","&&");
            break;
        case OR:
            printf("%s","||");
            break;
        case NOT:
            printf("%s","!");
            break;
        case ASSIGN:
            printf("%s","=");
            break;
        case EQUAL:
            printf("%s","==");
            break;
        case NOTEQ:
            printf("%s","!=");
            break;
        case LESS:
            printf("%s","<");
            break;
        case LESSEQ:
            printf("%s","<=");
            break;
        case GREATER:
            printf("%s",">");
            break;
        case GREATEREQ:
            printf("%s",">=");
            break;
        case END:
            printf("\nEND\n");
            break;
        }
    }
}

void showTokensByCode(const Token *tokens)
{
    for(const Token *tk=tokens; tk; tk=tk->next)
    {
        printf("%d  ",tk->code);
        switch(tk->code)
        {
        //idenitifiers
        case ID:
            printf("%s %s\n","ID", tk->text);
            break;
        //keywords
        case TYPE_CHAR:
            printf("%s\n","TYPE_CHAR");
            break;
        case TYPE_DOUBLE:
            printf("%s\n","TYPE_DOUBLE");
            break;
        case IF:
            printf("%s\n","IF");
            break;
        case ELSE:
            printf("%s\n","ELSE");
            break;
        case TYPE_INT:
            printf("%s\n","TYPE_INT");
            break;
        case RETURN:
            printf("%s\n","RETURN");
            break;
        case STRUCT:
            printf("%s\n","STRUCT");
            break;
        case VOID:
            printf("%s\n","VOID");
            break;
        case WHILE:
            printf("%s\n","WHILE");
            break;
        //constants
        case INT:
            printf("%s %d\n","INT", tk->i);
            break;
        case DOUBLE:
            printf("%s %0.4f\n","DOUBLE", tk->d);
            break;
        case CHAR:
            printf("%s %c\n","CHAR", tk->c);
            break;
        case STRING:
            printf("%s %s\n", "STRING", tk->text);
            break;
        //delimiters
        case COMMA:
            printf("%s\n","COMMA");
            break;
        case SEMICOLON:
            printf("%s\n","SEMICOLON");
            break;
        case LPAR:
            printf("%s\n","LPAR (");
            break;
        case RPAR:
            printf("%s\n","RPAR )");
            break;
        case LBRACKET:
            printf("%s\n","LBRACKET [");
            break;
        case RBRACKET:
            printf("%s\n","RBRACKET ]");
            break;
        case LACC:
            printf("%s\n","LACC {");
            break;
        case RACC:
            printf("%s\n","RACC }");
            break;
        //operators
        case ADD:
            printf("%s\n","ADD +");
            break;
        case SUB:
            printf("%s\n","SUB -");
            break;
        case MUL:
            printf("%s\n","MUL *");
            break;
        case DIV:
            printf("%s\n","DIV /");
            break;
        case DOT:
            printf("%s\n","DOT .");
            break;
        case AND:
            printf("%s\n","AND &&");
            break;
        case OR:
            printf("%s\n","OR ||");
            break;
        case NOT:
            printf("%s\n","NOT !");
            break;
        case ASSIGN:
            printf("%s\n","ASSIGN =");
            break;
        case EQUAL:
            printf("%s\n","EQUAL ==");
            break;
        case NOTEQ:
            printf("%s\n","NOTEQ !=");
            break;
        case LESS:
            printf("%s\n","LESS <");
            break;
        case LESSEQ:
            printf("%s\n","LESSEQ <=");
            break;
        case GREATER:
            printf("%s\n","GREATER >");
            break;
        case GREATEREQ:
            printf("%s\n","GREATEREQ >=");
            break;

        case END:
            printf("%s\n", "END");
            break;

        }
    }
}


