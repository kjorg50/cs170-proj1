// Kyle Jorgensen, Proj 1, CS170, 4-17-13
// partner: Alex Mousavi

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Global Variables */
char* prompt = "sish>";
char buff[100];

typedef enum { false, true } bool;

int checkExit(char* test);
int createArgArray(char ** array, char * str);
int checkAmpersand(char ** array, int size);
void createExecParams(char ** array, char ** result, int size);

int main()
{ 
  int child_status = 0;
  pid_t child_p;

  printf("%s", prompt);

  // This while loop will continue as long as there is valid input
  while( fgets(buff, sizeof(buff), stdin) != NULL )
    {      
      if( checkExit(buff)==0 ){ exit(0); }

      char * wordarray[50];//change to theoretical max num of words according to buff
      int num = createArgArray(wordarray,buff);

      /* debug for tokens
      int i;
      printf("array size: %d\n",num);
      for (i=0;i<num;i++)
	printf("%s\n",wordarray[i]);
	// end debug */

      /* debug for ampersand
      if(checkAmpersand(wordarray,num)==0)
	printf("ampersand found\n");
	// end debug */
  
      // get the necessary tokens for the exec()
      char* args[num+1];
      createExecParams(wordarray, args, num);

      // create a child process
      child_p = fork();
      
      // after forking, do an exec within the child process
      if (child_p == 0) {
        printf("In child process: %d\n", getpid());
	      execvp(args[0], args);
      } 
      else if(child_p <= 0){
	      printf("ERROR in child process");
      } 
      else { // it is the parent, so wait
	      wait(&child_status);
      } 
      
      printf("%s", prompt); 
    }
   
  return 0;
}

// checks if the input text is "test"
int checkExit(char* test)
{
  char* exit = "exit\n";
  return strcmp(test, exit);
}

// This function takes the 'str' string containing all the input, and then
//  parses the input into tokens, which are stored in the 'array' variable.
//  The number of tokens is returned.
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

// Checks if the last token is an ampersand
int checkAmpersand(char ** array, int size){
  char ampersand[] = "&";
  return strcmp(array[size-1],ampersand);
}

// Takes all the tokens and adds the null value to the end of the array
void createExecParams(char ** array, char ** result, int size)
{
  int i;
  for(i=0; i<size; i++){
    result[i] = array[i];
  }
  result[size] = NULL;

}

