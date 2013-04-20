// Based on Alex's code

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
//GLOBALS//
char buff[100];
char * prompt = "sish>";
///////////
 
//PROTOTYPES//
int checkExit(char * s);
int createArgArray(char ** array, char * str);
int checkAmpersand(char ** array, int size);
//////////////
 
int main()
{
    //int status = 0;
    //pid_t pid;

    printf("%s",prompt);
    while(fgets(buff, sizeof(buff), stdin)!=NULL){
           
        if(checkExit(buff)==0){exit(0);}
        char * wordarray[50];//change to theoretical max num of words according to buff
        int num = createArgArray(wordarray,buff);
       
        //debug for tokens
        int i;
        printf("array size: %d\n",num);
        for (i=0;i<num;i++)
                printf("%s\n",wordarray[i]);
        //end debug
       
        //debug for ampersand
        if(checkAmpersand(wordarray,num)==0)
                printf("ampersand found\n");
        //end debug

        printf("%s",prompt);
    }
   
    return 0;
}
 
int checkExit(char * s){
        char * exit = "exit\n";
        return strcmp(s,exit);
}
 
 
int createArgArray(char ** array, char * str){
        char dividers[] = " ,\t\n";
        char * word;
        int count = 0;
        word = strtok( str, dividers );
        while( word != NULL )
    {
                array[count] = word;
                //printf("%s %i\n",word,count);
                word = strtok( NULL, dividers );
                count++;
    }
    return count;
}
 
int checkAmpersand(char ** array, int size){
        char ampersand[] = "&";
        return strcmp(array[size-1],ampersand);
}