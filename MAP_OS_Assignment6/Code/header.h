#include <stdio.h>
#include <string>
#include <vector>
#include <map>

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
#include <iterator>

using namespace std;
#define MAX_READ 1024

struct Cell {
	char ip[16];
	char port[5];
	char nodeId[9];
	char s[5];
};
struct Server {
	struct Cell serverDetails;
};
struct message {
	char key[255];
	string key1;
	char value[100];
	char ip[16];
	char port[5];
	char nodeId[9];
	char messageType[10];
};
struct leafSetStructure	{
	vector<struct Cell> closestIds;
};
struct routingTableStructure {
	vector<vector<struct Cell> > entries;
};
struct stateTable {
	int hopCount;
	string s1;
	struct leafSetStructure leafSet;
	struct routingTableStructure routingTable;
};
struct hashTable {
	map<string, string> entry;
};
struct Node {
	char nodeId[9];
	char nodeIP[16];
	char port[5];
	int serverSockFd;
	struct stateTable statetable;
	struct hashTable hashtable;
	bool isJoined;
};
struct hashTable initializeHashTable();
struct Cell initializeCell(char ip[], char port[], char nodeId[]);
struct Server initializeServer(struct Cell c);
struct stateTable initializeStateTable();
struct routingTableStructure initializeRoutingTable();
struct leafSetStructure initializeLeafSet();
struct message createMessageForJoin(char ip[], char port[],const char nodeId[]);
struct message createMessageForOperation(char op[], char key[], char value[]);
string createPacketForOperation(struct message msg);
void sendPacketForOperation(int list_fd,char op[], char key[], char value[]);
void printNode(struct Node node);
void printLeafSet(struct leafSetStructure leafSet, struct Node node);
void printRoutingTable(struct routingTableStructure routingTable);
void printHashTable(struct hashTable h);
void client_read (int sockfd,char *buffer);
void serv_read(int n_fd,char *buffer);
void setRTOnServerStart(vector<vector<struct Cell> > &entries, char md5[], struct Node node);
string binconvert(string);
string quadconvert(string);
void sendPacketForJoin(int list_fd,char op[], char ip[], char port[],string nodeID);
string createPacketForJoin(struct message msg);
void *connectNode(void *context);
string createPacketForNodeJoin(struct message msg);
void sendPacketForNodeJoin(int list_fd,char op[], char ip[], char port[],char nodeID[]);
struct message createMessageForNodeJoin(char ip[], char port[], char nodeId[]);
void setJoin(vector<vector<struct Cell> > &entries,vector<struct Cell> &closestIds,struct Node *node, char ip[], char port[], char nodeId[]);
struct Cell operation(struct Node *node, char op[], string key1, char value[], char result[]);
int getClosest(struct Node *node,string key1);

struct param {
    char ip[16];
    char port[5];
    char op[10];
    char key[255];
    char value[100];
};
