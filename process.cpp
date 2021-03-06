#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include<iostream>
#include<vector>
#include<fstream>
#include<algorithm>
#include<cstring>
using namespace std;

struct msgbuff
{
   long mtype; // must be long
   char mtext[64];
};

struct content
{
    int time;
    string operation;
    string data;
};

int clk = -1;
vector<content> inputList;
string msg = "";
key_t upMsgqId,
      downMsgqId;

// Handler for SIGUSR2
void IncClk(int signum) 

{	//cout<<" a second passed  "<<endl;
	clk++;
}

//modify ?? char we string(message.mtext, msg)
//send via up queue
void SendMsg()
{
	int send_val;

	struct msgbuff message;

	message.mtype = getpid();
	//modify?? check right msg
	strcpy(message.mtext, msg.c_str());

	// busy wait until msg sent
	cout<<" sending message to kernel "<<message.mtext<<endl;
	send_val = msgsnd(upMsgqId, &message, sizeof(message.mtext), IPC_NOWAIT);
	cout<<" done sending"<<endl;
	if(send_val == -1)
		perror("Errror in send");

}




// recieve via down queue
void RecieveMsg()
{
	int rec_val;
	struct msgbuff message;

	/* receive all types of messages */
	rec_val = msgrcv(downMsgqId, &message, sizeof(message.mtext),getpid(), !IPC_NOWAIT);  

	if(rec_val == -1)
		perror("Error in receive");
    //modify ??
    else if (message.mtext == "0")
        cout << "Successful ADD\n";
    else if (message.mtext == "1")
        cout << "Successful DEL\n";
    else if (message.mtext == "2")
        cout << "Unable ADD\n";
    else if (message.mtext == "3")
        cout << "Unable DEL\n";
}

//modify ??: check a valid input
//Assume: Time are unique
//Read input file 
bool InputRead(string fileName){
    content inputs; 
	ifstream inputFile(fileName.c_str()); //the constructor does not take string& until C++11. Prior to C++11, it only takes const char *
	if(inputFile.is_open()){
		while(inputFile >> inputs.time >> inputs.operation){
			getline(inputFile,inputs.data);
			//remove first space
			inputs.data.replace(0,1,"");
			inputList.push_back(inputs);
			cout<<"time "<<inputs.time<<" operation "<<inputs.operation<<" data "<<inputs.data <<endl;
		}
		inputFile.close();
		return true;
	}
	else{
		cout <<"Can't open " << fileName << " file\n";
		return false;
	}
}

bool CheckAvailableMsg()
{
    msg = "";
    for(unsigned i = 0; i<inputList.size(); i++){
        if(inputList[i].time == clk){
            if(inputList[i].operation == "ADD")
                msg += "A";
            else
                msg += "D";
           msg += inputList[i].data;
		cout<<"MSG SENT FROM PROCESS "<<msg<<endl;
		   //remove the sent msg
		   inputList.erase (inputList.begin()+i);
           return true;
        }
    }
    return false;
}

int main()
{   
	printf("Begin Of Process MAIN \n");
    string fileName = "";
    //when the process receives SIGUSR2 it increments the clk variable
    signal(SIGUSR2,IncClk);
         
    //cout << "Please enter the file name\n"; //modify ??
    //cin >> fileName;
    fileName="input.txt";
    if(!InputRead(fileName)) return 0;
    
    //modify ??
    upMsgqId = msgget(99, IPC_CREAT|0644);//199 for up // dummy values 
	downMsgqId = msgget(100, IPC_CREAT|0644);// 200 for down // kernel should have the same keys
	cout<<"Process Up Queue ID "<<upMsgqId<<endl;
	cout<<"Process down Queue ID "<<downMsgqId<<endl;
	if(upMsgqId == -1 ||downMsgqId == -1 )
	{	
		perror("Error in create");
		exit(-1);
	}

	//SendIntializationMsg
	msg="P";
	SendMsg();
    while(1){
        if(CheckAvailableMsg()){
            SendMsg();
        }
        
        RecieveMsg();
    }
    
	return 0;
}
