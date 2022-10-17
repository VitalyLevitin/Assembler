#include "header.h"
#include "external.h"

/*Updates all the known labels to their proper addresses, using the information
 * gained from the first passage. if an error occurs outputs it to stderr.*/

void labelUpdate(FILE *fp1,FILE *fp2){
	int i,j,m,k,flag;
	for(i=0 ; i<DC ; i++){
		for(j=i ; j<DC ;j++){
			if(i!=j && !strcmp(labels[i].name, labels[j].name)){/*if the same label is encountered*/
				fprintf(stderr, "%s Same label declaration.\n", labels[i].name);
                errorFlag=1;
			}
		}
	}
	i=IC;
	for(IC=0;IC<i;IC++){
		if(memory[IC].word[15] == 1){/*If flag is active*/
			for(flag=0,j=0;j<DC && flag==0; j++){
				if(strcmp(memory[IC].label, labels[j].name) == 0){
					flag=1;/*Found the label*/
					if(labels[j].d_type == EXTERN){
						fprintf(fp1, "%s", labels[j].name);
						for(m=0;m<MAX_STRING_LENGTH-strlen(labels[j].name); m++)
							fputc(' ',fp1);
						fprintf(fp1,"  %d",IC+100);
						fputc('\n',fp1);
						memory[IC].word[0]=1;/*Bit 0 == External bit*/
						memory[IC].word[1]=0;/*Removing the active flag on bit 1 (relocate)*/
					}
					else{/*It's Relocatable (because flag is active it can only be ex/en)*/
                        insertCommand(labels[j].address, 3, 14);
						memory[IC].word[1]=1;
					}
				}	
			}
			if(!flag){/*We mark the flag as 1 if we match the label*/
					fprintf(stderr,"Label %s not found.\n",memory[IC].label);
                errorFlag=1;
					return;
			}
		}
	}
	for(k=flag=0;fp2!=NULL && k<entryCount;k++){
		for(j=0;j<DC ;j++){
							if(strcmp(labels[j].name, entry[k].name) == 0){
								flag++;
								fprintf(fp2,"%s",entry[k].name);
								for(m=0;m<MAX_STRING_LENGTH-strlen(entry[k].name);m++)
									fputc(' ',fp2);
								fprintf(fp2, "  %d", labels[j].address);
								fputc('\n',fp2);
							}
						}
					}
					if(flag!=entryCount){
					fprintf(stderr,"Label %s not found.\n",memory[IC].label);
                        errorFlag=1;
					return;
			}
	IC=i;
}

/*Transfers the newly created binary code to octal based code and writes it to the file*/
void toOctal(FILE *fp){
	int i,j;
	int value;
    fprintf(fp, "   %d  %d\n", IC-DC-1,DC+1);
	for(i=0;i<IC;i++) {
        fprintf(fp, "0%d  ", i + 100);
        for (j= 14; j >= 0; j-=3) {
            value = 0;
            if(memory[i].word[j] == 1)
                value+=4;
            if(memory[i].word[j - 1] == 1)
                value+=2;
            if(memory[i].word[j - 2] == 1)
                value+=1;
            fprintf(fp, "%d",value);
        }
        fputc('\n',fp);
    }
}
