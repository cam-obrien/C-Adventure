#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>



//		   Struct		//

struct Room{
	char * name;
	char * type;
	int num_connections;
	int open_connections;
	struct Room * connections;
};



//		Function Headers	//

void initGraph(struct Room *);
int nameTaken(struct Room *, const char *);
void connectGraph(struct Room *);
int validateConnecton(struct Room, struct Room);
int graphIsFull(struct Room *);
void addRandomConnection(struct Room *);
int createFiles(struct Room * graph);
void writeToFile(struct Room * graph, char * filepath, int place);



//		Function Bodies		//



/*********************
 * Function: nameTaken
 * Parameters: room array graph, string name
 * Description: checks through the names in the graph, if a duplicate is found returns 1, else the name is original
 *  		and returns 0
*********************/
int nameTaken(struct Room * graph, const char * name){
	int i;

	// iterate through the graph
	for(i=0;i<7;i++){

		// if there is no name assigned to this spot, return 0
		if(graph[i].name == NULL){
			return 0;
		}
		// if there is a name and it matches the name passed in, return 1 as it is not a viable name
		else if(!(strcmp(graph[i].name, name))){
			return 1;
		}
	}

	// the name is original and may be used. return 0
	return 0;
}



/*********************
 * Function: initGraph
 * Parameters: room array
 * Description: initializes the graph and gives it a name and room type
*********************/
void initGraph(struct Room * graph){
	srand(time(NULL));
	
	int i, rName;
	
	// const array of hardcoded names
	const char * names[10] = {"funny", "scary", "dungeon", "gold", "dark", "stairs", "armory", "slippery", "mirror", "slide"};

	// iterate 7 times for the 7 rooms
	for(i=0;i<7;i++){

		// create current room
		struct Room r;

		// allocate memore for name and type strings
		r.name = calloc(16, sizeof(char));
		r.type = calloc(16, sizeof(char));

		// if a duplicate name is created it generates a new number from the array of names until an
		// original name is found
		do{
			rName = rand() % 10;
		}while(nameTaken(graph, names[rName]));

		// copy original name to room struct
		strcpy(r.name, names[rName]);

		// set the number of connections from this room to 0
		r.num_connections = 0;
		
		// allocate memory for connections
		r.connections = calloc(r.num_connections, sizeof(struct Room));
		
		// allocate memory for connection names
		int j;
		for(j=0;j<6;j++){
			r.connections[j].name = calloc(16, sizeof(char));
		}

		// assign room types based on the state of the for loop
		switch(i){
			case 0: strcpy(r.type, "START_ROOM"); break;
			case 6: strcpy(r.type, "END_ROOM"); break;
			default: strcpy(r.type, "MID_ROOM"); break;
		}

		//assign the room to its spot in the graph
		graph[i] = r;		
	}	
}



/*********************
 * Function: graphIsFull
 * Parameters: room array
 * Description: checks each spot in the graph for the number of connections, if each has at least 3 then return 
 * 		that the graph is full
*********************/
int graphIsFull(struct Room * graph){
	int i;

	// iterate through the graph
	for(i=0;i<7;i++){

		// if this spot in the graph has less than 3 connections, return 0 as it is not full
		if(graph[i].num_connections < 3){
			return 0;
		}
	}

	// each spot in the graph has at least 3 connections, return 1
	return 1;
}



/*********************
 * Function: addRandomConnection 
 * Parameters: room array
 * Description: grabs two random rooms from the graph and connects them to eachother
*********************/
void addRandomConnection(struct Room * graph){
	srand(time(NULL));
	
	// holds the spots in the graph of the two rooms 
	int r1, r2;

	// loops until a room with less than 6 connections is found
	do{
		// generate a random number from 0-6
		r1 = rand() % 7;
	}while(!(graph[r1].num_connections < 6));

	// loops until a room is found with less than 6 connections, and until a the second room is not the same as the first
	do{
		r2 = rand() % 7;
	}while((r1 == r2) || (!(graph[r2].num_connections < 6)));

	// if the connection is valid then connect the two rooms
	if(!(validateConnection(graph[r1], graph[r2]))){

		// put the rooms in eachother's connection arrays
		graph[r1].connections[graph[r1].num_connections] = graph[r2];
		graph[r2].connections[graph[r2].num_connections] = graph[r1];
		
		// increment the number of connections for the two rooms
		graph[r1].num_connections++;
		graph[r2].num_connections++;
	}
}



/*********************
 * Function: validateConnection
 * Parameters: room 1, room 2
 * Description: validates that the rooms are not already in eachother's connection array, not 
 * 		allowing duplicate connections
*********************/
int validateConnection(struct Room r1, struct Room r2){
	int i;
	
	// checks room 1's connections for room 2's name, since when one connection is added to
	// another they both get eachothers name, it is not neccesary to check both rooms connections
	// for the others name. Only one way is needed
	for(i=0;i<r1.num_connections;i++){

		// if room 2's name is already in room 1's connections, return an error
		if(!(strcmp(r1.connections[i].name, r2.name))){
			return 1;
		}
	}

	// the rooms have not already been connected, return no error
	return 0;
}



/*********************
 * Function: connectGraph
 * Parameters: room array
 * Description: adds connections until the graph is full
*********************/
void connectGraph(struct Room * graph){
	
	// while the graph is not full
	while(!(graphIsFull(graph))){

		// add random connections
		addRandomConnection(graph);		
	}
}



/*********************
 * Function: createFiles
 * Parameters: room array
 * Description: creates the directory with pid, then creates the files and writes to them to hold 
 * 		the data in each spot of the graph array
*********************/
int createFiles(struct Room * graph){
	int i;
	
	// get this programs pid
	pid_t pid = getpid();

	// convert the pid to a string
	char * mypid = malloc(21);
	sprintf(mypid, "%lu", pid);

	// append the pid to obriecam.rooms.
	char dirpath[100] = "obriecam.rooms.";
	strcat(dirpath, mypid);

	// make the directory with the prefix and pid appended
	mkdir(dirpath, 0755);
	
	// append a "/" for adding files
	strcat(dirpath, "/");

	// will hold file paths for each file
	char filepath[100];

	for(i=0;i<7;i++){
	
		// create space for the room's name to be appended with "_room"
		char roomname[100];
		strcpy(roomname, graph[i].name);
		strcat(roomname, "_room");

		//reset the file path variable with the directory path
		strcpy(filepath, dirpath);

		//cat the room name string to the filepath
		strcat(filepath, roomname);
	
		// create file with filepath name
		int file_descriptor;
		file_descriptor = open(filepath, O_CREAT, 0600);
		if(file_descriptor < 0){
			// if the file does not exist, return an error
			return 1;
		}

		// write to the file with the current iteration of the graph passed in
		writeToFile(graph, filepath, i);
	}	

	// all files made successfully
	return 0;	
}



/*********************
 * Function: writeToFile
 * Parameters: room array, filepath, and place in the graph array
 * Description: writes to the room file of the current room
*********************/
void writeToFile(struct Room * graph, char * filepath, int place){
	int i;
	FILE *fp;

	// open the file with write permissions
	fp = fopen(filepath, "w+");

	// print the room name into the file
	fprintf(fp, "ROOM NAME: %s", graph[place].name);

	// print each connection into the file
	for(i=0;i<graph[place].num_connections;i++){
		fprintf(fp, "\nCONNECTION %d: %s", (i+1), graph[place].connections[i].name);
	}	
	
	// print the room type into the file
	fprintf(fp, "\nROOM TYPE: %s\n", graph[place].type);

	// close the file stream
	fclose(fp);
}


//		Main		//

int main(){
	int i, j;
	struct Room * graph = calloc(7, sizeof(struct Room));
	initGraph(graph);	
	connectGraph(graph);
	if(createFiles(graph)){
		printf("error while making files\n");
		return 1;
	}

	return 0;

}
