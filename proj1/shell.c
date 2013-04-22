// Kyle Jorgensen and Alex Mousavi, Proj 1, CS170, 4-17-13


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Global Variables */
char* prompt = "sish>";
char buff[1024];

//for args
char ** commands;
//

//prototypes//
int checkExit(char* test);
int createArgArray(char ** array, char * str);
void createExecParams(char ** array, char ** result, int size);
int countPipes(char * str);
int setCommands(char ** array,char * str);
void splitCommand(char * head,int * out, int * in);
int verifyCommand(char * str);
void trim(char * str);
int verifyAmpersand(char * str);
int checkCd(char * array);
int cd(char * arg, int count);
//////////////

int main()
{ 

  printf("%s", prompt);

  // This while loop will continue as long as there is valid input
  while( fgets(buff, sizeof(buff), stdin) != NULL )
    {      
    	if( checkExit(buff)==0 ){ exit(0); }

		//check for ampersand		
		int amp = verifyAmpersand(buff);
		if(amp==-1){
			printf("ERROR: exec failed\n");
			printf("%s", prompt);
			continue; // stop this iteration of the while loop and continue
		}
			
		//gets count of pipes
		int pipeCount = countPipes(buff);
		
		//create of an array of commands separated between the pipes
		int cmdCount = pipeCount+1;
		char * commands[cmdCount];
		if(setCommands(commands,buff) != cmdCount){
			printf("ERROR: exec failed\n");
			printf("%s", prompt);
			continue; // stop this iteration of the while loop and continue

			//this isn't just a check, setCommands sets the 'commands' array
		}
    	
    	char * in = NULL;
    	char * out = NULL;
    	int fileFirst = 0;
    	int fileLast = 0;
    	int valid = 1;

        // Now loop through all the commands of the input
    	int i;
    	for(i=0;i<cmdCount;i++){
    		
    		int verify = verifyCommand(commands[i]);
            // if a command is not valid, then break out of the loop
    		if(verify!=1){
    			valid = 0;
                break;
    		}
    		
    		char * mainCommand = commands[i];
			char * outFile = NULL;
			char * inFile = NULL;
			
			// determine input/output file positions 
			int outDistance = -1;
			int inDistance = -1;
			splitCommand(mainCommand,&outDistance,&inDistance);
            // if there is an output file
			if(outDistance != -1){
				outFile = &mainCommand[outDistance];
				trim(outFile);
			}
            // if there is an input file
			if(inDistance != -1){
				inFile = &mainCommand[inDistance];
				trim(inFile);
			}
			
			// if the input command is not the first command, 
			// or if the output command is not the last command,
			// then it is NOT valid
			if(inFile!= NULL && i!=0){
				valid=0;
			} else if(inFile != NULL) {
				in = inFile;
			}
			if(outFile != NULL && i!=cmdCount-1){
				valid=0;
			} else if(outFile != NULL) {
				out = outFile;
			}
			
			
    	}// end for loop
    	
    	
    	if(valid==0){
    		printf("Invalid command.\n");
    		printf("%s", prompt);
    		continue;
    	}
    	
    	
    	// now create a new command array

    	char ** cmdPtr = NULL;
    	
    	int addIn = (in==NULL) ? 0 : 1;
    	int addOut = (out==NULL) ? 0 : 1;
    	int newCmdCount = cmdCount + addIn + addOut;
    	char * newCommands[newCmdCount];
    	
    	fileFirst = addIn;
    	fileLast = addOut;
    	
		// if there are input or output files
    	if(newCmdCount > cmdCount){
    		int k=0;
    		int start;
    		if(addIn){
    			newCommands[0] = in;
    			start=1;
    		} else {
    			start=0;
    		}
    		
			// adjust the newCommands array to accomodate the output file
    		if(addOut){
    			newCommands[newCmdCount-1] = out;
    		}
    		
			// adjust the newCommands array to accomodate the input file
    		for(k=0;k<cmdCount;k++){
    			newCommands[k+start] = commands[k];
    			
    		}
    		
    		cmdCount = newCmdCount;
    		cmdPtr = newCommands;
    		//printf("I'm still alive. 2\n");
    	} else {
    		cmdPtr = commands;
    	}
    	
    	
    	/////////////////
    	//intialize pipes
    	int read[cmdCount];
    	int write[cmdCount];
    	int j,k;
    	//initialize with marker
    	for(k=0;k<cmdCount;k++){
    		write[k]=-1;read[k]=-1;
    	}
		for(j=0;j<cmdCount-1;j++){
			int f[2];
			pipe(f);
			read[j+1]=f[0];
			write[j]=f[1];
		}
		// Go through and execute the commands with proper forks, pipes, etc. 
    	for(i=0;i<cmdCount;i++){
    		
    		//initialize array argument
    		char * command[1024];
    		
			// returns the number of tokens in the command and sets 
			// the command array with those tokens
			int cmdSize =  createArgArray(command,cmdPtr[i]);
			char * args[cmdCount+1];
    		createExecParams(command,args,cmdSize); 
    		
    		//initialize needed vars for forking
    		int child_status = 0;
  			pid_t child_p = fork();
    		
    		if(child_p < 0){
    			printf("ERROR in child process");
    			exit(1);
    		} else if (child_p==0){
    
    			//in child, so do stuff
    			

    			//write if not the last file
    			if(write[i]!=-1){
					close(1);
					dup2(write[i],1);
				} 
				
				
				//read if not the first file
				if(read[i]!=-1){
					close(0);
					dup2(read[i],0);
				}
				
				
				if(i==0&&fileFirst==1){
					//read from input file and print it (to pipe input)
					// i.e. command < input.txt
					
					char buffer[300];
					//check length of args here?
					FILE * file;
					file = fopen(args[0],"r");
					if(file==NULL){
						fclose(file);
						printf("File not found");
						exit(1);
					}
					
					// fgets gets the input from a file
					while( fgets(buffer, sizeof(buffer), file) != NULL )
					{
						printf(buffer);
					}
					fclose(file);
					exit(0);
					
				} else if(i==cmdCount-1&&fileLast==1){
					//read from input and output to file
					// i.e. command > file.txt

					FILE * file;
					file = fopen(args[0],"w");
					if(file!=NULL){
						char buffer[300];

						// fgets gets input from stdin and writes to file
						while( fgets(buffer, sizeof(buffer), stdin) != NULL )
						{
							fprintf(file,buffer);
						}
					}
					fclose(file);
					exit(0);
					
				} 
				else {
					//run the execute
					
					// if it is a cd command
					if(checkCd(args[0])){
						
						if(cmdSize==1){
							cd(NULL,1);
						} else {
							cd(args[1],cmdSize);
						}
					} else { // it is a normal command
					
						int status = execvp(args[0], args);
						if(status < 0){
							printf("ERROR: exec failed\n");
							exit(1);
						}
					}
				}
    		} else {
    			//in parent, wait for child if there is no ampersand 
    			if(amp==0)wait(&child_status);
				close(write[i]);
				if(i > 0){
					close(read[i]);
				}
    		}
    	} // end for loop
    	
    	printf("%s", prompt);
    	
    	
    }// end while loop 
   
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

// Takes all the tokens and adds the null value to the end of the array
void createExecParams(char ** array, char ** result, int size)
{
  int i;
  for(i=0; i<size; i++){
    result[i] = array[i];
  }
  result[size] = NULL;

}

// returns the number of '|' characters in the input
int countPipes(char * str){
	int count = 0;
	int i=0;
	while(str[i]!='\0'){
		char c = str[i];
		if(c=='|'){
			count++;
		}
		i++;
	}
	return count;
}

// This does basically the same thing as the createArgArray() function.
// It just divides the commands based on the pipe character, and then
// returns the number of commands.
int setCommands(char ** array,char * str){
	char dividers[] = "|";
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

// Verifies the section between the pipes.
// State machine to parse commands.
// Returns 1 if the command is valid, otherwise it returns 0
int verifyCommand(char * str){
	int inFound = 0;
	int outFound = 0;
	int state = 0;
	int i=0;
	while(1){
		char c = str[i];
		//printf("%c",c);
		if(state==0){
			//printf("0");
			if(c=='<'||c=='>')
				return 0;
			else if(c!=' ')
				state=1;
			
		} else if(state==1){
			//printf("1");
			if(c=='\0'||c=='\n')
				return 1;
			else if(c=='>'){
				outFound = 1;
				state = 2;
			} else if(c=='<'){
				inFound = 1;
				state = 2;
			}
			
		} else if(state==2){
			//printf("2");
			if(c=='<'||c=='>'||c=='\0'||c=='\n')
				return 0;
			else if(c!=' ')
				state=3;
			
		} else if(state==3){
			//printf("3");
			if(c=='\0'||c=='\n')
				return 1;
			else if(c==' ')
				state=4;
			else if(c=='>'){
				if(outFound==1)return 0;
				outFound=1;
				state=2;
			} else if(c=='<'){
				if(inFound==1)return 0;
				inFound=1;
				state=2;
			}
			
		} else if(state==4){
			//printf("4");
			if(c=='\0'||c=='\n')
				return 1;
			else if(c=='>'){
				if(outFound==1)return 0;
				outFound=1;
				state=2;
			} else if(c=='<'){
				if(inFound==1)return 0;
				inFound=1;
				state=2;
			} else if(c!=' ')
				return 0;
		}
		
		i++;
	}
	
	return 0;
	
}


// Splits the command between < and >
// Sets the correct indices for *in and *out which are the input/output filenames 
void splitCommand(char * head,int * out, int * in){
	int i=0;
	while(head[i]!='\0'){
		char c = head[i];
		if(c == '>'){
			head[i]='\0';
			i++;
			while(head[i]==' ')i++;
			*out = i;
		} else if( c == '<' ){
			head[i]='\0';
			i++;
			while(head[i]==' ')i++;
			*in = i;
		}
		i++;
	}

}

//replace whitespace with \0 after input filenames or output filenames
void trim(char * str){
	int i=0;
	while(str[i]!='\0'){
		if(str[i]==' '||str[i]=='\n')
			str[i]='\0';
		i++;
	}
}

// Checks if the last character is an ampersand
// Returns 1 if true, 0 if false, or -1 if there is an ampersand syntax error
int verifyAmpersand(char * str){
	int found = 0;
	int i=0;
	while(str[i] != '\0'){
		char c = str[i];
		if(found){
			
			// if ampersand found and there is whitespace after it, remove the whitespace
			if(c == ' '||c == '\n'||c=='\t'){
				str[i] = '\0';
			} else { // otherwise return an error
				return -1;
			}
		} else if(c == '&'){
			str[i] = '\0';
			found = 1;
		}
		i++;
	}
	return found;
}

// Returns 1 if string is cd
int checkCd(char * array){
	char cd[] = "cd";
	if(strcmp(array,cd)==0){
		return 1;
	} else {
		return 0;
	}
}

// takes a cd command and changes to the correct directory
int cd(char * arg, int count){
	char * str = arg;
	if(count == 1){
		chdir(getenv("HOME"));
	} else {
		if(str[0]=='~'){
			char * dir = strcat(getenv("HOME"),&str[1]);
			printf("%s\n",dir);
			chdir(dir);
			dir = NULL;
		}
		chdir(str);
	} 
	return 0;
}












