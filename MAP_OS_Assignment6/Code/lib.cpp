#include <stdio.h>
#include <string.h>
#include "header.h"
struct Cell initializeCell(char ip[], char port[], char nodeId[]) {
	struct Cell newCell;
	strcpy(newCell.ip,ip);
	strcpy(newCell.port,port);
	strcpy(newCell.nodeId,nodeId);
	return newCell;
}

struct Server initializeServer(struct Cell c) {
	struct Server server;
	server.serverDetails = c;
	return server;
}
struct leafSetStructure initializeLeafSet() {
	struct leafSetStructure leaf;
	struct Cell c;
	c.nodeId[0] = '\0';
	c.ip[0] = '\0';
	c.port[0] = '\0';
	strcpy(c.port,"\0");
	leaf.closestIds.resize(4, c);
	return leaf;
}
struct routingTableStructure initializeRoutingTable() {
	struct routingTableStructure rt;
	struct Cell c;
	c.nodeId[0] = '\0';
	c.ip[0] = '\0';
	c.port[0] = '\0';
	rt.entries.resize(4, vector<struct Cell>(4,c));
	return rt;
}
struct hashTable initializeHashTable() {
	struct hashTable h;
	return h;
}
struct stateTable initializeStateTable() {
	struct stateTable s;
	s.routingTable = initializeRoutingTable();
	s.leafSet = initializeLeafSet();
	return s;
}

struct message createMessageForJoin(char ip[], char port[], const char nodeId[]) {
	struct message msg;
	strcpy(msg.ip, ip);
	strcpy(msg.port, port);
	strcpy(msg.nodeId, nodeId);
	strcpy(msg.messageType, "join");
	return msg;
}

void sendPacketForJoin(int list_fd,char op[], char ip[], char port[],string nodeID)
{
	string pack=createPacketForJoin(createMessageForJoin(ip,port,nodeID.c_str()));
	// struct message msg =createMessageForOperation("put",key,value);
	int pack_size=pack.size();
	char pack_msg[pack_size];
	strcpy(pack_msg, pack.c_str());
	write(list_fd, pack_msg, strlen(pack_msg));
}

string createPacketForJoin(struct message msg) {
	string pack;
	pack=pack+msg.messageType+"##"+msg.ip+"##"+msg.port+"##"+msg.nodeId+"##";
	return pack;
}

struct message createMessageForOperation(char op[], char key[], char  value[]) {
	struct message msg;
	strcpy(msg.key, key);
	strcpy(msg.value, value);
	strcpy(msg.messageType, op);
	return msg;
}

string createPacketForOperation(struct message msg) {
	string pack;
	pack=pack+msg.messageType+"##"+msg.key+"##"+msg.value+"##";
	return pack;
}

void sendPacketForOperation(int list_fd,char op[], char key[], char  value[])
{
	string pack=createPacketForOperation(createMessageForOperation(op,key,value));
	// struct message msg =createMessageForOperation("put",key,value);
	int pack_size=pack.size();
	char pack_msg[pack_size];
	strcpy(pack_msg, pack.c_str());
	write(list_fd, pack_msg, strlen(pack_msg));
}

void printLeafSet(struct leafSetStructure leafSet, struct Node node) {
	printf("Leaf Set\n");
	int i=0;
	while(i<4) {
		if (i==2) {
			if (node.nodeId[0]!='\0')
				printf("%s  ", node.nodeId);
			else 
				printf("----  ");
		}
		if (leafSet.closestIds[i].nodeId[0]!='\0') {
			printf("%s", leafSet.closestIds[i].nodeId);
		} else {
			printf("----");
		}
		printf("  ");
		i++;
	} 
	printf("\n");
}

void printRoutingTable(struct routingTableStructure routingTable) {
	printf("Routing Table\n");
	int i=0,j;
	while(i<4) {
		j=0;
		while(j<4) {
			if (routingTable.entries[i][j].nodeId[0]!='\0') {
				printf("%s", routingTable.entries[i][j].nodeId);
			} else {
				printf("----");
			}
			printf("  ");
			j++;
		}
		printf("\n");
		i++;
	} 
	printf("\n");
}
void printHashTable(hashTable h) {
	map<string, string>::iterator it;
	map<string, string> m=h.entry;
	int count = 0;
	printf("Local Hash Table\n");
	for (it=m.begin();it!=m.end();it++) {
		cout << it->first << ":" << it->second << endl;
		count++;
	}
	printf("\n");
	printf("Total entries in the Local Hash Table : %d\n", count);
}
void printNode(struct Node node){
	printf("======================%s:%s=======================\n", node.nodeIP, node.port);
	printLeafSet(node.statetable.leafSet, node);
	printRoutingTable(node.statetable.routingTable);
	printHashTable(node.hashtable);
}
int getClosest(struct Node *node,string key)
{
	int i,val1,val2,min,index;
	char s[100];
	strcpy(s,key.c_str());
	sscanf(s,"%d",&val1);
	sscanf(node->nodeId,"%d",&val2);
	min=abs(val1-val2);
	index=-1;
	for(i=0;i<4;i++)
	{
		sscanf(node->statetable.leafSet.closestIds[i].nodeId,"%d",&val2);
		if(abs(val1-val2)<min)
		{	
			min=abs(val1-val2);
			index=i;
		}
	}
	return index;
}


/*void operation(struct Node *node, char op[], string key, char value[], char result[]) {
	struct hashTable h = node->hashtable;
	//string key1(key);
	string value1(value);
	//cout << value1 << "in cal" << endl;
	if(!strcmp(op, "put")) {
		// cout << "in put";
		node->hashtable.entry[key] = value1;
	} else if (!strcmp(op, "get")) {
		string val;
		map<string,string>::iterator it = node->hashtable.entry.find(key);
		if(it != node->hashtable.entry.end())
		{
		   //element found;
		    val= node->hashtable.entry[key];
		}
		// cout << "in get";
		//cout << val << "in hjas";
		strcpy(result,  val.c_str());
	}
}*/

struct Cell operation(struct Node *node, char op[], string key, char value[], char result[]) {
	int index;
	struct hashTable h = node->hashtable;
	//string key1(key);
	string value1(value);
	//cout << value1 << "in cal" << endl;
	
	if(!node->isJoined)
	{
		if(!strcmp(op, "put")) {
			node->hashtable.entry[key] = value1;
		} else if (!strcmp(op, "get")) {

			string val;
			map<string,string>::iterator it = node->hashtable.entry.find(key);
			if(it != node->hashtable.entry.end())
			{
			   //element found;
			    val= node->hashtable.entry[key];
			}
			// string val = node->hashtable.entry[key];
			//cout << val << "in hjas";

			strcpy(result,  val.c_str());
		}
	}
	else
	{
		index=getClosest(node,key);
		if(index==-1)
		{
			if(!strcmp(op, "put")) {
				node->hashtable.entry[key] = value1;
			} else if (!strcmp(op, "get")) {
				string val;
				map<string,string>::iterator it = node->hashtable.entry.find(key);
				if(it != node->hashtable.entry.end())
				{
				   //element found;
				    val= node->hashtable.entry[key];
				}
				// string val = node->hashtable.entry[key];
				//cout << val << "in hjas";
				strcpy(result,  val.c_str());
			}
		}
		else
		{
			return	node->statetable.leafSet.closestIds[index];
		}	
	}
	struct Cell c;
	c.nodeId[0]='\0';
	return c;
}
void setJoin(vector<vector<struct Cell> > &entries,vector<struct Cell> &closestIds, struct Node *node, char ip[], char port[], char nodeId[]) {
	node->isJoined=true;
	int i=0, val, val2;
	while(1) {
		while(node->nodeId[i]==nodeId[i])
			i++;
		strcpy(entries[i][nodeId[i]-'0'].nodeId, nodeId);
		strcpy(entries[i][nodeId[i]-'0'].ip, ip);
		strcpy(entries[i][nodeId[i]-'0'].port, port);
		sscanf(nodeId,"%d",&val);
		sscanf(node->nodeId,"%d",&val2);
		if (val<val2) {
			if (closestIds[0].nodeId[0]=='\0') {
				strcpy(closestIds[0].nodeId, nodeId);
				strcpy(closestIds[0].ip, ip);
				strcpy(closestIds[0].port, port);
			} else {
				strcpy(closestIds[1].nodeId, nodeId);
				strcpy(closestIds[1].ip, ip);
				strcpy(closestIds[1].port, port);
			}
		} else {
			if (closestIds[2].nodeId[0]=='\0') {
				strcpy(closestIds[2].nodeId, nodeId);
				strcpy(closestIds[2].ip, ip);
				strcpy(closestIds[2].port, port);
			} else {
				strcpy(closestIds[3].nodeId, nodeId);
				strcpy(closestIds[3].ip, ip);
				strcpy(closestIds[3].port, port);
			}
		}
		break;
		
	}
}
void setRTOnServerStart(vector<vector<struct Cell> > &entries, char md5[], struct Node node) {
	//leafSetStructure lf = node->statetable.leafSet;
	// char md5[100];
	// strcpy(md5, node->nodeId);
	struct Cell c ;
	int i =0, j=0, val;
	while(md5[i]!='\0' && i < 4) {
		val = md5[i]-'0';
		c = entries[j][val];
		strcpy(entries[j][val].nodeId, node.nodeId);
		strcpy(c.ip, node.nodeIP);
		strcpy(c.port, node.port);
		j++;
		i++;
	}
}

void client_read (int sockfd,char *buffer)
{
	int ret;
	ret = read(sockfd,buffer,100);
	if (ret < 0){ 
		perror("Client:ERROR reading from socket");
		exit(1);
	}
	buffer[ret]='\0';
	//cout<<"client_read::" <<buffer <<endl;	
}

void serv_read(int n_fd,char *buffer)
{
	//char buffer[MAX_READ];
	int ret = read( n_fd,buffer,MAX_READ);
	if (ret >= 0) {
		buffer[ret]='\0';
		// perror("RepoServer:ERROR reading from socket");
		// exit(1);
	}
	//cout << "Message: " << buffer<< endl;
}


struct message createMessageForNodeJoin(char ip[], char port[], char nodeId[]) {
	struct message msg;
	strcpy(msg.ip, ip);
	strcpy(msg.port, port);
	strcpy(msg.nodeId, nodeId);
	strcpy(msg.messageType, "nodejoin");
	return msg;
}

void sendPacketForNodeJoin(int list_fd,char op[], char ip[], char port[],char nodeID[])
{
	string pack=createPacketForNodeJoin(createMessageForNodeJoin(ip,port,nodeID));
	// struct message msg =createMessageForOperation("put",key,value);
	int pack_size=pack.size();
	char pack_msg[pack_size];
	strcpy(pack_msg, pack.c_str());
	write(list_fd, pack_msg, strlen(pack_msg));
}

string createPacketForNodeJoin(struct message msg) {
	string pack;
	pack=pack+msg.messageType+"##"+msg.ip+"##"+msg.port+"##"+msg.nodeId+"##";
	return pack;
}


