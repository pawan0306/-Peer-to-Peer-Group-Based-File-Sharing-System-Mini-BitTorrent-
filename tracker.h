#include <bits/stdc++.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
  
#define UID string
#define GID string
#define FPATH string
#define FPATH2 string
#define ACTIVE bool
#define LIMIT 15

using namespace std;

struct usr_info {

    string id;
    string ip;
    int port;
    string pswd;
    set <GID> grps_owns;
    set <GID> memberof;
    map <GID, set <FPATH>> file_shares;
    ACTIVE active;
};

struct file_info {

    FPATH fpath;
    string f_SHA;
    int pieces;
    deque <string> p_SHA;
    int f_size;
    set <UID> has_file;
}; 

struct grps_info {

    string gid;
    string ownr_id;
    set <UID> members;
    deque <UID> req_lst;
    map <UID, set <FPATH>> file_lst;
    map <FPATH, file_info> fpath_finfo;
};
  

void error(string);
void *th_qt(void *);
void *th_srvc(void *);
void *cli_srvc(void *);
void snd_mssge(int, char []);


//map <UID, usr_info> uid__uinfo;
//map <GID, grps_info> gid__ginfo;


int cli_num;


 