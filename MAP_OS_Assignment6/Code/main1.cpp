 // g++ main1.cpp -lpthread -o a.out

#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fstream>
#include <pthread.h>
#include <strings.h>
#include <string>
#include <stdio.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include "header.h"
#include "md5.h"
// #include "md5.h"


#define MAX_thread 3		// CLI, server, one request at a time
#define CLI_thread 0
#define SERV_thread 1
#define Req_handle_thread 2
using namespace std;

int serv_port;
int status;
void *CLI_task(void *);

void *Serv_task(void *);
void *req_proc_task (void *fd);

// static int connFd;
pthread_t thread_pool[MAX_thread];

struct Server server;
struct Node node;

int main()
{	
	node.statetable = initializeStateTable();
	node.hashtable = initializeHashTable();

	pthread_create(&thread_pool[CLI_thread], NULL, CLI_task, NULL);

	pthread_create(&thread_pool[SERV_thread], NULL, Serv_task, NULL); 
	


	/*for(int i = 0; i < MAX_thread-1; i++)
    {
        pthread_join(thread_pool[i], NULL);
    }*/
    pthread_join(thread_pool[CLI_thread], NULL);
	return 0;
}
void *CLI_task(void *)
{
	int temp_port=0;
	int list_fd;
	struct sockaddr_in svrAdd;
	struct hostent *server;
	//create client skt
	list_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(list_fd < 0)
	{
		cerr << "CLI_task:Cannot open socket" << endl;
		// return 0;
	}
	// else{}
	server = gethostbyname("localhost");
	if(server == NULL)
    {
        cerr << "Host does not exist" << endl;
        // return 0;
    }
    // else{}
    bzero((char *) &svrAdd, sizeof(svrAdd));
	svrAdd.sin_family = AF_INET;

	bcopy((char *) server -> h_addr, (char *) &svrAdd.sin_addr.s_addr, server -> h_length);

	// svrAdd.sin_port = htons(serv_port);


	// cout <<"In CLI_task:: " << serv_port << endl;
	while(1)
	{
		// int temp_port=0;
		string cli_inp;
		cout << "pastry $ ";
		cin >> cli_inp;
		if(cli_inp=="port")
		{
			cin >> temp_port;
			if((temp_port > 65535) || (temp_port < 2000))
			{
				cerr << "Please enter a port number between 2000 - 65535" << endl;
				temp_port=0;
			}
			else
			{
				cout <<"Port set to "<<temp_port<<endl;
				/*check port available 
				if not available temp_port=0 and give out proper errmsg*/
				
				/*serv_port=temp_port;			// when create is called*/	
			}
			// else
			// {
			// 	cerr << "PORT already assigned: Don't run CREATE" << endl;
			// }
		}
		else if(cli_inp=="create")
		{
			if(temp_port!=0 && serv_port==0)
			{
				serv_port=temp_port;
				svrAdd.sin_port = htons(serv_port);
				int checker = connect(list_fd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));
				// cout << "checker:"<<checker << endl;
			    if(checker <0)
			    {
			    	cerr << "Cannot connect! in create" << endl;
			    	break;
			    }
			}
			else if(temp_port==0)
			{
				cerr << "Node ip and port are not initialized." << endl;
			}
			else if(serv_port!=0)
			{
				cerr << "Server is already running." << endl;
			}
		}
		else if(cli_inp=="put")
		{
			char key[255], value[100];
			cin >>key;
			cin >>value;
			// svrAdd.sin_port = htons(serv_port);
			if(serv_port!=0)
			{
				sendPacketForOperation(list_fd,"put",key,value);
				status=0;
				while(status==0);			//wait till put is completed at server side
				status=0;
				// cout<<"new key added"<<endl;
			}
		}

		else if(cli_inp=="join")
		{
			// cout<< "in cli.. join";
			char ip[16], port[5];
			cin >>ip;
			cin >>port;
			// svrAdd.sin_port = htons(serv_port);
			if(serv_port!=0)
			{
				string s1;
			    char key[30];
				strcpy(key,ip);
			    strcat(key,port);
			    s1=md5(key);
				s1=s1.substr(0,2);
				string s2=binconvert(s1);
				s1=quadconvert(s2);
				s1=s1.substr(0,4);
				sendPacketForJoin(list_fd,"join",ip,port,s1);
				// cout << "done...in cli"<<endl;

				status=0;
				while(status==0);			//wait till put is completed at server side
				status=0;
			}
		}
		else if(cli_inp=="get")
		{
			char key[255],value[100]="0";
			cin >>key;
			if(serv_port!=0)
			{
				// svrAdd.sin_port = htons(serv_port);
				// int checker = connect(list_fd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));
				// cout <<"checker ::"<<checker<< endl;
				
				sendPacketForOperation(list_fd,"get",key,value);
				client_read(list_fd,value);
				if(strcmp(value,"##"))
				{
					cout <<value <<endl;
				}
			}
		} else if (cli_inp=="dump") {
			printNode(node);

		}
		else if (cli_inp=="lset") {
			printLeafSet(node.statetable.leafSet, node);
			// printNode(node);

		}
		else if (cli_inp=="routetable") {
			printRoutingTable(node.statetable.routingTable);
			// printLeafSet(node.statetable.leafSet, node);
			// printNode(node);

		}
		else if(cli_inp=="shutdown")
		{
			cout << "Shutting server down..."<<endl;
			sendPacketForOperation(list_fd,"exit","NULL","NULL");
			break;
		}
		else if(cli_inp=="quit")
		{
			cout  << "Redistribution starts in 5 seconds..."<<endl;
			sleep(5);
			cout << "Redistributing..."<<endl;
			cout << "\nShutting server down..."<<endl;
			sendPacketForOperation(list_fd,"exit","NULL","NULL");
			break;
		}
	}
}


void *Serv_task(void *)
{
	int list_fd;	
	socklen_t length;
	struct sockaddr_in serv_addr, cli_add;

	while(serv_port==0);
	//cout << serv_port << endl;

	list_fd = socket(AF_INET, SOCK_STREAM, 0);
	//cout << "list_fd  "<< list_fd << endl;
	if(list_fd < 0)
	{
		cerr << "Cannot open socket" << endl;
		// pthread_exit ();
		// return 0;
	}
	else
	{
		bzero((char*) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(serv_port);

		if(bind(list_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			cerr << "Cannot bind" << endl;
			// return 0;
			// pthread_exit ();
		}
		else
		{
			listen(list_fd, 5);
			char *publicip;
			struct ifaddrs * ifAddrStruct=NULL;
		    struct ifaddrs * ifa=NULL;
		    void * tmpAddrPtr=NULL;      
		    getifaddrs(&ifAddrStruct);

		    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		        if (ifa ->ifa_addr->sa_family==AF_INET) 
		        { 

		            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
		            char addressBuffer[INET_ADDRSTRLEN];
		            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
		            // cout <<"ranj::"<< ifa->ifa_name <<"rupali "<<addressBuffer<<endl;
		            if (!(strcmp(ifa->ifa_name,"wlan0"))|| !(strcmp(ifa->ifa_name,"enp1s0")))
		            {
		            	publicip = (char *)malloc(sizeof(char)*16);
		            	strcpy(publicip,addressBuffer);
		            	break;
		            }
		            
		        }  
		    }
		    string s1;
		    char *buf;
		    char nodeId[16];
		    char val1[100];
		    buf=(char *)malloc(sizeof(char)*16);
			sprintf(buf, "%d", serv_port);
			strcpy(val1,publicip);
		    strcat(val1,buf);
		    s1=md5(val1);
			s1=s1.substr(0,2);
			string s2=binconvert(s1);
			s1=quadconvert(s2);
			s1=s1.substr(0,4);
			strcpy(nodeId, s1.c_str());
			struct Cell c = initializeCell(publicip, buf, nodeId);
			server = initializeServer(c);
		
			strcpy(node.nodeIP, publicip);
			strcpy(node.port, buf);
			strcpy(node.nodeId, s1.c_str());
			//vector<vector<struct Cell> > a = ;
			//cout << node.nodeIP << node.port << node.nodeId;
			node.isJoined = false;
			setRTOnServerStart(node.statetable.routingTable.entries, node.nodeId, node);
			
			pthread_t new_req_thr;
			while(1)
			{
				int connFd = accept(list_fd, (struct sockaddr *)&cli_add, &length);
				// char buffer[100];
				// read(list_fd,buffer,100);
				// cout << buffer;
				if (connFd < 0)
				{
					cerr << "Cannot accept connection" << endl;
					// pthread_exit ();
				}
				else
				{
					
					pthread_create( &new_req_thr, NULL, req_proc_task,(void *) &connFd);
					// pthread new_req_thr(task1,NULL);
					// pthread_create(&thread_pool[Req_handle_thread], NULL, task1, NULL); 
				}	
			}
		}
	}
}


void *req_proc_task (void *fd)
{
	int *conn_Fd=(int *)fd;
	int connFd=*conn_Fd;
	while(1)
	{
		//cout << "in req_proc_task Thread No: " << pthread_self() << endl;
		char buffer[MAX_READ];
		// cout<<"waiting"<< pthread_self()<<endl;
		serv_read(connFd,buffer);
		// cout <<buffer<<endl;
		char result[100];
		char *nodeId;
		char *op= strtok (buffer,"##");
		if(!strcmp(op,"exit"))
			break;
		char *key = strtok (NULL,"##");			// ip
		char *value= strtok (NULL,"##");		// port

		if (!strcmp(op, "join") || !strcmp(op, "nodejoin")) {
			nodeId= strtok (NULL,"##");
		} else {
			//cout << value << "in sr" << endl;
			string s1;
			s1=md5(key);
			s1=s1.substr(0,2);
			string s2=binconvert(s1);
			s1=quadconvert(s2);
			// operation(&node, op, s1, value, result);
			struct Cell c=operation(&node, op, s1, value, result);
			if(c.nodeId[0]!='\0') 
			{
				// cout<<"send packet again";
				// cout << "ipip:: "<<c.ip << endl;
				struct param readParams;
				strcpy(readParams.ip,c.ip);
				strcpy(readParams.port,c.port);
				strcpy(readParams.op,op);
				strcpy(readParams.key,key);
				strcpy(readParams.value,value);
				pthread_t comm_thr;
				pthread_create( &comm_thr, NULL, connectNode, (void *)&readParams);
				status=0;
				strcpy(result,"##");
				while(status==0);
				status=0;
				// pthread_join(comm_thr, NULL);
				
			}
		}
		if(!strcmp(op,"put"))
		{
			// put the key at the node
			status=1;
		}
		else if(!strcmp(op,"get"))
		{
			if(strlen(result)==0)
				strcpy(result,"##");
			write(connFd,result,strlen(result));
		} else if (!strcmp(op, "join")) {
			// cout<< "in req_proc_task.. join";
			// connect to given ip over port
			struct param readParams;
			strcpy(readParams.ip,key);
			strcpy(readParams.port,value);
			strcpy(readParams.op,op);
			pthread_t comm_thr;
			pthread_create( &comm_thr, NULL, connectNode, (void *)&readParams);
			// connectNode(key,value);
			pthread_join(comm_thr, NULL);
			status=1;
			setJoin(node.statetable.routingTable.entries,node.statetable.leafSet.closestIds, &node, key, value, nodeId);

			// setJoinReq(node, )
			//send();
			// cout << "done...123"<<endl;
			
		} else if (!strcmp(op, "nodejoin")) {
			// cout << "nodejoin" <<endl;
			setJoin(node.statetable.routingTable.entries,node.statetable.leafSet.closestIds, &node, key, value, nodeId);

			// joinNodes(node, key, value, nodeId);

			break;
		}
	}

	// cout << "\nClosing thread and conn" << endl;
	// close(connFd);
}


void *connectNode(void *context)
{
	// cout << "in connectNode Thread No: " << pthread_self() << endl;
	struct param *readParams = (struct param *)context;
	// char str[30];
	// strcpy(str, inp_str.c_str());

	int listenFd, portNo;
	struct sockaddr_in svrAdd;
    struct hostent *serv;
    portNo = atoi(readParams->port);
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0)
    {
        cerr << "Cannot open socket" << endl;
        // return 0;
    }
    // cout << "hihi:: "<< readParams->ip << endl;
    serv = gethostbyname(readParams->ip);

    if(serv == NULL)
    {
        cerr << "Host does not exist" << endl;
        // return 0;
    }
    bzero((char *) &svrAdd, sizeof(svrAdd));
    svrAdd.sin_family = AF_INET;

    bcopy((char *) serv -> h_addr, (char *) &svrAdd.sin_addr.s_addr, serv -> h_length);
    
    svrAdd.sin_port = htons(portNo);

    int checker = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));
    
    if (checker < 0)
    {
        cerr << "Cannot connect!" << endl;
        // return 0;
    }
    else
    {
    	// cout<< "connected"<<endl;
    	//sendPacketForOperation(listenFd,"exit","NULL","NULL");
    	if(!strcmp(readParams->op,"join"))
    		sendPacketForNodeJoin(listenFd,"nodejoin",node.nodeIP,node.port,node.nodeId);
    	else if(!strcmp(readParams->op,"put"))
    	{
    		sendPacketForOperation(listenFd,"put",readParams->key,readParams->value);
   			//status=0;
			// while(status==0);
			// status=0;
    	}
    	else if(!strcmp(readParams->op,"get"))
    	{
    		char value[100]="0";
    		sendPacketForOperation(listenFd,"get",readParams->key,readParams->value);
			client_read(listenFd,value);
			if(strcmp(value,"##"))
			{
				cout << value <<endl;
			}
			status=1;
    	}
    }
    // pthread_exit(NULL);
    // close(listenFd);
}





//node.nodeIP
//node.port
//node.nodeId