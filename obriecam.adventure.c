#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>



//		Struct			//

struct Room{
	char * name;
	int num_connections;
	char ** connections;
       	char * type;	
};



//		Mutex Declaration		//

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;



//		Function Headers		//

void getDirName(char [] );
void getRoomInfo(struct Room * );
void initGraph(struct Room * );
void runGame(struct Room * , int , int, char**);
int roomPlace(struct Room * , char * );
int startPlace(struct Room * );
int checkInput(struct Room , char * );
void * threadTime();
void timeCalled();



//		Function Bodies			//

/*************************************
 * Function: threadTime 
 * Parameters: none
 * Description: retrieves the local time and writes it to a file in desired format
 * Pre-Conditions: user entered time when asked for a room choice
 * Post-Conditions: time is written to the time file
**************************************/
void * threadTime(){

	// lock the mutex with this thread
	pthread_mutex_lock(&myMutex);

	// file pointers and strings containing path
	FILE * fp;
	char file [] = "currentTime.txt";

	// create or overwrite the file
	fp = fopen(file, "w+");

	// time variables and structs
	time_t rawtime;
	struct tm * info;
	char buffer[80];

	// time commands
	time(&rawtime);
	info = localtime(&rawtime);

	// arrange the time information in the desired format
	strftime(buffer, 80, "%l:%M%p, %A, %B %e, %Y", info);

	// print the time information into the file
	fprintf(fp, " %s\n", buffer);

	// close the file stream
	fclose(fp);

	// unlock the mutex
	pthread_mutex_unlock(&myMutex);

	// return null for the join function to allow further program progress
	return NULL;
}



/*************************************
 * Function: timeCalled()
 * Parameters: N/A
 * Description: unlocks mutex, called thread time function, then locks the mutex and prints the time from the
 * 		file
 * Pre-Conditions: user enters "time" when asked which room to go to
 * Post-Conditions: time is printed to screen
**************************************/
void timeCalled(){

	// create thread and result code variables
	pthread_t thread;
	int result_code;

	// unlock the mutex
	pthread_mutex_unlock(&myMutex);

	// create the thread
	pthread_create(&thread, NULL, threadTime, NULL);

	// ensure the thread terminates before continuing
	result_code = pthread_join(thread, NULL);
	
	// relock the mutex
	pthread_mutex_lock(&myMutex);

	// create file pointer for time file
	FILE * fp;
	char c;
	fp = fopen("currentTime.txt", "r");

	// print the contents of the file
	printf("\n");
	while((c=fgetc(fp))!=EOF){
		printf("%c", c);
	}
	printf("\n");
	
	// close the file stream
	fclose(fp);	
}



/*************************************
 * Function: getDirName
 * Parameters: Empty string to hold the newest directory with the desired prefix
 * Description: looks for the newest directory starting with obriecam.rooms.
 * Pre-Conditions: string is empty
 * Post-Conditions: string contains the newest directory to be used in later functions
**************************************/
void getDirName(char * newestDirName){
	
	// timestamp of newest subdirectory examined
	int newestDirTime = -1;
	
	// prefix to look for in the dirsectory
	char targetDirPrefix[32] = "obriecam.rooms.";
	memset(newestDirName, '\0', sizeof(newestDirName));

	// directory pointer and structs
	DIR * dirToCheck;
	struct dirent * fileInDir;
	struct stat dirAttributes;
	
	// open directory that this program was run in
	dirToCheck = opendir(".");

	// ensure that the directory can be opened
	if(dirToCheck > 0){
		
		// check every entry in the directory
		while((fileInDir = readdir(dirToCheck)) != NULL){
			
			// if the entry in the directory has the desired prefix
			if(strstr(fileInDir->d_name, targetDirPrefix) != NULL){		
				
				// get the attributes of the entry
				stat(fileInDir->d_name, &dirAttributes);
			
				// if the time is bigger
				if((int)dirAttributes.st_mtime > newestDirTime){
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}	
			}
		}
	}
	
	// close the directory we are in
	closedir(dirToCheck);
}



/*************************************
 * Function: getRoomInfo
 * Parameters: array of rooms to be filled with the contents of the rooms directory
 * Description: opens the newest room directory, reads through all files in the directory then assigns the values
 * 		of the files to their respective spots in the array of rooms.
 * Pre-Conditions: graph of rooms is empty
 * Post-Conditions: graph of rooms contains all graph information (names, connections, num connections, room type).
**************************************/
void getRoomInfo(struct Room * graph){
	
	// the directory we are in and the subdir of the starting dir
	DIR * dirToCheck;
	struct dirent * fileInDir;
	
	// string for the newsest directory's name
	char newestDirName[256];

	// retrieve the newest directory name
	getDirName(newestDirName);


	// open the newest directory
	dirToCheck = opendir(newestDirName);

	// string to hold file path for files in the newest directory
	char filePath[256];

	// psuedo for loop for iterating through the array of rooms for filling in struct
	int i=0;

	// go through the entire directory
	while((fileInDir = readdir(dirToCheck)) != NULL){
		
		// copy the newest directory name into file path
		strcpy(filePath, newestDirName);
		
		// append '/' to the file path
		strcat(filePath, "/");

		// avoid . and .. files
		if( !( fileInDir->d_name[0] =='.')){
			
			// append the current file name to the file path
			strcat(filePath, fileInDir->d_name);

			// file pointer
			FILE *fp;
			size_t len = 0;
			ssize_t read;
			
			// string for line of the file
			char * line = NULL;

			// open the current file with read permissions
			fp = fopen(filePath, "r");
	
			// go through each line in the file
			while((read = getline(&line, &len, fp)) != -1){

				// create two "part" strings to hold things like ROOM NAME: and CONNECTION 1:
				char part1[20], part2[20], info[20];
				
				// scan in the part and info variables
				sscanf(line, "%s %s %s", &part1, &part2, &info);
				
				// if part1 is connection
				if(!(strcmp(part1, "CONNECTION"))){
					
					// copy the connection name to the struct
					strcpy(graph[i].connections[graph[i].num_connections], info);
					
					// increment the number of connections this room has
					graph[i].num_connections++;
				}
				else{
					// if the second part is "TYPE"
					if(!(strcmp(part2, "TYPE:"))){
						
						// copy info variable to type variable
						strcpy(graph[i].type, info);
					}
					// must be the room name
					else{
						// copy info variable to name variable
						strcpy(graph[i].name, info);	
					}
				}
			}
			// increment to the next spot in the graph
			i++;

			// close the file stream
			fclose(fp);
		}
	}
	
	// close the directory
	closedir(dirToCheck);
}



/*************************************
 * Function: initGraph
 * Parameters: uninitialized array of rooms
 * Description: initializes the graph, allocating memory for names, types, and connections
 * Pre-Conditions: graph is only declared
 * Post-Conditions: graph is inialized and ready to be filled
**************************************/
void initGraph(struct Room * graph){
	int i, j;

	// allocate memory for name, type, connections array, and set the number of connections to 0
	for(i=0;i<7;i++){
		graph[i].name = calloc(16, sizeof(char));
		graph[i].type = calloc(16, sizeof(char));
		graph[i].connections = calloc(6, sizeof(char *));
		graph[i].num_connections = 0;
		
		// allocate memory for the names of each connection in the connections array
		for(j=0;j<6;j++){
			graph[i].connections[j] = calloc(16, sizeof(char));
		}
	}
}



/*************************************
 * Function: startPlace
 * Parameters: array of rooms
 * Description: looks through the array of rooms for the start room, when the start room is found the iteration of the room is returned
 * 		this allows the program to know which room to start in.
 * Pre-Conditions: graph has been filled with information
 * Post-Conditions: starting room is identified and the game is ready to start
**************************************/
int startPlace(struct Room * graph){
	int i;
	
	// find the room of start type
	for(i=0;i<7;i++){
		if(!strcmp(graph[i].type, "START_ROOM")){

			// return the spot in the graph of the start room
			return i;
		}
	}
}



/*************************************
 * Function: roomPlace
 * Parameters: array of rooms and name of room to look for
 * Description: Finds the spot in the array of the room name that is passed in and returns to function call
 * Pre-Conditions: the user has put in a valid connection and the program must "move" to that room
 * Post-Conditions: the program knows where to go in the array to "move" to the desired room
**************************************/
int roomPlace(struct Room * graph, char * name){
	int i;

	// find the room with the corresponding name as the one passed in
	for(i=0;i<7;i++){
		if(!strcmp(graph[i].name, name)){
		
			// return the spot in the graph of the name passed in
			return i;			
		}
	}
}



/*************************************
 * Function: checkInput
 * Parameters: single room, input given by user when prompted to pick a room
 * Description: looks through the room's connections and returns 0 if a match is found, otherwise returns 1
 * Pre-Conditions: user gives input for moving rooms
 * Post-Conditions: input is either validated or refuted
**************************************/
int checkInput(struct Room room, char input[]){
	int i;

	// if the player put in time
	if(!strcmp("time", input)){
		timeCalled();
		
		// returns 2 to tell the runGame function that there was not an error, but do not
		// reprompt with current location and connections, only as where to
		return 2;
	}

	// iterate through the connections of the room
	for(i=0;i<room.num_connections;i++){
		
		// if a match is found
		if(strcmp(room.connections[i], input) == 0){
			
			// tells the runGame function that a valid room was found
			return 0;
		}
	}

	// a match was not found in the connections array, error message and return error
	printf("\nHUH, I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	return 1;

}



/*************************************
 * Function: runGame 
 * Parameters: array of rooms, current place in the array, current place in the path array, path array
 * Description: prompts the user for inpur for moving rooms, validates input, copies the next room to the path array
 * 		increments the path array iterator, checks if the next room is the end, if it is ends the program,
 * 		otherwise calls itself to be run again with the next room as a parameter
 * Pre-Conditions: game is either begun or is continued
 * Post-Conditions: game is either continued or ended based on player performance
**************************************/
void runGame(struct Room * graph, int place, int pathPlace, char **gamePath){
	char nextLocation[16];
	
	// i for iteration, repeat for reprompting, and timeBool for only prompting where to after time is called
	int i, repeat = 1, timeBool = 0;


	// loops while either time is called or no match is found in connections for room choice
	do{
		// if the time bool is false, prompt with location and connections
		if(!timeBool){
			printf("CURRENT LOCATION: %s\n", graph[place].name);	
			printf("POSSIBLE CONNECTIONS: ");
			for(i=0;i<graph[place].num_connections;i++){
				if(i== (graph[place].num_connections-1)){
					printf("%s.\n", graph[place].connections[i]);
				}
				else{
					printf("%s, ", graph[place].connections[i]); 
				}
			}
		}
		
		// variables for holding input
		char * input;
		size_t buffsize = 32;
		
		// allocate memory for input
		input = (char *)malloc(buffsize * sizeof(char));

		// prompt where to?
		printf("WHERE TO? >");

		// get the line of input
		getline(&input, &buffsize, stdin);
		
		// remove the newline char at the end of the input
		input[strcspn(input, "\n")] = 0;

		// hold the result of the check input function
		int inputResult = checkInput(graph[place], input);

		// if input result is 0
		if(!inputResult){
			
			// copy the input variable to the next location variable
			strcpy(nextLocation, input);
			
			// end the while loop
			repeat = 0;
		}

		// if the input result returned 2 then set the time boolean to true
		if(inputResult == 2){
			timeBool = 1;
		}
		else{
			// time was not called, and no match was found in checkInput
			timeBool = 0;
		}
		
	}while(repeat);

	printf("\n");

	// copy the next location to the game path
	strcpy(gamePath[pathPlace], nextLocation);
	
	// increment the spot in the path
	pathPlace++;

	// if the next room is the end room
	if(!strcmp(graph[roomPlace(graph, nextLocation)].type, "END_ROOM")){
		
		// print winning message with number of steps taken
		printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
		printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", pathPlace);
		
		// print the path taken by the player
		for(i=0;i<pathPlace;i++){
			printf("%s\n", gamePath[i]);
		}
	}
	else{

		// run the game with graph, the place of the next location, spot in the game path, and the game path
		runGame(graph, roomPlace(graph, nextLocation), pathPlace, gamePath);
	}

}



/*************************
 * Function: freeGraph
 * Parameters: array of rooms 
 * Description: frees up allocated memory for the graph
 * Pre-Conditions: graph has been used and adventure has been completed
 * Post-Conditions: graph's memory has been free'd 
*************************/
void freeGraph(struct Room * graph){
	int i, j;

	// free memory for name, type, and, connections in the array and the connections array itself 
	for(i=0;i<7;i++){
		free(graph[i].name);
		free(graph[i].type);
		for(j=0;j<graph[i].num_connections;j++){
			free(graph[i].connections[j]);
		}
		free(graph[i].connections);
	}
}



//		Main		//

int main(){

	// lock the mutex
	pthread_mutex_lock(&myMutex);
	
	// declare the graph and game path variables
	struct Room * graph;
	char **gamePath;

	// allocate memory to the game path array 
	gamePath = calloc(50, sizeof(char *));
	
	int i;
	
	// allocate memory for each spot in the game path array
	for(i=0;i<50;i++){
		gamePath[i] = calloc(16, sizeof(char));
	}

	// allocate memory for the graph array
	graph = calloc(7, sizeof(struct Room));
	
	// initialize the graph to be ready for information
	initGraph(graph);

	// retrieve information for the graph
	getRoomInfo(graph);

	// run the game with appropriate parameters
	runGame(graph, startPlace(graph), 0, gamePath);


	// Free memory for each string in the array
	for(i=0;i<50;i++){
		free(gamePath[i]);
	}

	// free memory for game path
	free(gamePath);

	// free information inside structs
	freeGraph(graph);

	// free the graph itself
	free(graph);

	// destroy the mutex
	pthread_mutex_destroy(&myMutex);

	// return 0 for no errors
	return 0;
}
