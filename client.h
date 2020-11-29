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
#include <netinet/in.h>
#include <netdb.h> 
#include <openssl/sha.h>
#include <sys/stat.h>
   
#define FPATH string
#define UID string
#define BUFF 1024
#define CONN int
#define PORT int
#define IP string
#define LIMIT 15

using namespace std;

void error(string);
void client_service(int);
void rd_log(int);
void rd_out(int);
void *client_service(void *);
void *lis_cli_req(void *);
void *download_assist(void *);
void *_download_pi(void *);
void *fetch_piece_info(void *);
void *downlaod_cmd(void *);
string get_sha(string, int *, string);


struct file_info {

    FPATH fpath2;
    int f_pieces;
    int cu_pieces;
    long size;
    vector <bool> p_info;

    void _up() {
        p_info.resize(f_pieces, true);
        cu_pieces = f_pieces;
    }
    void _dw() {
        p_info.resize(f_pieces, false);
    }
};
 
struct usr_info {

    string uid;
    string ip;
    int port;
};

struct dwn_prior_data {
    
    string fpath;
    string fpath2;
    int pieces;
    long f_size;
    int n_uid;
    string gid;
    string f_sha;
    deque <string> p_sha;
    vector <usr_info> lst;
    void _lst() {
        lst.resize(n_uid);
    }
};
 
 
/*char serv_ip[15];
int serv_port;
char tracker_ip[15];
int tracker_port;

CONN conn;
bool drbuff_rdone = false;
int dwn_cnn;

map <FPATH, file_info> fp__fi;
map <FPATH, pair <string, string>> dwn_status;
map<int, map <int, map <CONN, pair <PORT, IP>>>> piece_map; */
