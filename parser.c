#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "ad.h"
#include "utils.h"

Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token
Symbol *owner;

void tkerr(const char *fmt,...)
{
    fprintf(stderr,"error in line %d: ",iTk->line);
    va_list va;
    va_start(va,fmt);
    vfprintf(stderr,fmt,va);
    va_end(va);
    fprintf(stderr,"\n");
    exit(EXIT_FAILURE);
}

char *tkCodeName(int code) {
    switch(code) {
        case ID:
            return "ID";
        case TYPE_INT:
            return "TYPE_INT";
        case TYPE_CHAR:
            return "TYPE_CHAR";
        case TYPE_DOUBLE:
            return "TYPE_DOUBLE";
        case ELSE:
            return "ELSE";
        case IF:
            return "IF";
        case RETURN:
            return "RETURN";
        case STRUCT:
            return "STRUCT";
        case VOID:
            return "VOID";
        case WHILE:
            return "WHILE";
        case COMMA:
            return "COMMA";
        case SEMICOLON:
            return "SEMICOLON";
        case LPAR:
            return "LPAR";
        case RPAR:
            return "RPAR";
        case LBRACKET:
            return "LBRACKET";
        case RBRACKET:
            return "RBRACKET";
        case LACC:
            return "LACC";
        case RACC:
            return "RACC";
        case END:
            return "END";
        case ADD:
            return "ADD";
        case MUL:
            return "MUL";
        case DIV:
            return "DIV";
        case DOT:
            return "DOT";
        case AND:
            return "AND";
        case OR:
            return "OR";
        case NOT:
            return "NOT";
        case NOTEQ:
            return "NOTEQ";
        case LESS:
            return "LESS";
        case LESSEQ:
            return "LESSEQ";
        case GREATER:
            return "GREATER";
        case GREATEREQ:
            return "GREATEREQ";
        case ASSIGN:
            return "ASSIGN";
        case EQUAL:
            return "EQUAL";
        case SUB:
            return "SUB";
        case INT:
            return "INT";
        case DOUBLE:
            return "DOUBLE";
        case CHAR:
            return "CHAR";
        case STRING:
            return "STRING";
        default:
            return "N\\A";
    }
}


bool consume(int code)
{
    printf("consume(%s)",tkCodeName(code));
    if(iTk->code==code)
    {
        consumedTk=iTk;
        iTk=iTk->next;
        printf(" => consumed\n");
        return true;
    }
    printf(" => found %s\n",tkCodeName(iTk->code));
    return false;
}

//structDef: STRUCT ID LACC varDef* RACC SEMICOLON
// 2
bool structDef(){
    Token *start = iTk;
    if(consume(STRUCT)) {
        if(consume(ID)) {
            Token *tkName=consumedTk;
            if(consume(LACC)) {
                Symbol *s=findSymbolInDomain(symTable,tkName->text);
                if(s)tkerr("symbol redefinition: %s",tkName->text);
                s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
                s->type.tb=TB_STRUCT;
                s->type.s=s;
                s->type.n=-1;
                pushDomain();
                owner=s;
                for(;;) {
                    if(varDef()) {
                    }
                    else break;
                }
                if(consume(RACC)) {
                    if(consume(SEMICOLON)) {
                        owner=NULL;
                        dropDomain();
                        return true;
                    }else tkerr("Lipseste ; dupa definirea structurii");
                }else tkerr("Lipseste } din definirea structurii");
            }
        }
    }

    iTk = start;
    return false;
}

//varDef: typeBase ID arrayDecl? SEMICOLON
// 3
bool varDef()
{
    Token *start = iTk;
    Type t;
    if(typeBase(&t)) {
        if(consume(ID)) {
            Token *tkName=consumedTk;
            if(arrayDecl(&t)) {
                if(t.n==0)tkerr("a vector variable must have a specified dimension");
            }
            if(consume(SEMICOLON)) {
                Symbol *var=findSymbolInDomain(symTable,tkName->text);
                if(var)tkerr("symbol redefinition: %s",tkName->text);
                var=newSymbol(tkName->text,SK_VAR);
                var->type=t;
                var->owner=owner;
                addSymbolToDomain(symTable,var);
                if(owner){
                    switch(owner->kind){
                        case SK_FN:
                        var->varIdx=symbolsLen(owner->fn.locals);
                        addSymbolToList(&owner->fn.locals,dupSymbol(var));
                        break;
                        case SK_STRUCT:
                        var->varIdx=typeSize(&owner->type);
                        addSymbolToList(&owner->structMembers,dupSymbol(var));
                        break;
                    }
                }else{
                    var->varMem=safeAlloc(typeSize(&t));
                }

                return true;
            } // ar trebui sa fac cumva sa imi dea eroarea daca e variabila si nu am ;
              // dar sa nu imi apara eroarea si la fnDef()
        } else tkerr("Lipseste numele variabilei");
    }

    iTk = start;
    return false;
}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
// 4
bool typeBase(Type *t)
{
    t->n=-1;
    Token *start = iTk;
    if(consume(TYPE_INT))
    {
        t->tb=TB_INT;
        return true;
    }
    if(consume(TYPE_DOUBLE))
    {
        t->tb=TB_DOUBLE;
        return true;
    }
    if(consume(TYPE_CHAR))
    {
        t->tb=TB_CHAR;
        return true;
    }
    if(consume(STRUCT))
    {

        if(consume(ID))
        {
            Token *tkName = consumedTk;
            t->tb=TB_STRUCT;
            t->s=findSymbol(tkName->text);
            if(!t->s)tkerr("structura nedefinita: %s",tkName->text);
            return true;
        }
        else {
                tkerr("lipseste numele structurii!");
        }
    }
    iTk = start;
    return false;
}

//arrayDecl: LBRACKET INT? RBRACKET
// 5
bool arrayDecl(Type *t){
    Token *start = iTk;
    if(consume(LBRACKET)){
        if(consume(INT)){
            Token *tkSize=consumedTk;
            t->n=tkSize->i;
        }else{
            t->n=0; // array fara dimensiune: int v[]
        }
        if(consume(RBRACKET)){
            return true;
        }else tkerr("missing ] or invalid expression inside [...]");
    }
    iTk = start;
    return false;
}


//fnDef: ( typeBase | VOID ) ID
//          LPAR ( fnParam ( COMMA fnParam )* )? RPAR
//          stmCompound
// 6
bool fnDef()
{
    Token *start = iTk;
    Type t;
    if (consume(VOID)) {
        t.tb=TB_VOID;
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (consume(LPAR)) {
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr("symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if (fnParam()) {
                    for (;;) {
                        if (consume(COMMA)) {
                            if (fnParam()) {
                            } else {
                                tkerr("Lipseste parametrul dupa , in definirea functiei");
                                break;
                            }
                        } else break;
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(false)) {
                        dropDomain();
                        owner=NULL;
                        return true;
                    }
                }
            } else tkerr("Lipseste ( in definirea functiei");
        } else tkerr("Lipseste numele functiei");
    }
    else if (typeBase(&t)) {
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (consume(LPAR)) {
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr("symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();
                if (fnParam()) {
                    for (;;) {
                        if (consume(COMMA)) {
                            if (fnParam()) {
                            } else {
                                tkerr("Lipseste parametrul dupa , in definirea functiei");
                                break;
                            }
                        } else break;
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(false)) {
                        dropDomain();
                        owner=NULL;
                        return true;
                    }
                }
            }
        } else tkerr("Lipseste numele functiei");
    }
    iTk = start;
    return false;
}


// fnParam: typeBase ID arrayDecl?
// 7
bool fnParam()
{
    Token *start = iTk;
    Type t;
    if(typeBase(&t)) {
        if(consume(ID)) {
            Token *tkName = consumedTk;
            if(arrayDecl(&t)) {
                t.n=0;
            }
            Symbol *param=findSymbolInDomain(symTable,tkName->text);
            if(param)tkerr("symbol redefinition: %s",tkName->text);
            param=newSymbol(tkName->text,SK_PARAM);
            param->type=t;
            param->owner=owner;
            param->paramIdx=symbolsLen(owner->fn.params);
            // parametrul este adaugat atat la domeniul curent, cat si la parametrii fn
            addSymbolToDomain(symTable,param);
            addSymbolToList(&owner->fn.params,dupSymbol(param));
            return true;
        } tkerr("Lipseste numele parametrului functiei");
    }
    iTk = start;
    return false;
}

//stm: stmCompound
//| IF LPAR expr RPAR stm ( ELSE stm )?
//| WHILE LPAR expr RPAR stm
//| RETURN expr? SEMICOLON
//| expr? SEMICOLON
// 8
bool stm() {
    Token *start = iTk;
    if(stmCompound(true)){
        return true;
    }
    // | IF LPAR expr RPAR stm ( ELSE stm )?
    if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr()) {
                if(consume(RPAR)) {
                    if(stm()){
                        if(consume(ELSE)) {
                            if(stm()){
                                return true;
                            } else tkerr("Lipseste statement dupa conditia else");
                        }
                        return true;
                    } else tkerr("Lipseste statement dupa conditia if");
                } else tkerr("Lipseste ) in if");
            } else tkerr("Lipseste expresia in if");
        } else tkerr("Lipseste ( in if");
    }
    // | WHILE LPAR expr RPAR stm
    if(consume(WHILE)) {
        if (consume(LPAR)) {
            if(expr()){
                if(consume(RPAR)) {
                    if(stm()) {
                        return true;
                    } else tkerr("Lipseste statement in while");
                }else tkerr("Lipseste ) in while");
            } else tkerr("Lipseste expresia in while");
        } else tkerr("Lipseste ( in while");
    }
    // | RETURN expr? SEMICOLON
    if(consume(RETURN)) {
        if(expr()){
            if(consume(SEMICOLON)){
                return true;
            } else tkerr("Lipseste ; in return");
        }
        if(consume(SEMICOLON)) return true;
    }
    // | expr? SEMICOLON
    if(expr()) {
        if(consume(SEMICOLON)){
            return true;
        } else tkerr("Lipseste ; in expresie");
    }
    else if(consume(SEMICOLON)) return true;

    iTk = start;
    return false;
}

// stmCompound: LACC ( varDef | stm )* RACC
// 9
bool stmCompound(bool newDomain){

	Token *start = iTk;
	if(consume(LACC)) {
        if(newDomain)pushDomain();
        for(;;) {
            if(varDef()) {}
            else if(stm()) {}
            else break;
        }
        if(consume(RACC))
            {if(newDomain)dropDomain();
            return true;
            } else tkerr("missing } in compound statement");
    }
	iTk = start;
	return false;
}


//expr = exprAssign
// 10
bool expr() {
    if(exprAssign()) {
        return true;
    }
    return false;
}

//exprAssign = exprUnary ASSIGN exprAssign | exprOr
// 11
bool exprAssign() {
    Token *start = iTk;
    if(exprUnary()) {
        if(consume(ASSIGN)) {
            if(exprAssign()){
                return true;
            } else tkerr("Lipseste expresia dupa =");
        }
    }
    iTk = start; // daca prima expresie din SAU da fail
    if(exprOr()) {
        return true;
    }
    iTk = start;
    return false;
}

//exprOr = exprOr OR exprAnd | exprAnd
//recursivitate la stanga
// 12
bool exprOrPrim() {
    Token *start = iTk;
    if(consume(OR)) {
        if(exprAnd()) {
            if(exprOrPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa ||");
    }
    iTk = start;
    return true; // epsilon
}

bool exprOr() {
    Token *start = iTk;
    if(exprAnd()) {
        if(exprOrPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprAnd = exprAnd AND exprEq | exprEq
// recursivitate la stanga
// 13
bool exprAndPrim() {
    Token *start = iTk;
    if(consume(AND)) {
        if(exprEq()) {
            if(exprAndPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa &&");
    }
    iTk = start;
    return true; //epsilon
}

bool exprAnd() {
    Token *start = iTk;
    if(exprEq()) {
        if(exprAndPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprEq = exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
// recursivitate la stanga
// 14
bool exprEqPrim() {
    Token *start = iTk;
    if(consume(EQUAL)) {
        if(exprRel()) {
            if(exprEqPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa ==");
    }
    else if(consume(NOTEQ)) {
        if(exprRel()) {
            if(exprEqPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa !=");
    }
    iTk = start;
    return true; //epsilon
}

bool exprEq() {
    printf("#exprEq: %s\n", tkCodeName(iTk->code));
    Token *start = iTk;
    if(exprRel()) {
        if(exprEqPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprRel = exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
// recursivitate la stanga
// 15
bool exprRelPrim() {
    Token *start = iTk;
    if(consume(LESS)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        }  else tkerr("Lipseste expresia dupa <");
    }
    else if(consume(LESSEQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa <=");
    }
    else if(consume(GREATER)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa >");
    }
    else if(consume(GREATEREQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa >=");
    }
    iTk = start;
    return true; // epsilon
}

bool exprRel() {
    printf("#exprRel: %s\n", tkCodeName(iTk->code));
    Token *start = iTk;
    if(exprAdd()) {
        if(exprRelPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprAdd = exprAdd ( ADD | SUB ) exprMul | exprMul
// recursivitate la stanga
// 16
bool exprAddPrim() {
    Token *start = iTk;
    if(consume(ADD)) {
        if(exprMul()) {
            if(exprAddPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa +");
    }
    else if(consume(SUB)) {
        if(exprMul()) {
            if(exprAddPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa -");
    }
    iTk = start;
    return true; // epsilon
}

bool exprAdd() {
    printf("#exprAdd: %s\n", tkCodeName(iTk->code));
    Token *start = iTk;
    if(exprMul()) {
        if(exprAddPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}
//exprMul = exprMul ( MUL | DIV ) exprCast | exprCast
// recursivitate la stanga
// 17
bool exprMulPrim() {
    printf("#exprMulPrim: %s\n", tkCodeName(iTk->code));
    Token *start = iTk;
    if(consume(MUL)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa *");
    }
    else if(consume(DIV)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa /");
    }
    iTk = start;
    return true; // epsilon
}

bool exprMul() {
    printf("#exprMul: %s\n", tkCodeName(iTk->code));
    Token *start = iTk;
    if(exprCast()) {
        if(exprMulPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprCast = LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
// 18
bool exprCast() {
    Token *start = iTk;
    if(consume(LPAR)) {
        Type t;
        if(typeBase(&t)) {
            if(arrayDecl(&t)) {
                if(consume(RPAR)) {
                    if(exprCast()) {
                        return true;
                    }
                } else tkerr("Lipseste ) in expresia de cast");
            }
            // arrayDecl? - tratare caz optionalitate
            if(consume(RPAR)) {
                if(exprCast()) {
                    return true;
                }
            }
        } else tkerr("Lipseste sau este gresit tipul din expresia de cast");
    }

    iTk = start; // daca prima verificare nu merge ma intorc
    if(exprUnary()) {
        return true;
    }
    iTk = start;
    return false;
}

//exprUnary = ( SUB | NOT ) exprUnary | exprPostfix
// 19
bool exprUnary() {
    printf("#exprUnary: %s\n", tkCodeName(iTk->code));
    Token *start = iTk;
    if(consume(SUB)) {
        if(exprUnary()) {
            return true;
        } else tkerr("Lipseste expresia dupa -");
    }
    else if(consume(NOT)) {
        if(exprUnary()) {
            return true;
        } else tkerr("Lipseste expresia dupa !");
    }
    if(exprPostfix()) {
        return true;
    }
    iTk = start;
    return false;
}

//exprPostfix: exprPostfix LBRACKET expr RBRACKET
            // | exprPostfix DOT ID
            // | exprPrimary
//recursivitate la stanga
// 20
bool exprPostfixPrim() {
    Token *start = iTk;
    if(consume(LBRACKET)) {
        if(expr()) {
            if(consume(RBRACKET)) {
                if(exprPostfixPrim()) {
                    return true;
                }
            } else tkerr("Lipseste ] din accesarea vectorului");
        } else tkerr("Lipseste expresia din accesarea vectorului");
    }
    if(consume(DOT)) {
        if(consume(ID)) {
            if(exprPostfixPrim()) {
                return true;
            }
        } else tkerr("Lipseste identificatorul dupa .");
    }
    iTk = start;
    return true;
}

bool exprPostfix() {
    printf("#exprPostfix: %s\n", tkCodeName(iTk->code));
    Token *start = iTk;
    if(exprPrimary()) {
        if(exprPostfixPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
            //| INT | DOUBLE | CHAR | STRING | LPAR expr RPAR
// 21
bool exprPrimary() {
    Token *start = iTk;
    if(consume(ID)) {
        if(consume(LPAR)) {
            if(expr()) {
                for(;;) {
                    if(consume(COMMA)) {
                        if(expr()) {

                        } else {
                            tkerr("Lipseste expresia dupa , in apelul functiei");
                            break;
                        }
                    }
                    else break;
                }
            }
            if(consume(RPAR)) {
                return true;
            } else tkerr("Lipseste ) in apelul functiei");
        }
        return true;
    }

    iTk = start;
    if (consume(INT)) {
        return true;
        return true;
    }

    else if (consume(DOUBLE)) {
        return true;
    }
    else if (consume(CHAR)) {
        return true;
    }
    else if (consume(STRING)) {
        return true;
    }
    if(consume(LPAR)) {
        if(expr()) {
            if(consume(RPAR)) {
                return true;
            } else tkerr("Lipseste ) in apelul functiei");
        }
    }
    iTk = start;
    return false;
}

// unit: ( structDef | fnDef | varDef )* END
// 1
bool unit()
{
    printf("#unit: %s\n", tkCodeName(iTk->code));
    Token *start = iTk;
    for(;;)
    {
        if(structDef()) {}
        else if(varDef()) {}
        else if(fnDef()) {}
        else break;
    }
    if(consume(END))
    {
        return true;
    }
    iTk = start;
    return false;
}

void parse(Token *tokens)
{
    iTk=tokens;
    if(!unit())tkerr("syntax error");
}

