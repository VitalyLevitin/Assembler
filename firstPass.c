#include "header.h"
#include "external.h"

/*Passes through a given file and sends it to be parsed by each line.
 * If an error occurs keeps a list of the error and outputs them.
 * If the file analise is correct, transforms the given file into binary code
 * in the program memory.*/
void parseFile(FILE *fp1){
	char line [MAX_LINE_LENGTH];
	int ch,i,lineLength;
	ch=' ';
    errorFlag = IC = lineCount = DC = entryCount = externalCount = 0;
    labels= entry=NULL;
	while (ch!=-1) {
		lineCount++;
		while(ch==' ' || ch=='\t')
			ch = getc(fp1);
		for (i=0; ch!='\n' && ch!=-1 && i<MAX_LINE_LENGTH;i++ ){
			line[i] = (char) ch;
			ch = getc(fp1);
		}
		lineLength = i;
		line[i] ='\0';
		if(i!=0 && line[0]!=';' && line[0]!='\0' && line[0]!='\n')
			parseLine(line,lineLength);
		while (ch!='\n' && ch!=-1)
				ch = getc(fp1);
		if(ch=='\n')
			ch = getc(fp1);
	}

}
/*Sends to the parser the full line from the given file*/
void parseLine(char line[], int lineLength){
	char token[MAX_STRING_LENGTH];
	int pos=0,i;
	for (i =0; pos<lineLength && (line[pos] != '.' && line[pos] != ' ' && line[pos]!='\t' && line[pos]!=','); pos++)
	{/*Save the given line in a token*/
		if(i<MAX_STRING_LENGTH && ((i==0 && isalpha(line[pos])) || ((isalpha(line[pos]) || isdigit(line[pos])) && i!=0) ||
		    (i!=0 && pos+1<=lineLength && line[pos]==':' && (pos+1==lineLength || line[pos+1]==' '|| line[pos+1]=='\t')))){
			token[i]=line[pos];
			i++;
		}else{
			fprintf(stderr, "Error line %d: Unknown command or label.\n", lineCount);
            errorFlag=1;
		}
	}
	token[i]='\0';/*End of the token*/
	if (i==0 || (isalpha(token[0]) && token[i-1]==':')){
		if(i!=0)
			token[i-1]='\0';
        isLabel(line,token,lineLength, &pos);
		return;
	}
	if(legalCommand(token)!=-1){
        isCommand(line,token,lineLength, &pos);
		return;
	}
	fprintf(stderr, "Error line %d: Unknown command or label.\n", lineCount);
    errorFlag=1;
}