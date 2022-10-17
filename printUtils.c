#include "header.h"
#include "external.h"

/*Writes all the information gained from the parser into the memory for later usage.*/

/*In charge of data segment*/
void insertNumber(int num) {
    int flag_minus = 0, i = 0;
    int temp = num;
    if (num < MIN_VALUE_DATA || num > MAX_VALUE_DATA) {
        fprintf(stderr, "Error line %d: Overflow - %d.\n", lineCount, num);
        errorFlag = 1;
        return;
    }
    if (num < 0) {
        flag_minus = 1;
        temp = (num * -1) - 1;
    }
    do {
        memory[IC].word[i] = temp % 2;
        i++;
        temp = temp / 2;
    } while (temp != 0 && i >= 0);
    if (flag_minus == 1) {
        for (i = 0; i < 15; i++) {
            if(memory[IC].word[i] != 1)
                memory[IC].word[i]=1;
            else
                memory[IC].word[i]=0;
        }
    }
    IC++;
}

/*In charge of the code segment*/
void insertCommand(int value, int i, int j) {
    int minusFlag = 0;
    int temp = value;
    if (j - i == 11 && (value > MAX_VALUE || value < MIN_VALUE)) {/*If it's IMM and oob value*/
        fprintf(stderr, "Error line %d: Overflow - %d.\n", lineCount, value);
        errorFlag = 1;
        return;
    }
    if (value < 0) {
        minusFlag = 1;
        temp = (value * -1) - 1;
    }
    do {
        memory[IC].word[i] = temp % 2;
        i++;
        temp = temp / 2;
    } while (temp != 0 && i <= j);
    if (minusFlag == 1) {
        for (i = 3; i < 15; i++) {
            if (memory[IC].word[i] != 1)
                memory[IC].word[i] = 1;
            else
                memory[IC].word[i] = 0;
        }
    }
}


int fileGenerator(char* name) {
    FILE *fp1 = NULL, *fp2 = NULL; /* fp1 for read fp2 for write */
    if (!(openFile(&fp1 , name, "r", AS_EXTENSION)))
        return 1;
    parseFile(fp1);
    fclose(fp1);
    if (!errorFlag) {
        if (DC) {
            if (!(openFile(&fp1, name, "w", EXT_EXTENSION)))
                return 1;
            if (entryCount) {
                if (!(openFile(&fp2, name, "w", ENT_EXTENSION)))/* open .ent file to write */
                    return 1;
                labelUpdate(fp1, fp2);
            } else {
                fp2 = NULL;
                labelUpdate(fp1, NULL);
            }
            if (errorFlag) {
                if (fp1) {
                    fprintf(stderr, "Error occurred. Deleting temp file.\n");
                    remove(name);
                    fclose(fp1);
                }
                if (fp2) {
                    fprintf(stderr, "Error occurred. Deleting temp file.\n");
                    remove(name);
                    fclose(fp2);
                }

            } else {
                if (fp2)
                    fclose(fp2);
                if (fp1)
                    fclose(fp1);
            }

        } else {
            if (entry) {
                fprintf(stderr, "Undefined entry.\n");
                errorFlag = 1;
            }
        }
        if (!errorFlag) {
            if (!(openFile(&fp2, name, "w", OB_EXTENSION)))
                return 1;
            else {
                toOctal(fp2);
                fclose(fp2);
                printf("File parsing completed.\n\n");
            }
        } else
            fprintf(stderr, "Error occurred.\n");
        memset(memory, 0, sizeof(memory));
        free(labels);
        free(entry);

    } else {
        fprintf(stderr, "Error occurred. No files have been created.\n\n");
        free(labels);
        free(entry);
        memset(memory, 0, sizeof(memory));
    }
    return 0;
}

int openFile(FILE ** fp, char* name, char * permission, char * extension)
{
    char fileName[MAX_FILENAME];/*File name holder with a limit of length*/
    strcpy(fileName, name);/*Insert argv name into the empty char array*/
    strcat(fileName, extension);/*Add the necessary extension (.ob etc...)*/
    if (!(*fp = fopen(fileName, permission)))/*Opens the file with th necessary permission type*/
    {
        printf("Unable to open the file named %s.\n", fileName);
        return 0;
    }
    /*Return pointer to the file*/
    return 1;
}