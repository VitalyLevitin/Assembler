#include "header.h"


/* Author: Vitaly Levitin.
 *
 * This program mimics assembler behavior by receiving at least one .as file
 * Parses the given information inside of it (actual code) into octal based code to be used later
 * in the next decrypter to create machine code.
 * If there's an error parsing the data, outputs it as stderr output.
 * If the file parsed correctly, outputs 3 files that will store all the needed information:
 * file.ob - The code itself in octal base.
 * file.ent - A list of entries that came with the code.
 * file.ext - A list of external uses in the code.*/


struct memoryStruct memory[MEMORY_SIZE];/*Struct that controls the amount of information stored*/
int IC, DC, lineCount, entryCount, externalCount, errorFlag;/*External integers that serve a global role*/
ptr labels, entry;/*Pointers to structs that will hold all of the labels and entries*/


int main(int argc, char *argv[]) {
    int i, j;
    if (argc == 1) {
        fprintf(stderr, "No input file.\n");
        return 1;
    }
    for (i = 1; i < argc; i++) {
        printf("Opening the %d file:\n", i);
        j = fileGenerator(argv[i]);
        if(j==1)return 1;
    }
    return 0;
}