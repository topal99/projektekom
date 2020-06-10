#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "interpreter.h"

Instr *parser(char *line)
{
    if (line == NULL)
    {
        printf("Error in parser: NULL line!\n");
        exit(EXIT_FAILURE);
    }

    char var1[256], var2[256], var3[256], op;

    if (sscanf(line, "reav(%[0-9,a-z,A-Z]s);", var1) == 1)
        return mkInstr(READ, mkVar(strdup(var1)), mkEmpty(), mkEmpty());
    else if (sscanf(line, "prinv(%[0-9,a-z,A-Z]s);", var1) == 1)
        return mkInstr(PRINT, mkVar(strdup(var1)), mkEmpty(), mkEmpty());
    else if (sscanf(line, "%s = %s %c %[0-9,a-z,A-Z]s;", var1, var2, &op, var3) == 4)
    {
        if (isdigit(var2[0]))
        {
            if (isdigit(var3[0]))
            {
                if (op == '+')
                    return mkInstr(ADD, mkVar(strdup(var1)), mkInt(atoi(var2)), mkInt(atoi(var3)));
                else if (op == '-')
                    return mkInstr(SUB, mkVar(strdup(var1)), mkInt(atoi(var2)), mkInt(atoi(var3)));
                else if (op == '*')
                    return mkInstr(MUL, mkVar(strdup(var1)), mkInt(atoi(var2)), mkInt(atoi(var3)));
                else if (op == '/')
                    return mkInstr(DIV, mkVar(strdup(var1)), mkInt(atoi(var2)), mkInt(atoi(var3)));
            }
            else
            {
                if (op == '+')
                    return mkInstr(ADD, mkVar(strdup(var1)), mkInt(atoi(var2)), mkVar(strdup(var3)));
                else if (op == '-')
                    return mkInstr(SUB, mkVar(strdup(var1)), mkInt(atoi(var2)), mkVar(strdup(var3)));
                else if (op == '*')
                    return mkInstr(MUL, mkVar(strdup(var1)), mkInt(atoi(var2)), mkVar(strdup(var3)));
                else if (op == '/')
                    return mkInstr(DIV, mkVar(strdup(var1)), mkInt(atoi(var2)), mkVar(strdup(var3)));
            }
        }
        else
        {
            if (isdigit(var3[0]))
            {
                if (op == '+')
                    return mkInstr(ADD, mkVar(strdup(var1)), mkVar(strdup(var2)), mkInt(atoi(var3)));
                else if (op == '-')
                    return mkInstr(SUB, mkVar(strdup(var1)), mkVar(strdup(var2)), mkInt(atoi(var3)));
                else if (op == '*')
                    return mkInstr(MUL, mkVar(strdup(var1)), mkVar(strdup(var2)), mkInt(atoi(var3)));
                else if (op == '/')
                    return mkInstr(DIV, mkVar(strdup(var1)), mkVar(strdup(var2)), mkInt(atoi(var3)));
            }
            else
            {
                if (op == '+')
                    return mkInstr(ADD, mkVar(strdup(var1)), mkVar(strdup(var2)), mkVar(strdup(var3)));
                else if (op == '-')
                    return mkInstr(SUB, mkVar(strdup(var1)), mkVar(strdup(var2)), mkVar(strdup(var3)));
                else if (op == '*')
                    return mkInstr(MUL, mkVar(strdup(var1)), mkVar(strdup(var2)), mkVar(strdup(var3)));
                else if (op == '/')
                    return mkInstr(DIV, mkVar(strdup(var1)), mkVar(strdup(var2)), mkVar(strdup(var3)));
            }
        }
    }
    else if (sscanf(line, "%s = %[0-9,a-z,A-Z]s;", var1, var2) == 2)
    {
        if (isdigit(var2[0]))
            return mkInstr(ATRIB, mkVar(strdup(var1)), mkInt(atoi(var2)), mkEmpty());
        else
            return mkInstr(ATRIB, mkVar(strdup(var1)), mkVar(strdup(var2)), mkEmpty());
    }
    else if (sscanf(line, "iv %s goto %[0-9,a-z,A-Z]s;", var1, var2) == 2)
        return mkInstr(IF_I, mkVar(strdup(var1)), mkVar(strdup(var2)), mkEmpty());
    else if (sscanf(line, "label %[0-9,a-z,A-Z]s;", var1) == 1)
        return mkInstr(LABEL, mkVar(strdup(var1)), mkEmpty(), mkEmpty());
    else if (sscanf(line, "goto %[0-9,a-z,A-Z]s;", var1) == 1)
        return mkInstr(GOTO_I, mkVar(strdup(var1)), mkEmpty(), mkEmpty());
    else if (sscanf(line, "quit;") == 0)
        return mkInstr(QUIT, mkEmpty(), mkEmpty(), mkEmpty());
    else
    {
        printf("Error in parser: Instruction \"%s\"not recognized!\n", line);
        exit(EXIT_FAILURE);
    }

    return NULL;
}

void run(InstrList *instrList)
{
    InstrList *aux;
    InstrList *instrListAux = instrList;
    while (instrListAux != NULL)
    {
        switch (instrListAux->instr->op)
        {
        case ATRIB:
            if (instrListAux->instr->second->kind == INT_CONST)
                runATRIB(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val);
            else
                runATRIB(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value);
            break;
        case ADD:
            if (instrListAux->instr->second->kind == STRING)
            {
                if (instrListAux->instr->third->kind == STRING)
                    runADD(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value, lookup(instrListAux->instr->third->contents.name)->value);
                else
                    runADD(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value, instrListAux->instr->third->contents.val);
            }
            else
            {
                if (instrListAux->instr->third->kind == STRING)
                    runADD(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val, lookup(instrListAux->instr->third->contents.name)->value);
                else
                    runADD(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val, instrListAux->instr->third->contents.val);
            }
            break;
        case SUB:
            if (instrListAux->instr->second->kind == STRING)
            {
                if (instrListAux->instr->third->kind == STRING)
                    runSUB(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value, lookup(instrListAux->instr->third->contents.name)->value);
                else
                    runSUB(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value, instrListAux->instr->third->contents.val);
            }
            else
            {
                if (instrListAux->instr->third->kind == STRING)
                    runSUB(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val, lookup(instrListAux->instr->third->contents.name)->value);
                else
                    runSUB(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val, instrListAux->instr->third->contents.val);
            }
            break;
        case MUL:
            if (instrListAux->instr->second->kind == STRING)
            {
                if (instrListAux->instr->third->kind == STRING)
                    runMUL(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value, lookup(instrListAux->instr->third->contents.name)->value);
                else
                    runMUL(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value, instrListAux->instr->third->contents.val);
            }
            else
            {
                if (instrListAux->instr->third->kind == STRING)
                    runMUL(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val, lookup(instrListAux->instr->third->contents.name)->value);
                else
                    runMUL(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val, instrListAux->instr->third->contents.val);
            }
            break;
        case DIV:
            if (instrListAux->instr->second->kind == STRING)
            {
                if (instrListAux->instr->third->kind == STRING)
                    runDIV(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value, lookup(instrListAux->instr->third->contents.name)->value);
                else
                    runDIV(instrListAux->instr->first->contents.name, lookup(instrListAux->instr->second->contents.name)->value, instrListAux->instr->third->contents.val);
            }
            else
            {
                if (instrListAux->instr->third->kind == STRING)
                    runDIV(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val, lookup(instrListAux->instr->third->contents.name)->value);
                else
                    runDIV(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.val, instrListAux->instr->third->contents.val);
            }
            break;
        case IF_I:
            aux = runIF_I(instrListAux->instr->first->contents.name, instrListAux->instr->second->contents.name);
            if (aux != NULL)
                instrListAux = aux;
            break;
        case PRINT:
            runPRINT(instrListAux->instr->first->contents.name);
            break;
        case READ:
            runREAD(instrListAux->instr->first->contents.name);
            break;
        case GOTO_I:
            instrListAux = runGOTO_I(instrListAux->instr->first->contents.name);
            break;
        case LABEL:
            break;
        case QUIT:
            runQUIT();
            break;
        default:
            printf("Internal error in interpreter: OpKind \"%d\" doesnt exist!\n", instrListAux->instr->op);
        }
        instrListAux = instrListAux->next;
    }
}

void runATRIB(char *name, int val)
{
    insert(name, val);
}

void runADD(char *name, int a, int b)
{
    insert(name, a + b);
}

void runSUB(char *name, int a, int b)
{
    insert(name, a - b);
}

void runMUL(char *name, int a, int b)
{
    insert(name, a * b);
}

void runDIV(char *name, int a, int b)
{
    insert(name, a / b);
}

InstrList *runIF_I(char *var, char *label)
{
    if (lookup(var) != NULL)
        if (lookup(var)->value != 0)
            return runGOTO_I(label);

    return NULL;
}

void runPRINT(char *var)
{
    printf("Nilai dari %s adalah: %d\n", var, (int)lookup(var)->value);
}

void runREAD(char *var)
{
    int x;
    printf("Masukkan nilai dari %s: ", var);
    scanf("%d", &x);

    insert(var, x);
}

InstrList *runGOTO_I(char *label)
{
    if (lookup(label) != NULL)
        if (lookup(label)->value != 0)
            return (InstrList *)lookup(label)->value;

    printf("Error in interpreter: label \"%s\" does not exist!\n", label);
    exit(EXIT_FAILURE);
    return NULL;
}

void runQUIT()
{
    printf("Projek Tekkom Menggunakan C Interpreter\n");
    exit(EXIT_SUCCESS);
}

Elem *mkVar(char *s)
{
    Elem *elem = (Elem *)malloc(sizeof(Elem));
    elem->kind = STRING;
    elem->contents.name = s;

    return elem;
}

Elem *mkInt(int x)
{
    Elem *elem = (Elem *)malloc(sizeof(Elem));
    elem->kind = INT_CONST;
    elem->contents.val = x;

    return elem;
}

Elem *mkEmpty()
{
    Elem *elem = (Elem *)malloc(sizeof(Elem));
    elem->kind = EMPTY;

    return elem;
}

Instr *mkInstr(OpKind op, Elem *x, Elem *y, Elem *z)
{
    Instr *instr = (Instr *)malloc(sizeof(Instr));
    instr->op = op;
    instr->first = x;
    instr->second = y;
    instr->third = z;

    return instr;
}

InstrList *mkList(Instr *instr, InstrList **startInstrList, InstrList *endInstrList)
{
    InstrList *instrListAux = (InstrList *)malloc(sizeof(InstrList));
    instrListAux->instr = instr;
    instrListAux->next = NULL;

    if (*startInstrList == NULL)
        *startInstrList = instrListAux;

    if (endInstrList != NULL)
        endInstrList->next = instrListAux;

    return instrListAux;
}

void initHashTable()
{
    memset(table, 0, sizeof(HASH_SIZE));
}

unsigned int hash(char *str)
{
    unsigned int h;
    unsigned char *p;
    h = 0;
    for (p = (unsigned char *)str; *p != '\0'; p++)
    {
        h = MULTIPLIER * h + *p;
    }
    return h % HASH_SIZE;
}

List *lookup(char *key)
{
    int index = (int)hash(key);
    List *aux = table[index];
    while (aux != NULL)
    {
        if (!strcmp(key, aux->key))
        {
            return aux;
        }
        aux = aux->next;
    }
    return NULL;
}

void insert(char *key, intptr_t value)
{
    int val = (int)hash(key);
    List *aux = (List *)malloc(sizeof(List));
    aux->key = key;
    aux->value = value;
    aux->next = table[val];
    table[val] = aux;
}
