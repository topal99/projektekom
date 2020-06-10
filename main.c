#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"

int main(int argc, char *argv[])
{
    char line[256];
    char *file;
    file = (char *)malloc(sizeof(char) * 256);
    InstrList *startInstrList = NULL;
    InstrList *endInstrList = NULL;
    printf("Enter the file to be parsed: ");
    scanf("%s", file);
    FILE *fp = fopen(file, "r");

    if (fp == NULL)
    {
        printf("Error: %s does not exist or can't be parsed\n", file);
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), fp))
    {
        Instr *instr = parser(line);
        endInstrList = mkList(instr, &startInstrList, endInstrList);
        if (instr->op == LABEL)
            insert(instr->first->contents.name, (intptr_t)endInstrList);
    }
    initHashTable();
    run(startInstrList);

    return EXIT_SUCCESS;
}