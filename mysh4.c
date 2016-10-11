#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main(void)
{
	int selection = 1; 		//variable to determine program flow
	char *input = NULL;
	char ***vector =NULL;
	char **pre_vector = NULL;
	char *tmp;
	char *path =NULL;
	int size = 0;
	size_t buffer;			
	int i,j = 0;
	int commands = 0 , params = 0;
	int max =0;
	pid_t child;
	pid_t pid;
	int status;
	int numberofPipes;
	
	while(selection == 1)
	{
			commands = 0;
			params = 0;	
			printf("$");
			getline(&input,&buffer,stdin);

			tmp = strtok(input,"|\n");
			while(tmp!=NULL )
			{				
				pre_vector = (char**)realloc(pre_vector,(commands+1)*sizeof(char*));		//dynamic allocation of memory
				pre_vector[commands]=tmp;
				if(strlen((char *)pre_vector[commands])>max)								//determining the second dimension of the array used for storing commands
					max = strlen((char *)pre_vector[commands]);
				commands++;				
				tmp=strtok(NULL,"\n|");
			}
			pre_vector = (char**)realloc(pre_vector,(commands+1)*sizeof(char*));	
			pre_vector[commands] = NULL;

			vector = (char***)malloc((commands)*sizeof(char**));			//allocation of memory for a 2d array of char*
			for(i=0;i<commands;i++)
				vector[i] = (char**)malloc((max+1)*sizeof(char*));	

			for(i=0;i<commands;i++)									//tokenizing each command in order to separate parameters
			{				
				params = 0;
				tmp=strtok(pre_vector[i]," \t");
				while(tmp!=NULL )
				{
					vector[i][params]=tmp;
					params++;
					tmp=strtok(NULL," \t");
				}	
				vector[i][params] = NULL;								
			}		

		if(commands==0 ||vector[0][0] == NULL )	//2nd check to prevent core dumped error
		{
			//Do nothing!
			//printf("Empty command. Please try again! \n");	
		}
		else if(strcmp(vector[0][0],"exit") == 0)
		{
			//printf("Goodbye! \n");
			selection = 0;
			break;
		}
		else if (strcmp(vector[0][0],"cd") == 0 && vector[0][1]!=NULL)	//if the directory specified is null, a core dumped error is returned
		{
			int changed = 0;				
			/*First try to move to a relative path*/
			path = getcwd(path,buffer);
			path = strcat(path,"/");
			path = strcat(path,vector[0][1]);
			if(chdir(path)!=0)
				changed = 1; 
			/*If the path is not relative, try if an absolute path was entered*/
			if(changed == 1)
			{
				if(chdir(vector[0][1])!=0)
				{
					//printf("No such directory exists. Please try again \n");
				}

			}
		}							
		else
		{			
			if(commands>1)
			{
				numberofPipes = commands-1;				
				int pipes[2*numberofPipes];

				for(i=0;i<numberofPipes;i++)		//pipe creation
					pipe(pipes + i*2);

				for(i=0;i<commands;i++)
				{					
					child = fork();
					if(child < 0) 
				   		return EXIT_FAILURE;
   					else if(child == 0)
					{
						if( i != commands-1 ) 			//not last command
							dup2(pipes[2*i+1],1);
						if( i != 0 ) 					//not first command
							dup2(pipes[2*(i-1)],0);
						for(j=0;j<2*numberofPipes;j++)	//close all pipes
							close(pipes[j]);
						execvp(vector[i][0], vector[i]);
						//This code is executed only if exec() fails
						return EXIT_FAILURE;
					}
				}
				if(child > 0) 							//parent process
				{
					for(j=0;j<2*numberofPipes;j++)
						close(pipes[j]);		
				}
			}
			else if(commands == 1)			//no need for IPC 
			{
				child = fork();
				if(child < 0) 
				   return EXIT_FAILURE;
   				else if (child == 0)
   				{
	   				execvp(vector[0][0], vector[0]);
					return EXIT_FAILURE;
				}
			}
			status = 0 ;
			while ((pid = wait(&status)) > 0);	//wait for all children processes to finish		
		}
	}
	//free allocated memory
	free(tmp);
	free(path);
	free(pre_vector);
	free(vector);
	free(input);
	return 0;
}

