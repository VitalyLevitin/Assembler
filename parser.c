#include "header.h"
#include "external.h"

/*Receiving a single line from the text file, if the format is correct parses it the proper binary code
* If the code is incorrect writes an error to stderr. (will be printed after checking the whole file)*/

/*Struct that stores the given command with all it's information*/
struct cmdStruct {
    char name[5]; /* 4 chars + '0' */
    int code; /* code between 0 and 15 */
    int operands; /* how many operands for this command (0-2) */
};
/*The different viable types of commands*/
struct cmdStruct cmds[] = {
        {"mov",  0,  2},
        {"cmp",  1,  2},
        {"add",  2,  2},
        {"sub",  3,  2},
        {"lea",  4,  2},
        {"clr",  5,  1},
        {"not",  6,  1},
        {"inc",  7,  1},
        {"dec",  8,  1},
        {"jmp",  9,  1},
        {"bne",  10, 1},
        {"red",  11, 1},
        {"prn",  12, 1},
        {"jsr",  13, 1},
        {"rts",  14, 0},
        {"stop", 15, 0}
};


/*Returns the operation code num or -1 if wrong operation code*/
int legalCommand(char name[]) {
    int i;
    for (i = 0; i < 16; i++) {
        if (!strcmp(cmds[i].name, name))
            return i;
    }
    return -1;
}

/*Receives a label and reallocates the memory for the new given label.
 * Checks what type of label we received and categorizes it.
 * If it's not a legal label, will print it to stderr and abort the check*/
void isLabel(char line[], char label[], int lineLength, int *pos) {
    int i, j = 0;
    char token[MAX_STRING_LENGTH], ch = 0;
    if (label[0] != '\0') {/*checks if the label exist*/
        if (!DC)/*If the list in empty allocate the first data word*/
            labels = (ptr) malloc(sizeof(labelInfo));
        else
            labels = (ptr) realloc(labels, sizeof(labelInfo) * (DC + 1));/*Reallocate the list and expands it for new incoming data*/
        if (!labels) {
            fprintf(stderr, "Failure to allocate memory, aborts.\n");
            errorFlag = 1;
            return;
        }
        labels[DC].address = START_ADDRESS + IC;
        strcpy(labels[DC].name, label);
        for (ch = line[*pos]; (ch == ' ' || ch == '\t') && *pos < lineLength; ch = line[*pos], (*pos)++);
        if (*pos == lineLength) {
            labels[DC].l_type = LABL;
            labels[DC].d_type = EMPTY;/*label without any data type*/
            DC++;
            return;
        }
    }
    else {
        ch = line[*pos];
        (*pos)++;
    }/*Saving the string in a token*/
    for (i = 1, token[0] = ch; ch != ' ' && ch != '\t' && *pos <=lineLength; ch = line[*pos], token[i] = ch, i++, (*pos)++);
    if (i < MAX_STRING_LENGTH) {
        i--;
        token[i] = '\0';
    }
    if (legalCommand(token) != -1) {
        labels[DC].l_type = LABL;
        labels[DC].d_type = EMPTY;
        DC++;
        isCommand(line, token, lineLength, pos);
        return;
    }
    if (token[0] == '.' && i >= 4 && i <= 7) {
        if (!strcmp(token, ".data")) {
            if (label[0] != '\0') {
                labels[DC].l_type = DATA;
                labels[DC].d_type = INT;/*data of type int*/
                DC++;
            }
            for (ch = line[*pos]; *pos <= lineLength; (*pos)++, ch = line[*pos]) {
                if (ch != ' ' && ch != '\t' && j < MAX_STRING_LENGTH) {
                    /*Checking if it's a number*/
                    if ((ch >= '0' && ch <= '9') || (ch == '-' && j == 0)) {
                        token[j] = ch;
                        j++;
                    }
                    else {/*Got to the end of the number or unknown sign*/
                        if ((ch == ',' || ch == '\n' || *pos == lineLength) && j != 0) {
                            token[j] = '\0';/*End of String*/
                            j = 0;
                            insertNumber(atoi(token));/*writes the number into the main memory*/
                        } else {
                            fprintf(stderr, "Error line: %d Unknown character %c.\n", lineCount, ch);
                            errorFlag = 1;
                            return;
                        }
                    }
                }
            }
        }
        else {
            if (!strcmp(token, ".ext") || !strcmp(token, ".extern")) {
                if (!DC)/*If the list in empty allocate the first data word*/
                    labels = (ptr) malloc(sizeof(labelInfo));
                else
                    labels = (ptr) realloc(labels, sizeof(labelInfo) * (DC + 1));/*Reallocate the list and expands it for new incoming data*/
                if (!labels) {
                    fprintf(stderr, "Failure to allocate memory, aborts.\n");
                    errorFlag = 1;
                    return;
                }
                labels[DC].l_type = DATA;
                labels[DC].d_type = EXTERN;/*the data type extern*/
                /*Jumping spaces and tabs*/
                for (ch = line[*pos]; (ch == ' ' || ch == '\t') && *pos <lineLength; (*pos)++, ch = line[*pos]);
                /*Saving the next part in a temp var*/
                for (i = 0; ch != ' ' && ch != '\t' && *pos < lineLength; (*pos)++, ch = line[*pos]) {
                    if ((isalpha(ch) && i == 0) || ((isalpha(ch) || isdigit(ch)) && i != 0)) {
                        token[i] = ch;
                        i++;
                    }
                    else {
                        fprintf(stderr, "Error line %d: Invalid character.\n", lineCount);
                        errorFlag = 1;
                        return;
                    }
                }
                if (i < MAX_STRING_LENGTH && i != 0)
                    token[i] = '\0';
                else {
                    fprintf(stderr, "Error line: %d empty .extern declaration.\n", lineCount);
                    errorFlag = 1;
                    return;
                }
                for (ch = line[++(*pos)]; ch != '\0' && *pos <= lineLength; (*pos)++) {
                    if (ch != ' ' && ch != '\t') {
                        fprintf(stderr, "Error line %d: Wrong character after label declaration.\n", lineCount);
                        errorFlag = 1;
                        return;
                    }
                }
                if (i < MAX_STRING_LENGTH)
                    token[i] = '\0';
                labels[DC].address = START_ADDRESS + IC;
                strcpy(labels[DC].name, token);
                externalCount++;
                DC++;
                return;
            }
            else {
                /*Checking if it's a legal string declaration*/
                if (!strcmp(token, ".string")) {
                    if (label[0] != '\0') {
                        labels[DC].l_type = DATA;
                        labels[DC].d_type = STRING;
                        DC++;
                    }
                    for (; (ch == ' ' || ch == '\t') && *pos < lineLength; ch = line[*pos], (*pos)++);
                    if (ch == '"') {
                        for (ch = line[*pos], (*pos)++; ch != '"' && *pos <= lineLength; ch = line[*pos], (*pos)++)
                            insertNumber(ch);/*we got two quote marks so marking end of string*/
                        insertNumber(0);
                        if (ch != '"') {
                            fprintf(stderr, "Error line %d: String must end with the \" character.\n", lineCount);
                            errorFlag = 1;
                        }
                    }
                    else {
                        fprintf(stderr, "Error line %d: String must start with the \" character.\n", lineCount);
                        errorFlag = 1;
                    }
                    ch = line[*pos];
                    for (; ch != '\0' && *pos <= lineLength; (*pos)++) {
                        ch = line[*pos];
                        if (ch != ' ' && ch != '\t') {
                            fprintf(stderr, "Error line %d: Unknown character at the end of the string - %c.\n", lineCount, ch);
                            errorFlag = 1;
                            return;
                        }
                    }
                }
                else {/*Checking if it's a entry data type*/
                    if (!strcmp(token, ".entry")) {
                        if (!entryCount)
                            entry = (ptr) malloc(sizeof(labelInfo));
                        else
                            entry = (ptr) realloc(entry, sizeof(labelInfo) * (entryCount + 1));
                        if (!entry) {
                            fprintf(stderr, "Failure to allocate memory, aborts.\n");
                            errorFlag = 1;
                            return;
                        }
                        entry[entryCount].l_type = DATA;
                        entry[entryCount].d_type = ENTRY;
                        for (ch = line[*pos]; (ch == ' ' || ch == '\t') && *pos < lineLength; (*pos)++, ch = line[*pos]);
                        /*Saving the next part in a temp token*/
                        for (i = 0; ch != ' ' && ch != '\t' && *pos < lineLength; (*pos)++, ch = line[*pos]) {
                            if ((isalpha(ch) && i == 0) || ((isalpha(ch) || isdigit(ch)) && i != 0)) {
                                token[i] = ch;
                                i++;
                            }
                            else {
                                fprintf(stderr, "Error line %d: Wrong character.\n", lineCount);
                                errorFlag = 1;
                                return;
                            }
                        }
                        if (i < MAX_STRING_LENGTH && i != 0)
                            token[i] = '\0';
                        else {
                            fprintf(stderr, "Error line %d: Empty .entry declaration.\n", lineCount);
                            errorFlag = 1;
                            return;
                        }

                        for (; ch != '\0' && *pos <= lineLength; (*pos)++) {
                            ch = line[*pos];
                            if (ch != ' ' && ch != '\t') {
                                fprintf(stderr, "Error line %d: Unknown character at the end of the entry declaration - %c.\n", lineCount, ch);
                                errorFlag = 1;
                                return;
                            }
                        }
                        entry[entryCount].address = START_ADDRESS + IC;
                        strcpy(entry[entryCount].name, token);
                        entryCount++;
                        return;
                    }
                    else {
                        fprintf(stderr, "Error line %d: Unknown command after label declaration %s.\n", lineCount, token);
                        errorFlag = 1;
                        return;
                    }
                }
            }
        }
    } else {
            fprintf(stderr, "Error line %d: Undefined action after label declaration.\n", lineCount);
        errorFlag = 1;
    }
}
/*Checks a given line, if the token is a command writes it into the memory.
 * If errors arise, reports them*/
void isCommand(char line[], char token[], int lineLength, int *pos) {
    struct lineStruct currentLine;
    int i, operands = 0, flagComma = 0;
    if ((i = legalCommand(token)) == -1)
        return;
    strcpy(currentLine.cmd, token);
    currentLine.op_code = i;

    for (; *pos < lineLength && (line[*pos] == ' ' || line[*pos] == '\t'); (*pos)++);
    /*Checking if the starting line is a legal start*/
    if (line[*pos] == '#' || line[*pos] == '*' || isalpha(line[*pos])) {
        while (*pos < lineLength && operands < 2) {/*2 because we can only have 2 operands*/
            for (; *pos < lineLength && (line[*pos] == ' ' || line[*pos] == '\t'); (*pos)++);

            if (line[*pos] == '#') {/*If it's an immediate number*/
                for (i = 0, (*pos)++; *pos < lineLength && line[*pos] != ',' && line[*pos] != '\n' && line[*pos] != ' '; (*pos)++) {
                    if (isdigit(line[*pos]) || (i == 0 && line[*pos] == '-')) {
                        token[i] = line[*pos];
                        i++;
                    }
                    else {
                        fprintf(stderr, "Error line %d: After \"#\" character, there must be a valid number declaration.\n", lineCount);
                        errorFlag = 1;
                        return;
                    }
                }/*END FOR*/
                if (i == 0)/*Only #*/
                    fprintf(stderr, "Error line %d: No number detected.\n", lineCount);
                else {/*We got our Token*/
                    token[i] = '\0';
                    currentLine.op[operands] = (atoi(token));
                    currentLine.type[operands] = IMM;
                    operands++;
                }
            }/*END IMM CHECK*/
            else if (line[*pos] == '*') {/*If it's a Pointer*/
                for (i = 0; *pos < lineLength && line[*pos] != ' ' && line[*pos] != '\t' && line[*pos] != ',' &&
                     (isalpha(line[*pos]) || isdigit(line[*pos]) || line[*pos] == '*');
                     token[i] = line[*pos], i++, (*pos)++);
                token[i] = '\0';
                if (i == 3 && token[0] == '*' && token[1] == 'r' &&  isdigit(token[2])) {
                    if (atoi(token + 2) >= 8) {/*Token +2 since the operand looks like "*r2"*/
                        fprintf(stderr, "Error line %d: Non valid pointer declaration.\n", lineCount);
                        errorFlag = 1;
                    }
                    currentLine.op[operands] = (atoi(token + 2));
                    currentLine.type[operands] = PNT;
                    operands++;
                }
            }
            else if (line[*pos] == 'r') {/*Register check*/
                for (i = 0;*pos < lineLength && line[*pos] != ' ' && line[*pos] != '\t' && line[*pos] != ',' &&
                     (isalpha(line[*pos]) || isdigit(line[*pos]));
                     token[i] = line[*pos], i++, (*pos)++);
                token[i] = '\0';
                if (i == 2 && token[0] == 'r' && (isdigit(token[1])) != 0) {
                    if (atoi(token + 1) >= 8) {
                        fprintf(stderr, "Error line %d: Non valid Register declaration.\n", lineCount);
                        errorFlag = 1;
                    }
                    currentLine.op[operands] = (atoi(token + 1));
                    currentLine.type[operands] = REG;
                    operands++;
                }
            } else {/*IS A LABEL*/
                for (i = 0;*pos < lineLength && line[*pos] != ' ' && line[*pos] != '\t' && line[*pos] != ',' &&
                     (isalpha(line[*pos]) || isdigit(line[*pos]));
                     token[i] = line[*pos], i++, (*pos)++);
                token[i] = '\0';
                if (isalpha(token[0])) {
                    strcpy(memory[IC + operands + 1].label, token);
                    currentLine.type[operands] = LABEL;
                    operands++;
                }
                else {
                    fprintf(stderr, "Error line %d: Unknown command type %s.\n", lineCount, token);
                    errorFlag = 1;
                }
            }
            for (; *pos < lineLength && (line[*pos] == ' ' || line[*pos] == '\t'); (*pos)++);
            if (line[*pos] == ',') {
                (*pos)++;
                flagComma++;
            }
        }/* END WHILE */
    } else if (*pos < lineLength && line[*pos] != ' ' && line[*pos] != '\t' && line[*pos] != '\0') {
        fprintf(stderr, "Error line %d: Unknown declaration start %c.\n", lineCount, line[*pos]);
        errorFlag = 1;
        return;
    }
    if (operands != 0 && flagComma >= operands) {
        fprintf(stderr, "Error line %d: Expected operand after \",\" character.\n", lineCount);
        errorFlag = 1;
        return;
    }
    toBinary(currentLine, operands);
}
/*Receives a given line and checks if the syntax is legal.
 * if the syntax is legal translates the line into binary code.
 * if we have a label, we mark it with a flag to update in the second passage.
 * If there's an error writes it to strerr and moves to the next line.*/
void toBinary(struct lineStruct currentLine, int operands) {
    if (cmds[currentLine.op_code].operands != operands && (operands > 2)) {
        fprintf(stderr, "Error line %d: Incorrect command type / operand amount (needs to be between 0 and two).\n", lineCount);
        errorFlag = 1;
        return;
    }

    if (operands == 0) {/*Commands that have no operands*/
        if (!strcmp(currentLine.cmd, "stop") || !strcmp(currentLine.cmd, "rts")) {
            insertCommand(1, 2, 2);/*Adding 'A' tag*/
            insertCommand(currentLine.op_code, 11, 14);
            IC++;
        }
        else {
            fprintf(stderr, "Error line %d: Incorrect command type / operand amount (needs to be zero).\n", lineCount);
            errorFlag = 1;
            return;
        }
    }
    else if (operands == 1) {/*Commands with only one operand*/
        if (!strcmp(currentLine.cmd, "clr") || !strcmp(currentLine.cmd, "not") || !strcmp(currentLine.cmd, "inc") ||
            !strcmp(currentLine.cmd, "dec") || !strcmp(currentLine.cmd, "red")) {
            if (currentLine.type[0] != LABEL && currentLine.type[0] != REG && currentLine.type[0] != PNT) {
                fprintf(stderr, "Error line %d: Invalid second operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            insertCommand(1, 2, 2);/*Adding 'A' tag*/
            insertCommand(currentLine.op_code, 11, 14);

            if (currentLine.type[0] == LABEL) {
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else if (currentLine.type[0] == REG) {
                insertCommand(1, 6, 6);/*Marking it's a REGISTER*/
                IC++;
                binaryWriter(currentLine,1,1,2);/*Location 1 == dest bits*/
            } else {/*It's a pointer*/
                insertCommand(1, 5, 5);/*Marking it's a POINTER*/
                IC++;
                binaryWriter(currentLine,1,1,2);/*Location 1 == dest bits*/
            }
        } else if (!strcmp(currentLine.cmd, "jmp") || !strcmp(currentLine.cmd, "bne") || !strcmp(currentLine.cmd, "jsr")) {
            if (currentLine.type[0] != LABEL && currentLine.type[0] != PNT) {
                fprintf(stderr, "Error line %d: Invalid first operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            insertCommand(1, 2, 2);/*Adding 'A' tag*/
            insertCommand(currentLine.op_code, 11, 14);
            if (currentLine.type[0] == LABEL) {
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            }
            else {/*It's a pointer*/
                insertCommand(1, 5, 5);/*Marking it's a pointer in dest*/
                IC++;
                binaryWriter(currentLine,1,1,2);/*Location 1 == dest bits*/
            }
        } else if (!strcmp(currentLine.cmd, "prn")) {
            if (currentLine.type[0] != IMM && currentLine.type[0] != REG && currentLine.type[0] != LABEL &&
                currentLine.type[0] != PNT) {
                fprintf(stderr, "Error line %d: Invalid second operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            insertCommand(1, 2, 2);/*Adding 'A' tag*/
            insertCommand(currentLine.op_code, 11, 14);

            if (currentLine.type[0] == IMM) {
                insertCommand(1, 3, 3);/*Adding Method 0, as dest*/
                IC++;
                binaryWriter(currentLine,1,1,3);/*Location 1 means dest bits*/
            } else if (currentLine.type[0] == REG) {
                insertCommand(1, 6, 6);
                IC++;
                binaryWriter(currentLine,2,1,2);/*Location 2 means source bits*//*Location 2 == source bits*/
            } else if (currentLine.type[0] == LABEL) {
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else {
                insertCommand(1, 5, 5);/*Marking as Method 2, in dest*/
                IC++;
                binaryWriter(currentLine,2,1,2);/*Location 2 means source bits*//*Location 2 == source bits*/
            }
        } else { /*Not a viable 1 operator method*/
            fprintf(stderr, "Error line %d: Incorrect command type / number of operands (needs to be 1).\n", lineCount);
            errorFlag = 1;
            return;
        }

    } else if (operands == 2) {/*Commands with 2 operands*/
        if (!strcmp(currentLine.cmd, "mov") || !strcmp(currentLine.cmd, "add") || !strcmp(currentLine.cmd, "sub")) {
            if (currentLine.type[0] != REG && currentLine.type[0] != LABEL && currentLine.type[0] != IMM &&
                currentLine.type[0] != PNT) {
                fprintf(stderr, "Error line %d: Invalid first operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            if (currentLine.type[1] != LABEL && currentLine.type[1] != REG && currentLine.type[1] != PNT) {
                fprintf(stderr, "Error line %d: Invalid second operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            insertCommand(1, 2, 2);
            insertCommand(currentLine.op_code, 11, 14);
            if (currentLine.type[0] == REG && currentLine.type[1] == REG) {
                insertCommand(1, 6, 6);
                insertCommand(1, 10, 10);
                IC++;
                binaryWriter(currentLine,2,2,0);
            } else if (currentLine.type[0] == REG && currentLine.type[1] == PNT) {
                insertCommand(1, 5, 5);
                insertCommand(1, 10, 10);
                IC++;
                binaryWriter(currentLine,2,2,0);
            } else if (currentLine.type[0] == REG && currentLine.type[1] == LABEL) {
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                insertCommand(1, 10, 10);/*Marking as Method 3, in source*/
                IC++;
                binaryWriter(currentLine,2,1,2);/*Location 2 means source bits*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else if (currentLine.type[0] == PNT && currentLine.type[1] == PNT) {
                insertCommand(1, 5, 5);/*Marking as Method 2, in dest*/
                insertCommand(1, 9, 9);/*Marking as Method 2, in source*/
                IC++;
                binaryWriter(currentLine,2,2,0);/*First operand is source*/
            } else if (currentLine.type[0] == PNT && currentLine.type[1] == REG) {
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                insertCommand(1, 9, 9);/*Marking as Method 2, in source*/
                IC++;
               binaryWriter(currentLine,2,2,0);/*First operand is source*/
            } else if (currentLine.type[0] == PNT && currentLine.type[1] == LABEL) {
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                insertCommand(1, 9, 9);/*Marking as Method 2, in source*/
                IC++;
                binaryWriter(currentLine,2,1,2);/*Location 2 means source bits*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else if (currentLine.type[0] == LABEL && currentLine.type[1] == LABEL) {
                insertCommand(1, 8, 8);/*Marking as Method 1, in source*/
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else if (currentLine.type[0] == LABEL && currentLine.type[1] == REG) {
                insertCommand(1, 8, 8);/*Marking as Method 1, in source*/
                insertCommand(1, 6, 6);/*Marking as Method 3, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,1,2,0);/*Second operand written into dest bits*/
            } else if (currentLine.type[0] == LABEL && currentLine.type[1] == PNT) {
                insertCommand(1, 8, 8);/*Marking as Method 1, in source*/
                insertCommand(1, 5, 5);/*Marking as Method 2, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,1,2,0);/*Second operand written into dest bits*/
            } else if (currentLine.type[0] == IMM && currentLine.type[1] == REG) {
                insertCommand(1, 7, 7);/*Marking as Method 1, in source*/
                insertCommand(1, 6, 6);/*Marking as Method 3, in dest*/
                IC++;
                binaryWriter(currentLine,1,1,3);/*Location 1 means dest bits*/
                binaryWriter(currentLine,1,2,0);/*two operands, so we need to write second operand into dest*/
            } else if (currentLine.type[0] == IMM && currentLine.type[1] == PNT) {
                insertCommand(1, 7, 7);/*Marking as Method 1, in source*/
                insertCommand(1, 5, 5);/*Marking as Method 3, in dest*/
                IC++;
                binaryWriter(currentLine,1,1,3);/*Location 1 means dest bits*/
                binaryWriter(currentLine,1,2,0);/*two operands, so we need to write second operand into dest*/
            } else if (currentLine.type[0] == IMM && currentLine.type[1] == LABEL) {
                insertCommand(1, 7, 7);/*Marking as Method 1, in source*/
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                IC++;
                binaryWriter(currentLine,1,1,3);/*Location 1 means dest bits*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            }

        }  else if (!strcmp(currentLine.cmd, "cmp")) {
            if (currentLine.type[0] != REG && currentLine.type[0] != PNT && currentLine.type[0] != LABEL &&
                currentLine.type[0] != IMM) {
                fprintf(stderr, "Error line %d: Invalid first operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            if (currentLine.type[1] != REG && currentLine.type[1] != PNT && currentLine.type[1] != LABEL &&
                currentLine.type[1] != IMM) {
                fprintf(stderr, "Error line %d: Invalid second operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            insertCommand(1, 2, 2);/*Adding A flag*/
            insertCommand(currentLine.op_code, 11, 14);
            if (currentLine.type[0] == REG && currentLine.type[1] == REG) {
                insertCommand(1, 6, 6);
                insertCommand(1, 10, 10);
                IC++;
                binaryWriter(currentLine,2,2,0);/*First operand is source*/
            } else if (currentLine.type[0] == REG && currentLine.type[1] == PNT) {
                insertCommand(1, 5, 5);
                insertCommand(1, 10, 10);
                IC++;
                binaryWriter(currentLine,2,2,0);/*First operand is source*/
            } else if (currentLine.type[0] == REG && currentLine.type[1] == LABEL) {
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                insertCommand(1, 10, 10);/*Marking as Method 3, in source*/
                IC++;
                binaryWriter(currentLine,2,1,2);/*Location 2 means source bits*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else if (currentLine.type[0] == REG && currentLine.type[1] == IMM) {
                insertCommand(1, 3, 3);/*Marking as Method 0, in dest*/
                insertCommand(1, 10, 10);/*Marking as Method 3, in source*/
                IC++;
                binaryWriter(currentLine,2,1,2);/*Location 2 means source bits*/
                binaryWriter(currentLine,2,1,3);/*Second operand is IMM*/
            } else if (currentLine.type[0] == PNT && currentLine.type[1] == PNT) {
                insertCommand(1, 5, 5);/*Marking as Method 2, in dest*/
                insertCommand(1, 9, 9);/*Marking as Method 2, in source*/
                IC++;
                binaryWriter(currentLine,2,2,0);
            } else if (currentLine.type[0] == PNT && currentLine.type[1] == REG) {
                insertCommand(1, 4, 4);/*Marking as Method 3, in dest*/
                insertCommand(1, 9, 9);/*Marking as Method 2, in source*/
                IC++;
                binaryWriter(currentLine,2,2,0);
            } else if (currentLine.type[0] == PNT && currentLine.type[1] == LABEL) {
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                insertCommand(1, 9, 9);/*Marking as Method 2, in source*/
                IC++;
                binaryWriter(currentLine,2,1,2);/*Location 2 means source bits*//*Location is source and it's the first operand*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else if (currentLine.type[0] == PNT && currentLine.type[1] == IMM) {
                insertCommand(1, 3, 3);/*Marking as Method 0, in dest*/
                insertCommand(1, 9, 9);/*Marking as Method 2, in source*/
                IC++;
                binaryWriter(currentLine,2,1,2);/*Location 2 means source bits*/
                binaryWriter(currentLine,2,1,3);/*Second operand is IMM*/
            } else if (currentLine.type[0] == LABEL && currentLine.type[1] == LABEL) {
                insertCommand(1, 8, 8);/*Marking as Method 1, in source*/
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else if (currentLine.type[0] == LABEL && currentLine.type[1] == REG) {
                insertCommand(1, 8, 8);/*Marking as Method 1, in source*/
                insertCommand(1, 6, 6);/*Marking as Method 3, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,1,2,0);/*Second operand needs to be written into dest bits*/
            } else if (currentLine.type[0] == LABEL && currentLine.type[1] == PNT) {
                insertCommand(1, 8, 8);/*Marking as Method 1, in source*/
                insertCommand(1, 5, 5);/*Marking as Method 2, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,1,2,0);/*Second operand needs to be written into dest bits*/
            } else if (currentLine.type[0] == LABEL && currentLine.type[1] == IMM) {
                insertCommand(1, 8, 8);/*Marking as Method 1, in source*/
                insertCommand(1, 3, 3);/*Marking as Method 0, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,2,1,3);/*Location 2 means source bits*/
            } else if (currentLine.type[0] == IMM && currentLine.type[1] == IMM) {
                insertCommand(1, 7, 7);/*Marking as Method 0, in source*/
                insertCommand(1, 3, 3);/*Marking as Method 0, in dest*/
                IC++;
                binaryWriter(currentLine,1,1,3);/*Location 1 means dest bits*/
                binaryWriter(currentLine,2,1,3);/*Location 2 means source bits*/
            } else if (currentLine.type[0] == IMM && currentLine.type[1] == REG) {
                insertCommand(1, 7, 7);/*Marking as Method 0, in source*/
                insertCommand(1, 6, 6);/*Marking as Method 3, in dest*/
                IC++;
                binaryWriter(currentLine,1,1,3);/*Location 1 means dest bits*/
                binaryWriter(currentLine,1,2,0);/*Second operand needs to be written into dest bits*/
            } else if (currentLine.type[0] == IMM && currentLine.type[1] == PNT) {
                insertCommand(1, 7, 7);/*Marking as Method 0, in source*/
                insertCommand(1, 5, 5);/*Marking as Method 2, in dest*/
                IC++;
                binaryWriter(currentLine,1,1,3);/*Location 1 means dest bits*/
                binaryWriter(currentLine,1,2,0);/*Second operand needs to be written into dest bits*/
            } else if (currentLine.type[0] == IMM && currentLine.type[1] == LABEL) {
                insertCommand(1, 7, 7);/*Marking as Method 0, in source*/
                insertCommand(1, 4, 4);/*Marking as Method 1, in dest*/
                IC++;
                binaryWriter(currentLine,1,1,3);/*Location 1 means dest bits*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            }
        } else if (!strcmp(currentLine.cmd, "lea")) {
            if (currentLine.type[0] != LABEL) {
                fprintf(stderr, "Error line %d: Invalid first operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            if (currentLine.type[1] != REG && currentLine.type[1] != PNT && currentLine.type[1] != LABEL) {
                fprintf(stderr, "Error line %d: Invalid second operand.\n", lineCount);
                errorFlag = 1;
                return;
            }
            insertCommand(1, 2, 2);
            insertCommand(currentLine.op_code, 11, 14);
            insertCommand(1, 8, 8);

            if (currentLine.type[1] == LABEL) {
                insertCommand(1, 4, 4);
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
            } else if (currentLine.type[1] == PNT) {
                insertCommand(1, 5, 5);/*Marking as Method 2, in dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,1,2,0);/*Second operand needs to be written into dest bits*/
            } else {/*It's a register*/
                insertCommand(1, 6, 6);/*Adding Method 1, as dest*/
                IC++;
                binaryWriter(currentLine,0,1,1);/*Location doesn't affect labels*/
                binaryWriter(currentLine,1,2,0);/*Second operand needs to be written into dest bits*/
            }
        } else {
            fprintf(stderr, "Error line %d: Incorrect command type / number of arguments (Needs to be 2).\n", lineCount);
            errorFlag = 1;
            return;
        }
    }
}
/*Receives the current line, and if it's source or dest location.
 * writes the binary code according to the request.
 * Location can be 0 = Label, 1 = Dest, 2 = Source.
 * operands 1 or 2 (operands).
 * Method: 1 = Label. 2 = Register,Pointer. 3 = Immediate*/
void binaryWriter(struct lineStruct currentLine, int location, int operands, int method) {
    if (operands == 1) {
        if (currentLine.type[0] == LABEL || currentLine.type[1] == LABEL) {/*Location doesn't affect*/
            if(location == 0 && method == 1) {
                insertCommand(1, 1, 1);/*Add R flag*/
                insertCommand(0, 3, 14);/*We don't know the address yet so we add 0's*/
                insertCommand(1, 15, 15);/*FLAG*/
                IC++;
            }
        }if (currentLine.type[0] == REG || currentLine.type[0] == PNT) {
            if (location == 1 && method == 2) {/*Location is destination*/
                insertCommand(1, 2, 2);/*Adding  tag*/
                insertCommand(currentLine.op[0], 3, 5);/*Dest Register*/
                IC++;
            } else if (location == 2 && method == 2){/*Location is source*/
                insertCommand(1, 2, 2);/*Adding 'A' tag*/
                insertCommand(currentLine.op[0], 6, 8);/*Source Register*/
                IC++;
            }
        }
        if (currentLine.type[0] == IMM || currentLine.type[1] == IMM){/*Location doesn't affect*/
            if(location == 1 && method == 3) {/*Location is destination*/
                insertCommand(1, 2, 2);/*Adding A flag*/
                insertCommand(currentLine.op[0], 3, 14);/*Writing the IMM num*/
                IC++;
            }
            else if(location == 2 && method == 3){
                insertCommand(1, 2, 2);/*Adding A flag*/
                insertCommand(currentLine.op[1], 3, 14);/*Writing the IMM num*/
                IC++;
            }
        }
    }
    else{/*Opcount will be 2, made sure there can't be more in calling func*/
        if ((currentLine.type[0] == REG || currentLine.type[0] == PNT) && (currentLine.type[1] == REG || currentLine.type[1] == PNT)){
            if(location==1) {/*First var will be dest and the second source*/
                insertCommand(1, 2, 2);
                insertCommand(currentLine.op[0], 3, 5);
                insertCommand(currentLine.op[1], 6, 8);
                IC++;
            }
            else {/*Source is first, dest second*/
                insertCommand(1, 2, 2);/*Adding 'A' tag*/
                insertCommand(currentLine.op[0], 6, 8);
                insertCommand(currentLine.op[1], 3, 5);
                IC++;
            }
        }/*We need to write into dest bits the second operand.*/
        else if ((currentLine.type[0] == LABEL || currentLine.type[0] == IMM) && (currentLine.type[1] == PNT || currentLine.type[1] == REG)){
            insertCommand(1, 2, 2);/*Add A flag*/
            insertCommand(currentLine.op[1], 3, 5);/*Writing the Register into Dest bits*/
            IC++;
        }
    }
}