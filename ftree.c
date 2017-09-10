#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <dirent.h>
#include <arpa/inet.h>
#include "ftree.h"





void concatinate(char *path, char *name){
      int len = strlen(path);
 printf("file: %s\n", name);
      if(path[len -1 ] != '/' && len != 0){
            path[len] = '/';
        path[len+1] = '\0';
            len += 1;
        }
      strncat(path, name, strlen(name));
      path[len + strlen(name)  ] = '\0';

}


/*separate the name from end of the path*/
void seperate_strings(char *path, char *name){
        int len = strlen(path);
        if(path[len -1 ] != '/'){
            path[len - strlen(name)] = '\0';
        }else{
            path[len - strlen(name) +1] = '\0';
        }
}
   

/*construct struct stat for a file and returns the address of this struct*/
struct stat *statistics(const char *file_name){
    struct stat * stat_info= malloc(sizeof(struct stat));
    printf("file_name: %s\n", file_name);
    if(lstat(file_name, stat_info)== -1){
        perror("lstat");
		exit(1);
    }
    return stat_info;
}
/*copy char's from source (file) to the array data*/
void copy(FILE *source, char *data){
	int index = 0;
	char hold_char [2] = {'\0'}; 
    while(fread(hold_char, 1, sizeof(char), source) == 1 ){
        data[index] = hold_char[0];
		index += 1;
        }
	data[index ]= '\0';
	
}
int setup_client( char *host, unsigned short port){
     int soc;
     struct sockaddr_in peer;
 
     if ((soc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         perror("rrecieve_path(soc, base_path);andclient: socket");
         exit(1);
     }
 
     peer.sin_family = AF_INET;
     peer.sin_port = htons(PORT);
 
     if (inet_pton(AF_INET, host, &peer.sin_addr) < 1) {
         perror("rcopy_client: inet_pton");
         close(soc);
         exit(1);
     }
 
     if (connect(soc, (struct sockaddr *)&peer, sizeof(peer)) == -1){
         perror("rcopy_client: connect");
         exit(1);
     }
     return soc;
}

void fork_copy(int state, char *base_name,char *base_path, char *host, unsigned short port ){
	int result = 1;
	int soc, hold1, hold2;
	FILE *f;
	char data [MAXDATA];
	if(state == SENDFILE){ 	
		result = fork();
		//only proccess children
		if(result == 0){
			soc = setup_client(host, port);
			hold1 = ntohl(TRANSFILE);			
			
			write(soc, &hold1, sizeof(hold1)); // TODO: Maybe change			
			
			write(soc, base_name ,MAXPATH);
			f  = fopen(base_path, "r");
	        if (f == NULL){
                printf("Failed to open: %s", );		
				exit(1);
            }
			copy(f, data);
			write(soc, data, MAXDATA);
			//read whether it succed or not
			read(soc, &hold1, sizeof(hold1));
			hold2 = ntohl(hold1);
			if (hold2 == OK){
				printf("Hello"); // TODO: Remove me
			}else{
		
			}
		
			exit(1);
		}
	}	
}  

int trace_send(char *source,char * base_n ,  int soc, char *host, unsigned short port){	
	DIR *dir;
	char base_path [MAXPATH];
	char base_name [MAXPATH];
	int input_type;
	

	struct dirent *file_info;
	struct stat *stat_source;	

	//char msg [MAXDATA];	
 	char hash_val[8];
	
	FILE *f;
	base_path[0] = '\0';
	base_name[0] = '\0';
	concatinate(base_name, base_n);
	concatinate(base_path, source);
	dir = opendir(base_path);
	if (dir == NULL){
		perror("opendir");
		exit(1);
	}
	
	input_type = AWAITING_PATH; // was AWAITING_TYPE
	//proccess directories !!!!!!!!!!!!!!!!
	if ((basename(base_path)[0] != '.') || ((basename(base_path)[0] == '.') && ((basename(base_path)[1] ='\0') || (basename(base_path)[1] != '.')))){
		//send the server the type of the file that it is gong to expect(directory)
		printf("in is main:%s\n", base_name);				
		int hold = htonl(REGDIR);
		write(soc, &hold, sizeof(hold));
		//make sure server done reading
		//read(soc, &hold, MAXDATA);	
		//input_type = ntohl(hold);
		if(input_type == AWAITING_PATH){
			//transfer the path to server
			write(soc, base_name,MAXPATH);
		 //we might need to send signal here asking if the operation is done	
		}//send the permisiion
		struct stat * stat_server;
		mode_t hold_mode;
		stat_server = statistics(base_path);
		hold_mode = htonl(stat_server->st_mode);
    	write(soc,&hold_mode, sizeof(hold_mode));
        
    	
    	free(stat_server);
		
    }

	// visiting all the files under the opened directory 
	while((file_info = readdir(dir)) != NULL){
		if (file_info->d_name[0] != '.'){
			concatinate(base_path, file_info->d_name);
			concatinate(base_name, file_info->d_name);	
			stat_source = statistics(base_path);
		
		if (S_ISREG(stat_source->st_mode)){
			//request_type = REGFILE;
			if (file_info->d_name[0] != '.'){
				int hold2 = htonl(REGFILE);
       			write(soc, &hold2, sizeof(hold2));
      		//   //make sure reading proccess was done in the previous step in server
        	//read(soc, &hold2, MAXDATA);
        		input_type = AWAITING_PATH;
				if(input_type == AWAITING_PATH){				
       				//transfer the path to server
				//	hold2 = htonl(base);
        			write(soc, base_name,MAXPATH);
			//		read(soc, &hold2, sizeof(hold2));
	                input_type = AWAITING_SIZE;
				}if(input_type == AWAITING_SIZE){
					//transfer the size of the file to server
					hold2 = htonl(stat_source->st_size);
					write(soc, &hold2, sizeof(stat_source->st_size));
					read(soc, &hold2, sizeof(hold2));
                    input_type = ntohl(hold2);
					fork_copy(input_type,base_name,base_path, host, port);	
				}if(input_type == AWAITING_PERM){
                    //transfer the permission of the file to server
                    hold2 = htonl(stat_source->st_mode);
                    write(soc,&hold2, sizeof(stat_source->st_mode));
              //      read(soc, &hold2, sizeof(hold2));
                    input_type = AWAITING_HASH;
                }if(input_type == AWAITING_HASH){
                    //transfer the hash of the file to server
                 	f  = fopen(base_path, "r");
				    if (f == NULL){
				    	printf("2: %s\n", base_path);
				        perror("fopen");
				        exit(1);
				    }
                    write(soc, hash(hash_val, f), 8);
                    read(soc, &hold2, sizeof(hold2));
                    input_type = ntohl(hold2);
					fork_copy(input_type,base_name,base_path, host, port);
					if (fclose(f) != 0){
						perror("fclose");
						exit(1);
					}
					
                } 
            free(stat_source);		
				seperate_strings(base_name, file_info->d_name);
				seperate_strings(base_path, file_info->d_name);
			}
		}else if (S_ISDIR(stat_source->st_mode)){
			if (file_info->d_name[0] != '.'){
				//recursive  
				printf("in is dir:%s\n", file_info->d_name);	
				trace_send(base_path, base_name , soc, host, port);
				seperate_strings(base_name, file_info->d_name);
	     		seperate_strings(base_path, file_info->d_name);
				free(stat_source);
			}
		}
		}		
			
	}
	
	return 1;
}
/*connect client and their content from the client to the server.*/
int rcopy_client(char *source, char *host, unsigned short port){
    int soc;
	char base_n [MAXPATH] = {'\0'};
	concatinate(base_n, basename(source));
    soc = setup_client(host, port);
    trace_send(source, base_n, soc, host, port);
    return 1;
}


int setup_server(void){

    int on = 1, status;
    int listenfd;
    struct sockaddr_in self;

    // creating the socket for server
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    //make sure we can reuse the port immediately after the
    //server terminates.
    status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                        (const char *) &on, sizeof(on));
    if (status == -1) {
        perror("setsockopt -- REUSEADDR");
    }

    self.sin_family = AF_INET;
    self.sin_addr.s_addr = INADDR_ANY;
    self.sin_port = htons(PORT);
    memset (&self.sin_zero, 0, sizeof(self.sin_zero)); //Initialize sin_zero to 0

    printf("Listening on %d\n", PORT);

    if (bind(listenfd, (struct sockaddr *)&self, sizeof(self)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(listenfd, 5) == -1) {
        perror("listen");
        exit(1);
    }
    return listenfd;
}
void wait_path(int soc){
	int state, hold;
		
	state = AWAITING_PATH;
    hold = htonl(state);
    write(soc,&hold, sizeof(hold));
}
void recieve_path(int soc,struct request * client){
	char buff[MAXPATH]; 

	client->path[0] = '\0';
	read(soc, buff, MAXPATH);
	int len = strlen(client->path);
	strncat(client->path, buff, MAXPATH);
	client->path[len + strlen(buff)] = '\0';
	//inform client that server is waiting for permissions
}  
void recieve_perm(int soc, struct request * client){
	//change the mode if there is diffrence
	struct stat * stat_server;
   	mode_t  hold_mode;
	
//	state = AWAITING_PERM;
//    hold = htonl(state);
//    write(soc,&hold, sizeof(hold));
	stat_server = statistics(client->path);
    read(soc,&hold_mode, sizeof(hold_mode));
    client->mode = ntohl(hold_mode);
    if(stat_server->st_mode != client->mode){
    	if(chmod(client->path, stat_server->st_mode) == -1){
        	perror("chmod");
            exit(1);
        }
    }
	free(stat_server);
}
int recieve_hash(int soc, struct request * client){
	char hash_val[8] = {'\0'}; //TODO: change the 8 to variables	
	FILE *f;
	
//	state = AWAITING_HASH;
//    hold = htonl(state);
//    write(soc,&hold, sizeof(hold));
	
	read(soc, hash_val, sizeof(hash_val)); 
	f  = fopen(client->path, "r");
    if (f == NULL){
	    perror("fopen");
        exit(1);
    }

	return strcmp((hash(client->hash, f)), hash_val); 
}
int recieve_size(int soc, struct request * client, FILE *f_destination_w){
	size_t size, hold_size;
	struct stat * stat_server;
	//make sure there exist 	
	f_destination_w = fopen(client->path, "w+");
    if(f_destination_w != NULL){
    	stat_server = statistics(client->path);
    }// if the file in destination does not exist then we need to create it and copy it
    if(f_destination_w == NULL){
		perror("fopen");
		exit(1);
    }
	//read the size
	read(soc, &hold_size,sizeof(hold_size));
	size = ntohl(hold_size);
	// if sizes are not equal ask for the file
	if(stat_server->st_size != size){
		//state =	SENDFILE;
		//hold = htonl(state);
		//write(soc,&hold, sizeof(hold)); 
		return 1;
	 }
	//state = OK;
    //hold = htonl(state);
    //write(soc, &hold, sizeof(hold));

	return 0;		
}

/* Accept a connection. Note that a new file descriptor is created for
 * communication with the client-> The initial socket descriptor is used
 * to accept connections, but the new socket is used to communicate.
 * Return the new client's file descriptor or -1 on error.
 */
int accept_connection(int fd, struct request *clients) {
    int user_index = 0;
    while (user_index < MAX_CONNECTIONS && clients[user_index].soc != -1) {
        user_index++;
    }

    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0) {
        perror("server: accept");
        close(fd);
        exit(1);
    }

    if (user_index == MAX_CONNECTIONS) {
        fprintf(stderr, "server: max concurrent connections\n");
        close(client_fd);
        return -1;
    }
    
    clients[user_index].soc = client_fd;
	clients[user_index].type = NO_TYPE;
	clients[user_index].step = NO_STEP;
	clients[user_index].is_ready = 0;

    return client_fd;
}


void rcopy_server (unsigned short port ) {
	// list of clients
	struct request clients[MAX_CONNECTIONS];

	for (int index = 0; index < MAX_CONNECTIONS; index++) {
        clients[index].soc = -1;
    }


	
    int listenfd;
    int  nbytes;
	FILE *f = NULL;

	int request_type;

	int state, hold;

	//char base_path [MAXPATH];
	char data [MAXDATA];
	
	//mode_t mode, hold_mode;
	//socklen_t socklen;

	//struct stat stat_server;
    struct sockaddr_in peer;

	listenfd = setup_server();

	int max_fd = listenfd;
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(listenfd, &all_fds);
	
    //keep looking for clients that would connect at any time
    while(1){
		listen_fds = all_fds;
		int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("server: select");
            exit(1);
        }
		// Is it the original socket? Create a new connection ...
        if (FD_ISSET(listenfd, &listen_fds)) {
            int client_fd = accept_connection(listenfd, clients);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &all_fds);
            printf("Accepted connection\n");
        }
        //socklen = sizeof(peer);

	    for (int index = 0; index < MAX_CONNECTIONS; index++) {
			if (clients[index].soc> -1 && FD_ISSET(clients[index].soc, &listen_fds)) {
				if (clients[index].type == NO_TYPE){
			
		        	printf("New connection on port %d\n", ntohs(peer.sin_port));

					read(clients[index].soc, &hold, sizeof(hold));
					clients[index].type = ntohl(hold);
					clients[index].step = AWAITING_PATH;
				}
				request_type = clients[index].type;
				if(request_type == REGDIR){
					// inform client that sever is ready to recieve path
			//wait(soc);
					// read the path from the client
					if (clients[index].is_ready == 0) {
						clients[index].is_ready++;
					} else if (clients[index].step == AWAITING_PATH){
						recieve_path(clients[index].soc, &(clients[index])); // TODO: Check second argument
						clients[index].step = AWAITING_PERM;
						//check if there exsit such directory in server,if not then create it
						if(mkdir(clients[index].path, 0777) == -1){
					        if(errno != EEXIST){
           						fprintf(stderr, "couldn't open %s\n", clients[index].path);
          						perror("mkdir");
        						exit(1);
       						}
   						}						
					}else if(clients[index].step == AWAITING_PERM){
						//change the mode if there is diffrence		
						recieve_perm(clients[index].soc, &(clients[index])); // TODO: Check second argument
						//reset type since the proccess of the directory ended
						clients[index].type = NO_TYPE;
						clients[index].is_ready = 0;
					}
				}else if (request_type == REGFILE){
					// inform client that sever is ready to recieve path
   		        //     wait_path(soc);
   		             // read the path from the client
					if (clients[index].is_ready == 0) {
                        clients[index].is_ready++;
                    } else if (clients[index].step == AWAITING_PATH){
	   		             recieve_path(clients[index].soc, &(clients[index])); // TODO: Check 2nd arg
						 clients[index].step = AWAITING_SIZE;
					
     		   	        //check if there exsit such directory in server,if not then create
						//compare sizes if they are not equal ask for the file from the client to copy
					}else if (clients[index].step == AWAITING_SIZE){
						if(recieve_size(clients[index].soc, &(clients[index]), f) == 0){
							clients[index].step = AWAITING_PERM;
							//write to the client to confirm that the sizes are equal
							state = AWAITING_PERM;
                            hold = htonl(state);
                            write(clients[index].soc, &hold, sizeof(hold));
						}else{
                   			 //tell client to fork and send the data in the file 
                    	    state = SENDFILE;
                       		hold = htonl(state);
                      		write(clients[index].soc, &hold, sizeof(hold));
							clients[index].step = AWAITING_TYPE;
							clients[index].type = NO_TYPE;
                	    }
					}else if (clients[index].step == AWAITING_PERM){
						 recieve_perm(clients[index].soc, &(clients[index]));
						 clients[index].step = AWAITING_HASH;
					}else if (clients[index].step == AWAITING_HASH) {

						if(recieve_hash(clients[index].soc, &(clients[index])) == 0){
							clients[index].step = AWAITING_TYPE;
							clients[index].type = NO_TYPE;
							//tell the client that the files are similar
							state = OK;
                            hold = htonl(state);
                            write(clients[index].soc, &hold, sizeof(hold));
						}else{
							//ask the client to send the file
							state = SENDFILE;
							hold = htonl(state);
							write(clients[index].soc, &hold, sizeof(hold));
							clients[index].type = NO_TYPE;
							clients[index].is_ready = 0;
						}
					}
					
				}else if(request_type == TRANSFILE){
					char hold_char [2] ={'\0'};		
					
							
					FILE *f_write;
			//			wait_path(soc);
					if (clients[index].is_ready == 0) {
                        clients[index].is_ready++;
                    } else if (clients[index].step == AWAITING_PATH){
                         recieve_path(clients[index].soc, &(clients[index]));
                         clients[index].step = AWAITING_DATA;
                    }else if (clients[index].step == AWAITING_DATA){
						nbytes = read(clients[index].soc, data, sizeof(data));
						f_write = fopen(clients[index].path, "w");
						if(f_write == NULL){
							perror("fopen");
							exit(1);
						}// eddieeee : make sure that file need null terminateor at the end of the file or not
						int i = 0;
						int len1 = strlen(data);
						printf("LEN1: %d\n", len1);
						while (len1 != i){
								printf("len1: %d\n", len1);
								printf("i: %d\n", i);
							hold_char[0] = data[i];	
							if (fwrite(hold_char, 1, sizeof(char), f_write) != 1){
								// maybe send error here
       	    					 perror("fwrite");
       	    					 printf("system error in copying a file");
        		    		}
							i += 1;
						}
						state = OK;
                        hold = htonl(state);
                        write(clients[index].soc, &hold, sizeof(hold));
						fclose(f_write);
        			}	
				}
			}
		}
	}
}
