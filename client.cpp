#include "client.h"
 
char serv_ip[15];
int serv_port;
char tracker_ip[15];
int tracker_port;

CONN conn;
bool drbuff_rdone = false;
int dwn_cnn;

map <FPATH, file_info> fp__fi;
map <FPATH, pair <string, string>> dwn_status;
map<int, map <int, map <CONN, pair <PORT, IP>>>> piece_map; 
 


 
 
int main(int args, char *argv[]) {

    if(args != 3) {
        error("Invalid number of arguments");
    }
    
    char fbuff[15];
    memset(fbuff, '\0', sizeof(fbuff));
    FILE *f_fd = fopen(argv[2], "r");
    
    if(fgets(fbuff, 15, f_fd) == NULL) {
        error("cannot read file");
    }
    fbuff[strlen(fbuff)-1]='\0';
    strcpy(tracker_ip, fbuff);
    memset(fbuff, '\0', sizeof(fbuff));
    
    if(fgets(fbuff, 15, f_fd) == NULL) {
        error("cannot read file");
    }
    tracker_port = stoi(fbuff);
    int i = 0;
    string prt = "";
    while((serv_ip[i] = argv[1][i++]) != ':');
    while(argv[1][i] != '\0') prt += argv[1][i++];
    serv_port = stoi(prt);
    fclose(f_fd);
    
    /*strcpy(serv_ip,"127.0.0.1");
    serv_port=9005;
    strcpy(tracker_ip,"127.0.0.1");
    tracker_port=9002;*/




    pthread_t cli;
    pthread_t dwn;
    if(pthread_create(&cli, NULL, client_service, NULL) < 0) {
        error("cannot create client_service thread");
    }
    if(pthread_create(&dwn, NULL, download_assist, NULL) < 0) {
        error("cannot create downlaod_assist thread");
    }
    
    pthread_join(cli, NULL);
    pthread_join(dwn, NULL);

}

//******************************client service module**************************
 
void *client_service(void *attr) {

    int tracker_fd;
    struct sockaddr_in tracker_addr; 
    struct hostent *tracker_n;
    if((tracker_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("ERROR opening socket");
    }
    if((tracker_n = gethostbyname(tracker_ip)) == NULL) {
        error("no such host");
    }

    bzero((char *) &tracker_addr, sizeof(tracker_addr));
    tracker_addr.sin_family = AF_INET;
    bcopy((char *)tracker_n->h_addr, (char *)&tracker_addr.sin_addr.s_addr, tracker_n->h_length);
    tracker_addr.sin_port = htons(tracker_port);
    if (connect(tracker_fd,(struct sockaddr *) &tracker_addr,sizeof(tracker_addr)) < 0) {
        error("ERROR connecting");
    }
    char wbuff[256];
    int dwn = 0;
    pthread_t dwn_cmd[100];

    while(1) {

        char buff[256];
        memset(buff, 0, 256);
        string cbuff;
        fgets(buff,255,stdin);
        buff[strlen(buff)-1]='\0';
        cbuff=(string)buff;
        istringstream ss(cbuff);
        string cmd;
        ss >> cmd;
        if(cmd == "create_user") {

            cbuff += ' ' + (string)serv_ip + ' ' + to_string(serv_port);
            if(write(tracker_fd, cbuff.c_str(), cbuff.length()) < 0) {
                cout << "cannot write";
            }
            rd_log(tracker_fd);
            continue;
        }
 
        if(cmd == "login" || cmd == "create_group" || cmd == "join_group" || cmd == "leave_group" || cmd == "accept_request") {

            if(write(tracker_fd, cbuff.c_str(), cbuff.length()) < 0) {
                cout << "cannot write";
            }
            rd_log(tracker_fd);
            continue;
        }

        if(cmd == "list_requests" || cmd == "list_files" || cmd == "list_groups")  {

            if(write(tracker_fd, cbuff.c_str(), cbuff.length()) < 0) {
                cout << "cannot write";
            }
            rd_out(tracker_fd);
            continue;
        }

        if(cmd == "upload_file") {

            if(write(tracker_fd, cbuff.c_str(), cbuff.length()) < 0) {
                cout << "cannot write";
            }
            string fpath;
            string gid;
            ss >> fpath;
            ss >> gid;
            struct stat sb;
            if(stat(fpath.c_str(), &sb) < 0) {
                cout << "error in file path";
                continue;
            }
            int p;
            string mssge = get_sha(fpath, &p, gid);
            if(mssge.empty()) { 
                continue; 
            }
            mssge = to_string(sb.st_size) + ' ' + mssge;
            
            if(fp__fi.find(fpath) == fp__fi.end()) {
                fp__fi[fpath];
                auto i = fp__fi.find(fpath);
                i->second.fpath2 = fpath;
                i->second.f_pieces = p;
                i->second._up();
                i->second.size = sb.st_size;     
            } 
                
            


            if(write(tracker_fd, mssge.c_str(), mssge.length()) < 0) {
                cout << "cannot write";
            }
            rd_log(tracker_fd);
            continue;
        }

        if(cmd == "download_file") {

            if(write(tracker_fd, cbuff.c_str(), cbuff.length()) < 0) {
                cout << "cannot write";
            }
            char rbuff[50000];
            if(read(tracker_fd, rbuff, sizeof(rbuff)) < 0) {
                cout << "cannot read log";
                continue;
            }
            if(rbuff == "nosuchfile") {
                cout<<"FILE DOESNT EXIST";
                continue;
            }
            dwn_cnn++;

            string gid;
            string fpath;
            string fpath2;
            ss >> gid;
            ss >> fpath;
            ss >> fpath2;
            string dwn_str = to_string(dwn_cnn) + ' ' + gid + ' ' + fpath + ' ' + fpath2 + ' ';
            for(int i = 0; i < 50000; i++) {
                dwn_str += rbuff[i];
            } 

            if(pthread_create(&dwn_cmd[dwn++], NULL, downlaod_cmd, &dwn_str) < 0) {
                cout<<"error in creating thread download cmd";
                continue;
            }

            cout << "processing...\n";
            
            while(1) {
                if(drbuff_rdone) {
                    cout << "Downloading started";
                    drbuff_rdone = false;
                    break;
                }
            }
            continue;
            
        } 

        if(cmd == "show_downloads") {

                 
            for(auto i = dwn_status.begin(); i != dwn_status.end(); i++) {
                cout << (*i).second.second<<' '<<(*i).second.first<<' '<<(*i).first<<'\n';
            }
                 
            continue;
        }
        if(cmd == "stop_share") {

            if(write(tracker_fd, cbuff.c_str(), cbuff.length()) < 0) {
                cout << "cannot write";
            }
            string fpath;
            ss >> fpath;
            ss >> fpath;
                
            fp__fi.erase(fpath);
            
            rd_log(tracker_fd);
            continue;
        }

        if(cmd == "logout") {   
                               
            if(write(tracker_fd, cbuff.c_str(), cbuff.length()) < 0) {
                cout << "cannot write";
            }                  
            rd_log(tracker_fd);
            continue;
        }
    }
    return NULL;
}

//***********************************download cmd thread module**********************************

void *downlaod_cmd(void* attr) {

    string buff;
    for(int i = 0; i < 50000; i++) {
        buff += (*(string*)attr)[i];
    }

    istringstream ss(buff);
    dwn_prior_data dta;
    int _dwn_cnn;
    ss >> _dwn_cnn;
    ss >> dta.gid;
    ss >> dta.fpath;

    
    dwn_status[dta.fpath] = {dta.gid, "[D]"};
    
    
    drbuff_rdone = true;

    ss >> dta.fpath2;
    ss >> dta.pieces;
    ss >> dta.f_size;
    ss >> dta.n_uid;
    dta._lst();
    

            
    for(int i = 0; i < dta.n_uid; i++) {
        ss >> dta.lst[i].uid;
        ss >> dta.lst[i].ip;
        ss >> dta.lst[i].port;
    }
    int temp = 7 + dta.n_uid*3;
    int i = 0;
    for(int count = 0; count < temp; i++) {
        if(buff[i] == ' ') count++;
    }
    temp = 0;
    for(; temp < 20; temp++, i++) {
        dta.f_sha += buff[i];
    }
    string tmp_str = "";
    temp = 0;
    int temp2 = 0;
    for(; temp < (dta.pieces * 20); temp++, i++) {
        tmp_str += buff[i];
        temp2++;
        if(temp2 == 20) {
            dta.p_sha.push_back(tmp_str);
            tmp_str = "";
            temp2 = 0;
        }
    }
    //free(rbuff);
    pthread_t get_dta_th[dta.n_uid];
    string uid_prt_ip[dta.n_uid];
    int co = 0;
    while(co < dta.n_uid) {
        uid_prt_ip[co] = "" + dta.lst[co].uid + ' ' + to_string(_dwn_cnn) + ' ' + dta.fpath + ' ' + dta.lst[co].ip + ' ' + to_string(dta.lst[co].port);
        pthread_create(&get_dta_th[co], NULL, fetch_piece_info, &uid_prt_ip[co]);
        co++;
    }
    co = 0;
    while(co < dta.n_uid) {
        pthread_join(get_dta_th[co], NULL);
        co++;
    }
    
    if(piece_map[dwn_cnn].size() != dta.pieces) {
        //cout<< "cannot download the file";
        return NULL;
    }
         

        
    
    fp__fi[dta.fpath];
    auto mptmp = fp__fi[dta.fpath];
    mptmp.cu_pieces = 0;
    mptmp.f_pieces = dta.pieces;
    mptmp.fpath2 = dta.fpath2;    
    mptmp.size = dta.f_size;
    mptmp._dw();

        

    FILE *f_fd = fopen(dta.fpath2.c_str(), "a+");
    char *rbuff = new char[1000];
    co = (dta.f_size / 1000);
    while(co--) {
        fwrite(rbuff, 1, 1000, f_fd);
    }
    fwrite(rbuff, 1, (dta.f_size % 1000), f_fd);
    fclose(f_fd);

    cout<<"DOWNLOAD STARTED\n";

    deque <int> err_p;
    unsigned char psha[dta.pieces][20];
    pthread_t get_dta_th2[dta.pieces];
    string uid_prt_ip2[dta.pieces];
    co = 0;
    while(co < dta.pieces) {
        uid_prt_ip2[co] = "" + to_string(co) + ' ' + dta.fpath + ' ' + dta.fpath2 + ' ' + piece_map[_dwn_cnn][co].begin()->second.second + ' ' + to_string(piece_map[_dwn_cnn][co].begin()->second.first);
        pthread_create(&get_dta_th2[co], NULL, _download_pi, &uid_prt_ip2[co]);
        co++;
    }
    co = 0;
    while(co < dta.pieces) {
        pthread_join(get_dta_th[co], (void**)&psha[co]);
        for(int i = 0; i < 20; i++) {
            if(psha[co][i] != dta.p_sha[co][i]) {
                err_p.push_back(co);
            }
        }
        co++;
    }
    free(rbuff);
    
    //   *****************do not erase it 

   /* while(!err_p.empty()) {
        free(get_dta_th);
        free(uid_prt_ip);
        get_dta_th = new pthread_t[err_p.size()];
        uid_prt_ip = new string[err_p.size()];
        co = 0;
        int ln = err_p.size();
        while(!err_p.empty()) {
            uid_prt_ip[co] = "" + to_string(co) + ' ' + dta.fpath + ' ' + dta.fpath2 + ' ' + piece_map[_dwn_cnn][err_p.front()].begin()->second.second + ' ' + to_string(piece_map[_dwn_cnn][err_p.front()].begin()->second.first);
            pthread_create(&get_dta_th[co], NULL, _download_pi, &uid_prt_ip);
            co++;
            err_p.pop_front();
        }
        co = 0;
        while(co < ln) {
            pthread_join(get_dta_th[co], (void**)&psha[co]);
            for(int i = 0; i < 20; i++) {
                if(psha[co][i] != dta.p_sha[co][i]) {
                    err_p.push_back(co);
                }
            }
            co++;
        }
 
    }*/
    SHA_CTX sc;
    f_fd = fopen(dta.fpath2.c_str(), "r");
    rbuff = new char[1000];
    while(!feof(f_fd)) {
        memset(rbuff, 0, 1000);
        fread(rbuff, 1, 1000, f_fd);
        SHA1_Update(&sc, rbuff, strlen(rbuff));
    }
    unsigned char whole_sha[SHA_DIGEST_LENGTH];
    SHA1_Final(whole_sha, &sc);
    fclose(f_fd);
    bool clr = false;
    for(int i = 0; i < 20; i++) {
        if(whole_sha[i] != dta.f_sha[i]) {
            cout<<dta.fpath<<"::FILE IS CORRUPTED. PLEASE DELETE IT";
            clr = true;
            break;
        }
    }
    cout<<"Download finished for :"<<dta.fpath<<"\n";
    if(!clr) {
        
        dwn_status[dta.fpath] = {dta.gid, "[C]"};
             
    }

    
    piece_map.erase(_dwn_cnn);
         
    return NULL;

}

//***********************************peice downloading thread module************************

void* _download_pi(void* attr) {

    string inf = *(string*)attr;
    char d_ip[15];
    int d_port;
    int pnum;
    string fpth;
    string fpth2;
    struct sockaddr_in usr_addr; 
    struct hostent *usr_n;
    int _fd;
    SHA_CTX sc;

    istringstream ss(inf);
    ss >> pnum;
    ss >> fpth;
    ss >> fpth2;
    ss >> d_ip;
    ss >> d_port;

    if((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout<<"ERROR opening socket";
        return NULL;
    }
    if((usr_n = gethostbyname(d_ip)) == NULL) {
        cout<<"no such host "<<d_ip;
        return NULL;
    }

    memset((char *) &usr_addr, '\0', sizeof(usr_addr));
    usr_addr.sin_family = AF_INET;
    bcopy((char *)usr_n->h_addr, (char *)&usr_addr.sin_addr.s_addr, usr_n->h_length);
    usr_addr.sin_port = htons(d_port);
    if (connect(_fd,(struct sockaddr *) &usr_addr,sizeof(usr_addr)) < 0) {
        cout<<"ERROR connecting";
        return NULL;
    }
    
    string tmp="dwn";
    tmp += ' ' + to_string(pnum) + ' ' + fpth;
    if(write(_fd, tmp.c_str(), tmp.length()) < 0) {
        cout << "cannot write";
        return NULL;
    }

    SHA1_Init(&sc);
    int i = 0;
    FILE *f_fd = fopen(fpth2.c_str(), "a");
    fseek(f_fd, (pnum)*512, SEEK_SET);
    char *rbuff = new char[8192];
    while(i < 64) {
        memset(rbuff, 0, 8192);
        if(read(_fd, rbuff, 8192) < 0) {
            cout<<"error reading::file is corrupted please delete";
            return NULL;
        }
        int len = strlen(rbuff);
        SHA1_Update(&sc, rbuff, len);
        fwrite(rbuff, 1, len, f_fd);
        if(len < 8192){
            break;
        }
        i++;
    }
    unsigned char sha[SHA_DIGEST_LENGTH];
    SHA1_Final(sha, &sc);
    free(rbuff);         
    
    fp__fi[fpth].cu_pieces++;
    fp__fi[fpth].p_info[pnum] = true;
        

    return sha;


}

//********************************fetch peice info module**********************************


void* fetch_piece_info(void *attr) {

    int _fd;
    string usr_ip;
    int usr_port;
    int _dwn_cnn;
    string fpath;
    string uid;

    istringstream ss(*(string*)attr);
    ss >> uid;
    ss >> _dwn_cnn;
    ss >> fpath;
    ss >> usr_ip;
    ss >> usr_port;
    struct sockaddr_in usr_addr; 
    struct hostent *usr_n;
    if((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout<<"ERROR opening socket";
        return NULL;
    }
    if((usr_n = gethostbyname(usr_ip.c_str())) == NULL) {
        cout<<"no such host "<<usr_ip;
        return NULL;
    }
     
    memset((char *) &usr_addr, '\0', sizeof(usr_addr));
    usr_addr.sin_family = AF_INET;
    bcopy((char *)usr_n->h_addr, (char *)&usr_addr.sin_addr.s_addr, usr_n->h_length);
    usr_addr.sin_port = htons(usr_port);
    if (connect(_fd,(struct sockaddr *) &usr_addr,sizeof(usr_addr)) < 0) {
        cout<<"ERROR connecting";
        return NULL;
    }
    string locstr = "fileinfo " + fpath;
    if(write(_fd, locstr.c_str(), locstr.length()) < 0) {
        cout << "cannot write";
        return NULL;
    }
    char pi_buff[2000];
    if(read(_fd, pi_buff, 2000) < 0) {
        cout << "cannot read";
        return NULL;
    }
    istringstream ss2(pi_buff);
    int traffic_usr;
    ss2 >> traffic_usr;
    ss2 >> locstr;     
    while(ss2 >> locstr) {
        piece_map[_dwn_cnn][stoi(locstr)][traffic_usr]={usr_port, usr_ip};
    }
    return NULL;
    
}

//************************************download_assist module**************************************


void *download_assist(void *attr) {

    int serv_sockfd;
    struct sockaddr_in serv_addr;
    int op=1;
    if((serv_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("failed to create socket");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(serv_port);
    
    if(setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &op, sizeof(op))) { 
        error("failed to assign socket");
    }
    
    if(bind(serv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("failed to bind socket");
    }

    socklen_t clilen;
    int newsockfd;
    int t = 0;
    pthread_t id[1000];
    string arg[1000];
    listen(serv_sockfd, LIMIT);
    while(t < 1000){
        clilen = sizeof(serv_addr);
        newsockfd = accept(serv_sockfd, (struct sockaddr *) &serv_addr, &clilen);
        if (newsockfd < 0) {
            error("ERROR on accept");
        }
        arg[t] = to_string(newsockfd);
        pthread_create(&id[t], NULL, lis_cli_req, &arg[t]);
        t++;    
    }
    t = 0;
    while(t < 1000) {
        pthread_join(id[t], NULL);
    }
    return NULL;

}
//**********************************handling clients requests module**************************


void* lis_cli_req(void* attr) {

    int _fd = stoi(*(string*)attr);                          
    char buff[256];
    if(read(_fd, buff, 256) < 0) {
        error("read error during client reqs");
    }
    string cmd;
    istringstream ss(buff);
    ss >> cmd;
    string mssge="";

    if(cmd == "fileinfo") {

        string fl;
        ss >> fl;

             
        
        auto tmp = fp__fi[fl];
        mssge += to_string(conn) + ' ' + to_string(tmp.f_pieces) + ' ';
        for(int i = 0; i < tmp.f_pieces; i++) {
            if(tmp.p_info[i]) {
                mssge += to_string(i) + ' ';
            }
        }
            
        

        if(write(_fd, mssge.c_str(), mssge.length()) < 0){
            error("write error during client reqs");
        }
        return NULL;
    }
    if(cmd == "dwn") {

        
        conn++;
        int pnm;
        string fl;
        istringstream ss;
        ss >> pnm;
        ss >> fl;
        char rbuff[8192];
        FILE *fld = fopen(fp__fi[fl].fpath2.c_str(), "r");
        fseek(fld, pnm*512, SEEK_SET);
        int i = 0;
        while(!feof(fld) && i < 64) {
            memset(rbuff, 0, 8192);
            fread(rbuff, 1, 8192, fld);
            int len = strlen(rbuff);
            if(write(_fd, rbuff, len) < 0){
                error("error in writing during client down");
            }
            if(len < 8192) {
                break;
            }
            i++;
        }
        
        conn--;
        
        return NULL;
        
    }

    

}


//************************************get SHA-1 hash of the file*********************************


string get_sha(string fpath, int *piece, string gid) {

    unsigned char _tot[SHA_DIGEST_LENGTH];
    unsigned char _pi[SHA_DIGEST_LENGTH];
    SHA_CTX tot;
    SHA_CTX pi;
    string p_hash = "";
    string tot_hash = "";
    string mssge = "";

    char fbuff[BUFF];
    FILE *f_fd;
    if((f_fd = fopen(fpath.c_str(), "rb")) < 0) {
        cout << "File doesnt exist";
        return NULL;
    }
    int i = 0;
    

    if(!SHA1_Init(&tot)){error("SHA1 init tot failed");}
    if(!SHA1_Init(&pi)){error("SHA1 init tot failed");}
    while(!feof(f_fd)) {

        i++;
        memset(fbuff, 0, sizeof(fbuff));
        if(fread(&fbuff, 1, BUFF, f_fd) < 1) {
            cout << "cannot read file";
            return NULL;
        }
        
        if(!SHA1_Update(&tot, &fbuff, BUFF)) { error("SHA1 update tot failed"); }
        if(!SHA1_Update(&pi, &fbuff, BUFF)) { error("SHA1 update pi failed"); }
        
        if(i == 512) {
            (*piece)++;
            if(!SHA1_Final(_pi, &pi)) { error("SHA1 final pi failed"); }
            for(int i = 0; i < 20; i++) {
                p_hash += _pi[i];
            }
            memset(_pi, 0, sizeof(_pi));
            if(!SHA1_Init(&pi)) { error("SHA! init pi failed"); }
            i = 0;
        }
    }
    if(i < 512){
        (*piece)++;
        if(!SHA1_Final(_pi, &pi)) { error("SHA! init pi failed"); }
        for(int i = 0; i < 20; i++) {
            p_hash += _pi[i];
        }
    }
    SHA1_Final(_tot, &tot);
    for(int i = 0; i < 20; i++) {
        tot_hash += _tot[i];
    }
    string s="upload_file";
    mssge += to_string(*piece) + ' ' + tot_hash + p_hash;
    //for(int i = mssge.length(), j = 0 ; j < p_hash.length(); j++, i++) {
    //    mssge += p_hash[j];
    //}
    fclose(f_fd);
    return mssge;

}


void rd_out(int fd) {
    char rbuff[50000];
    memset(rbuff, '\0', sizeof(rbuff));
    if(read(fd, rbuff, sizeof(rbuff)) < 0) {
        cout<< "cannot read";
        return;
    }
    istringstream ss(rbuff);
    string s;
    while(ss >> s) {
        cout << s << "\n";
    }
}

void rd_log(int fd) {
    char log_buff[256];
    memset(log_buff, '\0', sizeof(log_buff));
    if(read(fd, log_buff, sizeof(log_buff)) < 0) {
        cout << "cannot read log";
        return;
    }
    printf("%s\n",log_buff);
    return;
}

void error(string s) {
    cout << s;
    exit(1);
}
