#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*Defines for the file structure and legal limits*/
#define MEMORY_SIZE 4096 /*The amount of "words" one file can hold*/
#define MAX_LINE_LENGTH 80 /*Length of each line in the file*/
#define START_ADDRESS 100 /*Instruction counter starting address*/
#define MAX_STRING_LENGTH 31 /*String(label) length max limit*/
#define MAX_VALUE_DATA 16383 /*Max value that can be stored in 15 bits*/
#define MIN_VALUE_DATA -16383
#define MAX_VALUE 2047 /*Max value that can be stored in 11 bits, 1 bit is a leading bit*/
#define MIN_VALUE -2047
#define MAX_FILENAME 50 /*Logical file name length*/

/*Defines for the file extension types*/
#define AS_EXTENSION ".as"
#define OB_EXTENSION ".ob"
#define EXT_EXTENSION ".ext"
#define ENT_EXTENSION ".ent"

typedef enum {IMM,LABEL,PNT,REG} op_type;/*Different types of methods*/
typedef enum {DATA,LABL} label_type;/*Label types */
typedef enum {INT,STRING,ENTRY,EXTERN,EMPTY} data_type;/*Data types*/

/*Pointer to our struct*/
typedef struct labelStruct * ptr;

/*Struct for labels*/
typedef struct labelStruct{
    char name[MAX_STRING_LENGTH];/*Label name*/
    int address;/*Label address*/
    label_type l_type;
    data_type d_type;
}labelInfo;

/*Our main memory storage unit*/
struct memoryStruct{
	int word[16];/*15 bits of a word + one bit that will be used as flag bit*/
	char label[MAX_STRING_LENGTH];/*Name of the label*/
};

/*Each line in the memory*/
struct lineStruct{
	char cmd[5];/*Max length of a given command aka "move\0"*/
	int op_code,op[2];/*The operation code number, max 2 operands*/
	op_type type[4];/*Each of the viable operation types*/
};





int legalCommand(char *);/*Checks if a command is one of the viable commands*/
void parseFile(FILE *);/*Parses the file line by line*/
void parseLine(char *, int );/*Parses the given line to it's different attributes*/
void isLabel(char *,char *,int, int *);/*Checks if it's a legal label*/
void labelUpdate(FILE *,FILE *);/*Updates the address of all the labels from the first run*/
void isCommand(char *,char *,int, int *);/*Checks for the syntax of the given line*/
void insertNumber(int );/*Inserts the number into our main memory*/
void insertCommand(int , int , int ); /*Inserts the command into our main memory*/
void toBinary(struct lineStruct , int );/*Transfers each of the given commands into it's proper binary code*/
void binaryWriter(struct lineStruct , int , int , int );/*Support function to isBinary*/
void toOctal(FILE *);/*Converts the given binary based code to octal based code*/
int fileGenerator(char *); /*Generates the different file types and writes the information into them*/
int openFile(FILE **, char*, char *, char *); /*Opens the requested file*/