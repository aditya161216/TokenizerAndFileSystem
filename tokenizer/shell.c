#include <stdio.h>
#include <stdbool.h>
#include "tokens.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LINE_SIZE 256

// function declarations
void parseSemiColons(char **arrayOfSplitTokens); // parse the input while checking if there are semi colons or not.
void executeCommand(char *tempArray[], int input, int output); // execute a single command, while passing in input and output.
bool isPrevThere(char* array[]); // return true if a prev is there in the command array, else return false.
void storeCommands(char* tempArray[], int tokenIndex); // store the commmands in the previous array to be executed later.
int indexOfPipe(char* tempArray[]); // return the index of the last pipe in the given array of commands. Return -1 if there are none.
int indexOfLastRedirect(char *tempArray[]); // obtain the index of the last redirect in an array of commands, return - 1 if there are none.
int redirectHelper(char* tempArray[], int specialIndex); // determine the type of redirection, return 1 if ">" and 2 if "<".
int isBuiltInCommand(char* currentCommand); // checks if the string is a built in command. Return an integer determining the type of command.
void executeBuiltInCommand(char* tempArray[],int positionOfBuiltIn); // execute a built in command
int getLength(char* array[]); // get length of an array
void printHelp(); // prints the help prompt
void executeSource(char* tempArray[]); // executes source
void executePrev(); //executes prev


// global variable declaration
char* prevArray[MAX_LINE_SIZE]; // array to store all the previous commands

int main(int argc, char **argv) {

// initialise the user input and the split tokens.
char input[MAX_LINE_SIZE];
char **splitTokens;

printf("Welcome to mini-shell.\n");

	while(1) { // infinite while loop

        	printf("shell $ "); // print the shell prompt.
	
        	if (fgets(input, MAX_LINE_SIZE, stdin) == 0) { // get the input from the user.
                	printf("\nBye bye.\n"); // prints bye bye when the user presses control d.
                	exit(0);
        	}

        splitTokens = get_tokens(input); // the splitTokens array is now split word by word, and special characters take up a spot in the array.

	parseSemiColons(splitTokens); // pass the split tokens to a helper method that will handle the semi colons
	
	}
	
	free_tokens(splitTokens);
	return 0;
}

// checks for semi colons within the command, and splits the commands up based on them
void parseSemiColons(char **arrayOfSplitTokens) { // build the array of commands, removing the semi colons, and calling a helper to execute the commands.

	// creating a temporary array to hold the commands.
	char* tempArray[getLength(arrayOfSplitTokens) + 1];

    // create an integer for traversing the array of split tokens
    int splitTokensIndex = 0;
	// creating an integer to traverse the temporary array
	int tempArrayIndex = 0;

        while (arrayOfSplitTokens[splitTokensIndex] != NULL) { // while there is a command

        	if (strcmp(arrayOfSplitTokens[splitTokensIndex], ";") != 0) { // if the element is not a semicolon
				tempArray[tempArrayIndex] = arrayOfSplitTokens[splitTokensIndex];
				tempArrayIndex++; // move on to the next spot in the temp array
				splitTokensIndex++; // move on to the next element in the token array
			} else { // if the element is a semi colon
				tempArray[tempArrayIndex] = NULL; // set the position of that semi colon to null
				arrayOfSplitTokens[tempArrayIndex] = NULL; // split tokens elem to null as well.
				tempArrayIndex = 0; // go back to the start of the temp array.
				executeCommand(tempArray, 0, 1);
				splitTokensIndex++;
			}      
        }

	tempArray[splitTokensIndex] = NULL;
    arrayOfSplitTokens[splitTokensIndex] = NULL;

	// we call store commands if prev is not there in the split commands
	if (!isPrevThere(arrayOfSplitTokens)) {
		storeCommands(arrayOfSplitTokens, splitTokensIndex); // store the commands in the prev array.
	}

	executeCommand(tempArray, 0, 1);
}

void executeCommand(char *tempArray[], int input, int output) {

	// close the previous input output and replace it 

	int pipeIndex = indexOfPipe(tempArray);

	if (pipeIndex != -1) { // if there is a pipe in the temp commands, we now rerun this code, to check if there is another pipe, while passing the input output along.
		int pipe_fds[2]; // creates two file descriptors (read and write end)
		pipe(pipe_fds); // returns 0 on success

   		int read_fd = pipe_fds[0]; // index 0 is the "read end" of the pipe
   		int write_fd = pipe_fds[1]; // index 1 is the "write end" of the pipe

		int inputCopy = dup(input);
		int outputCopy = dup(output);

		// tempArray[pipeIndex] = NULL; // now we have to set the index of the pipe to null, 
		//char* fileName = tempArray[pipeIndex];
		char *leftCommands[getLength(tempArray)];
		// int tempIndex = 0;

		// we have the location of the last pipe, everything before and everything after it
		// ideally, we should close the input here, and make it take the input of the recursive execute command, of the command the left of the pipe
		int i;
		for (i = 0; i < pipeIndex; i++) {
			leftCommands[i] = tempArray[i];
		}

		leftCommands[i] = NULL;

		char *rightCommands[getLength(tempArray)];
		int idx = 0;
		int j;
		for (j = pipeIndex + 1; j < getLength(tempArray); j++) {
			rightCommands[idx] = tempArray[j];
			++idx;
		}

		rightCommands[idx] = NULL;

		close(output);
		dup2(write_fd,1);
		
		executeCommand(leftCommands, input, write_fd); // child

		close(write_fd);
		dup2(outputCopy,1);

		dup2(read_fd, 0);
		close(input);

		executeCommand(rightCommands, read_fd, outputCopy); // parent

		close(read_fd);
		dup2(inputCopy,0);
		
		close(inputCopy);
		close(outputCopy);

		return;
	} 

	else if (strcmp(tempArray[getLength(tempArray) - 2], ">") == 0 || strcmp(tempArray[getLength(tempArray) - 2], "<") == 0) { // if there exists a redirect in the code.
		char* special_char = tempArray[getLength(tempArray) - 2];
		tempArray[getLength(tempArray) - 2] = NULL; // set the index of the special character to null, so when we re-execute executeCommand, it won't read this special char.
		int fileDescriptor; 
		int outputCopy;
		int inputCopy;
		if (strcmp(special_char, ">") == 0) {
			outputCopy = dup(output);
			fileDescriptor = open(tempArray[getLength(tempArray) + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			dup2(fileDescriptor, 1);
			close(output);
			executeCommand(tempArray, input, fileDescriptor);
			dup2(outputCopy, 1);
			close(outputCopy);
			return;
		}
		if (strcmp(special_char, "<") == 0) {
			inputCopy = dup(input);
			fileDescriptor = open(tempArray[getLength(tempArray) + 1], O_RDONLY);
			dup2(fileDescriptor, 0);
			close(input);
			executeCommand(tempArray, fileDescriptor, output);
			dup2(inputCopy, 0);
			close(inputCopy);
			return;
		}
		
	}

	else { // finally we fork and execute the command. 


		// checking for built in commands
		int positionOfBuiltIn = isBuiltInCommand(tempArray[0]);
		
			if (positionOfBuiltIn != -1) { // if the command is a built in command
				executeBuiltInCommand(tempArray, positionOfBuiltIn); // execute the built in command
				return;
			} else {  // if its not a built in command

				if (fork() == 0) { // fork and execute
					if (input != 0) {
						dup2(input,0);
						close(input);
					}

					if (output != 1) {
						dup2(output,1);
						close(output);
					}

					execvp(tempArray[0], tempArray);
					printf(strcat(tempArray[0], ": command not found1\n")); // if the execvp does not work print an error
					exit(1); // and exit with a status of 1.
				}
				wait(NULL); // wait for the child to finish running.
				return;
		}
	}
}

int getLength(char* array[]){
	int count = 0;
	while(array[count] != NULL){
		count++;
	}

	return count;
}

// checks if 'prev' is a command in the list of commands the user has entered
bool isPrevThere(char* array[]) {

	int i = 0;
	for (i = 0; i < 256; i++) { // loop through the entire array of commands
		if(array[i] != NULL) {
			if (strcmp(array[i], "prev") == 0) { // if a single element is prev
				return true; 
			}
		}

	}	
	return false; // if no elements are prev
}

// stores previous commands in the 'prev' array, so that it can be called upon if the user enters 'prev' later
void storeCommands(char* tempArray[], int tokenIndex) {
		// need to store loop through the temp array and store all the commands in the prev array.
		int traversalInteger = 0;

		for (traversalInteger = 0; traversalInteger < tokenIndex; traversalInteger++) { // for each comand we are before the last command 
			prevArray[traversalInteger] = tempArray[traversalInteger]; // store the command in the the prev array.
		}
	}

// returns the index of "|" in the list of commands the user has given
int indexOfPipe(char* tempArray[]) { // checking if "|" exists in the command and returning their index
	int index = 0;
	int indexOfLastPipe = -1;
	
	while (tempArray[index] != NULL) { // while the element is not null
		if (strcmp(tempArray[index], "|") == 0) { // and the element is a pipe
			// if (index > indexOfLastPipe){ // check if the index of this pipe is greater then the previous pipe
			// 	indexOfLastPipe = index; // store it if it is.
			// }
			indexOfLastPipe = index; // store it if it is.
		}
		index++;  // increment the index and move on.	
	}

	return indexOfLastPipe; // return the index of the last pipe in the array of commands, -1 if there is none.
}

int indexOfLastRedirect(char *tempArray[]) { // return the index of the last redirect in the code.
	int index = 0;
	int indexOfLastR = -1;
	
	while (tempArray[index] != NULL) { // while the element is not null.
		if (strcmp(tempArray[index], ">") == 0 || strcmp(tempArray[index], "<") == 0) { // if it is > or <.
			if (index > indexOfLastR){ // if the index of this redirect is greater than the index of the previous redirect,
				indexOfLastR = index; // store it.
			}
		}
	index++; // move on to the next element.
	}

	return indexOfLastR; // return the index of the last redirect.
}

// checks whether the user wants to redirect input or output, and accordingly executes the required function.
int redirectHelper(char* tempArray[], int specialIndex) { // return the type of redirect

	if (strcmp(tempArray[specialIndex], ">") == 0) { // if it is output redirection return one.
		return 1;
	} else { // return 2 for input redirection.
		return 2;
	}
}

// checks if the user has entered a built in command, returns -1 if not found
int isBuiltInCommand(char* currentCommand) {
	// create an array holding all the built in commands
	char* builtInCommandsArray[] = {"exit","cd","help","source","prev","\0"};

	// create an integer to loop through the list of the built in commands.
	int builtInIndex = 0;

	// create an integer representing the max size of the built in commands
	int numBuiltIns = sizeof(builtInCommandsArray) / sizeof(builtInCommandsArray[0]); 
 
	// for loop traversing through the built in commands 
	for (builtInIndex = 0; builtInIndex < numBuiltIns; builtInIndex++) {
		if (strcmp(currentCommand, builtInCommandsArray[builtInIndex]) == 0) {
			return builtInIndex;
		}
	}

	return -1;
}

// executes the built in command based on switch statements
void executeBuiltInCommand(char* tempArray[],int positionOfBuiltIn) {

	switch(positionOfBuiltIn) {
		case 0:
			printf("Bye bye.\n");
			exit(0);
			break;
		case 1:
			if (tempArray[1] != NULL) {
		                if(chdir(tempArray[1]) !=0){
					perror("unable to change the directory");
				}
                        }
			else { 
				if(chdir(getenv("HOME")) != 0) {
					perror("unable to change directory");
				}
                	}	

			break;
		case 2: 
			printHelp();
			break;
		case 3: 
			executeSource(tempArray);
			break;
		case 4:
			executePrev();
			break;

	}
}


// prints the 'help' message
void printHelp() {
	printf("\n<<LISTING ALL COMMANDS WITHIN THE SHELL>>"
		"\nexit -> exits the shell"
		"\ncd -> changes directory based on the argument given"
		"\nhelp -> print this help message"
		"\nsource -> executes a given script"
		"\nprev -> executes the previous command line given by the user\n");	
}


// executes the built in command 'source'
void executeSource(char* tempArray[]){
	char *tokens[4];
	tokens[0] = "./shell";
	tokens[1] = "<";
	tokens[2] = tempArray[1];
	tokens[3] = NULL;
	executeCommand(tokens, 0, 1);
}


// executes the commands in the prev array if the user enters 'prev'
void executePrev(){

	int lengthOfPrevArray = sizeof(prevArray) / sizeof(prevArray[0]);

	int index = 0;

        // creating a temporary array to hold the commands.
        char* tempArray[MAX_LINE_SIZE];
  
        // create an integer for traversing the array of split tokens
        int splitTokensIndex = 0;
        // creating an integer to traverse the temporary array
        int tempArrayIndex = 0;

	while (index < lengthOfPrevArray && (prevArray[splitTokensIndex] != NULL || prevArray[splitTokensIndex + 1] != NULL)) {
  
        	if (prevArray[splitTokensIndex] != NULL) { // if the element is not a semicolon
                	tempArray[tempArrayIndex] = prevArray[splitTokensIndex];
                	tempArrayIndex++; // move on to the next spot in the temp array
               		splitTokensIndex++; // move on to the next element in the token array
               } else { // if the element is null
                        tempArrayIndex = 0; // go back to the start of the temp array.
                        executeCommand(tempArray, 0, 1);
                        splitTokensIndex++;
                }
	index++;
        }

	for (int i = 0; i < 256; i++) {
		prevArray[i] = NULL;
	}

        executeCommand(tempArray, 0, 1);
}