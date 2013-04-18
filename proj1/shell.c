// Kyle Jorgensen, Proj 1, CS170, 4-17-13
// partner: Alex Mousavi

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// checks if the input text is "test"
int checkExit(char* test)
{
  char* exit = "exit\n";
  return strcmp(test, exit);
}

int main()
{
  char* prompt = "sish>";
  char input[100];
  char* arg[] = {"ls", NULL};

  pid_t p;

  //printf("Original program pid: %d\n", getpid());
  printf("%s", prompt);

  // This while loop with stop if fgets returns NULL.
  // fgets only returns NULL if there is an error or an EOF
  while( fgets(input, sizeof(input), stdin) != NULL)
    {
      if(checkExit(input)==0)
	{
	  exit(0);
	}
      
      // create a child process
      p = fork();
      
      // after forking, do an exec within the child process
      if (p == 0) {
	printf("In child process:\n");
	execvp(arg[0], arg);

      } else if(p == -1){
	printf("ERROR");
      }

      // now, execution goes back to the parent process
      
      printf("%s", prompt); 
    }
  
  
  return 0;
}
