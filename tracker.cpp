#include "tracker.h"

 
//map <int, pthread_t> cfd__pid;
map <UID, usr_info> uid__uinfo;
map <GID, grps_info> gid__ginfo;
char tracker_ip[15];
int tracker_port;

bool flag_ex = false;

int main(int args, char *argv[]) {
 
    string tracker_id;
    if(args != 3) {
        error("Invalid number of arguments");
    }
    
    char fbuff[15];
    memset(fbuff, '\0', sizeof(fbuff));
    tracker_id = argv[2];
    //change it 
    FILE *f_fd = fopen(argv[1], "r");
    //
    if(fgets(fbuff, 15, f_fd) == NULL) {
        error("cannot read file");
    }
    strcpy(tracker_ip, fbuff);
    memset(fbuff, '\0', sizeof(fbuff));
    
    if(fgets(fbuff, 10, f_fd) == NULL) {
        error("cannot read file");
    }
    tracker_port = stoi(fbuff);

    fclose(f_fd);


//***************************socket creation*********************************
     
     
    int serv_sockfd;
    struct sockaddr_in serv_addr;
    int op=1;
    if((serv_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("failed to create socket");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    

    //*************** making socket unblocking**************************************

    //fcntl(serv_sockfd, F_SETFL, fcntl(serv_sockfd, F_GETFL, 0) | O_NONBLOCK);
    
    //*********************************************************************************


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(tracker_port);
    
    if(setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &op, sizeof(op))) { 
        error("failed to assign socket");
    }
    
    if(bind(serv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("failed to bind socket");
    }


//********************************threads for listening cleints requets and quit command****************************    
    
    pthread_t id_qt_cmd;
    pthread_t id_srvc;

    if(pthread_create(&id_qt_cmd, NULL, th_qt, NULL) != 0) {
        error("failed to create thread-quit");
    }


    if(pthread_create(&id_srvc, NULL, th_srvc, &serv_sockfd) != 0) {
        error("failed to create thread serv");
    }

    pthread_join(id_qt_cmd, NULL);
    //pthread_join(id_srvc, NULL);
    
    
            

}


//*************************************** quit cmd listner thread definition****************************


void *th_qt(void *attr) {

    while(1) {

        string s;
        cin>>s;
        if(s=="quit"){
            flag_ex = true;
            break;
            exit(0);
        }    
    }
    pthread_exit(NULL);
} 


//***************************************listen to clients and establish connections************************************


void *th_srvc(void *attr) {

    int serv_sockfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    struct sockaddr_in serv_addr;
    int op=1;
    if((serv_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("failed to create socket");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(tracker_port);
    if(setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &op, sizeof(op))) { 
        error("failed to assign socket");
    }
    if(bind(serv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("failed to bind socket");
    }
    
    //
    int newsockfd[1000] = {0}; 
    pthread_t pid[1000];
    //int sockfd = *(int*) serv_sockfd;
    listen(serv_sockfd, LIMIT);
    int i = 0;
    while(1) {

        if(flag_ex) {
            while(cli_num != 0)
                pthread_exit(NULL);
        } 
        clilen = sizeof(cli_addr);
        if((newsockfd[i] = accept(serv_sockfd, (struct sockaddr*) &cli_addr, &clilen)) <= 0) {
            cout<<"failed to accept "<<int(cli_addr.sin_port);
        }
        if(pthread_create(&pid[i], NULL, cli_srvc, &newsockfd[i]) < 0) {
            cout<<"failed to create client thread";
        }
        cli_num++;
    }
}

//************************************clients services thread definition********************************

void *cli_srvc(void *attr) {

    char rbuff[256];
    int com_fd = *(int*)attr;
    //map<UID ,usr_info>::iterator uid_itr;
    UID _uid = "";
    bool lgin = false;

    while(1) {

        memset(rbuff, 0, 256);
        if(read(com_fd, rbuff, 255) < 0) {
            cout<<"unable to read";
            continue;
        }
        string cmd;
        istringstream ss(rbuff);
        ss >> cmd;

        //***********************************************************************************
        
        if(cmd == "create_user") {

            string uid;
            string pwd;
            string usr_ip;
            int usr_port;
            ss >> uid;
            ss >> pwd;
            ss >> usr_ip;
            ss >> usr_port;
             
            if(uid__uinfo.find(uid) != uid__uinfo.end()) {
                snd_mssge(com_fd, "SERVER::USERID ALREADY EXISTS");
                continue;
            }
            uid__uinfo[uid].id = uid;
            auto i = uid__uinfo.find(uid);
            i->second.pswd = pwd;
            i->second.ip = usr_ip;
            i->second.port = usr_port;
            i->second.active = false;
             
            snd_mssge(com_fd, "SERVER::REGISTERED SUCCESSFULLY. PLEASE LOGIN");
        }

        //***************************************************************************************
        
        if(cmd == "login") {

            string uid;
            string pwd;
            ss >> uid;
            ss >> pwd;
            
            if(uid__uinfo.find(uid) == uid__uinfo.end()) {
                snd_mssge(com_fd, "SERVER::USERID DOESNT EXISTS");
                continue;
            }
            if(uid__uinfo[uid].pswd != pwd) {
                snd_mssge(com_fd, "SERVER::INCORRECT PASSWORD");
                continue;
            }
            _uid = uid;
            lgin =true;
            auto i = uid__uinfo.find(uid);
            i->second.active = true;
             
            snd_mssge(com_fd, "SERVER::LOGGED IN SUCCESSFULLY");            

        }

        //******************************************************************************************

        if(cmd == "create_group") {

            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }

            string gid;
            ss >> gid;

              
            if(gid__ginfo.find(gid) != gid__ginfo.end()) {
                snd_mssge(com_fd, "SERVER::GROUPID ALREADY EXISTS");
                continue;
            }
               
            
              
            gid__ginfo[gid];
            auto u = gid__ginfo.find(gid);
               

                
              
            auto j = uid__uinfo.find(_uid);
            j->second.grps_owns.insert(gid);
            j->second.memberof.insert(gid);
            u->second.ownr_id = _uid;
            u->second.gid = gid;
            u->second.members.insert(_uid);
               
             
            snd_mssge(com_fd, "SERVER::GROUP CREATED SUCCESSFULLY");

        }
        
        //********************************************************************************************

        if(cmd == "join_group") {
 
            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }

            string gid;
            ss >> gid;

              
            if(gid__ginfo.find(gid) == gid__ginfo.end()) {
                snd_mssge(com_fd, "SERVER::GROUP DOESNT EXISTS");
                continue;
            }
            gid__ginfo[gid].req_lst.push_back(_uid);
               

            snd_mssge(com_fd, "SERVER::REQUEST SENT SUCCESSFULLY");
        
        }

        //***********************************************************************************************

        if(cmd == "leave_group") {

            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }

            string gid;
            ss >> gid;
              
            if(gid__ginfo.find(gid) == gid__ginfo.end()) {
                snd_mssge(com_fd, "SERVER::GROUP DOESNT EXISTS");
                continue;
            }

            if(gid__ginfo[gid].members.find(_uid) == gid__ginfo[gid].members.end()) {
                snd_mssge(com_fd, "SERVER::YOU ARE NOT MEMBER OF THE GROUP");
                continue;
            }
               

                
            auto u = uid__uinfo.find(_uid);
            u->second.memberof.erase(gid);
            u->second.grps_owns.erase(gid);
            u->second.file_shares.erase(gid);
             

              
            auto i = gid__ginfo.find(gid);
            if(i->second.ownr_id == _uid) {
                gid__ginfo.erase(gid);
            }
            else {
                i->second.members.erase(_uid);
                auto j = i->second.file_lst.find(_uid);
                for (auto k = j->second.begin(); k != j->second.end(); k++) {
                    i->second.fpath_finfo.erase((*k));
                }
            }
               
            
            snd_mssge(com_fd, "SERVER::SUCCESSFULLY LEFT THE GROUP");

        }

        //*******************************************************************************************


        if(cmd == "list_requests") {

            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }

            string gid;
            ss >> gid;
                
            if(uid__uinfo[_uid].grps_owns.find(gid) == uid__uinfo[_uid].grps_owns.end()) {
                snd_mssge(com_fd, "SERVER::YOU ARE NOT OWNER OF THIS GROUP");
                continue;
            }
             
              
            auto lst = gid__ginfo[gid].req_lst;
            string mssge;
            for(auto i = lst.begin(); i != lst.end(); i++) {
                mssge += (*i) + ' ';
            }
               
            write(com_fd, mssge.c_str(), mssge.length());

        }
        
        //******************************************************************************************

        if(cmd == "accept_request") {

            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }

            string gid;
            string uid;
            ss >> gid;
            ss >> uid;
            
                
              
            if(uid__uinfo[_uid].grps_owns.find(gid) == uid__uinfo[_uid].grps_owns.end()) {
                snd_mssge(com_fd, "SERVER::YOU ARE NOT OWNER OF THIS GROUP");
                continue;
            }
            
            if(gid__ginfo.find(gid) == gid__ginfo.end()) {
                snd_mssge(com_fd, "SERVER::GROUP DOESNT EXISTS");
                continue;
            }
            auto grp = gid__ginfo.find(gid);
            bool flag = false;
            auto pos = grp->second.req_lst.begin();
            for(auto i = grp->second.req_lst.begin(); i != grp->second.req_lst.end(); i++) {
                if(*i == uid) {
                    flag = true;
                    pos = i;
                }
            } 
            if(!flag) {
                snd_mssge(com_fd, "SERVER::USER DOESNT EXISTS IN THE LIST");
                continue;
            }

            grp->second.members.insert(uid);
            grp->second.req_lst.erase(pos);
            uid__uinfo[uid].memberof.insert(gid);
            
             
               
        }

        //****************************************************************************************

        if(cmd == "list_groups") {
            
            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }

            string mssge = "";
              
            for(auto i = gid__ginfo.begin(); i != gid__ginfo.end(); i++) {
                mssge += (*i).first + ' ';
            }    
               
            write(com_fd, mssge.c_str(), mssge.length());
                 
        }

        //*********************************************************************************************************


        if(cmd == "list_files") {
            
            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }

            string gid;
            ss >> gid;
              
            if(gid__ginfo.find(gid) == gid__ginfo.end()) {
                snd_mssge(com_fd, "SERVER::GROUP DOESNT EXISTS");
                continue;
            }
                
            string mssge = "";
            auto i = gid__ginfo.find(gid);
            for(auto j = i->second.fpath_finfo.begin(); j != i->second.fpath_finfo.end(); j++) {
                bool flag = false;
                for(auto k = (*j).second.has_file.begin(); k != (*j).second.has_file.end(); k++) {
                    if(uid__uinfo[(*k)].active == true) {
                        flag = true;
                        break; 
                    }
                }
                if(flag)
                    mssge += j->first + ' ';
            }
             
               
            write(com_fd, mssge.c_str(), mssge.length());
            
        }

        //*****************************************************************************************


        if(cmd == "upload_file") {

            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }

            char rbuff[50000];
            memset(rbuff, 0, 50000);
            string fpath;
            string gid;
            long fsize;
            int pieces;
            ss >> fpath;
            ss >> gid;

            if(read(com_fd, rbuff, 50000) < 0) {
                cout<<"unable to read files";
                continue;
            }

            istringstream data(rbuff);
            data >> fsize;
            data >> pieces;
                
              
            uid__uinfo[_uid].file_shares[gid].insert(fpath);
            auto i = gid__ginfo.find(gid);
            i->second.file_lst[_uid];
            i->second.file_lst[_uid].insert(fpath);
            i->second.fpath_finfo[fpath];
            auto j = i->second.fpath_finfo.find(fpath);
            
            j->second.fpath = fpath;
            j->second.pieces = pieces;
            j->second.f_size = fsize;
            j->second.has_file.insert(_uid);
            string temp = "";
            int count = 0;
            int k = 0;
            for( ; count < 2; k++) {
                if(rbuff[k] == ' ') count++;
            }
            count = 0;
            for(; count < 20; count++, k++) {
                temp += rbuff[k];
            }
            j->second.f_SHA = temp;
            temp = "";
            count = 0;
            int count2 = 0; 
            for(; count < (pieces * 20); count++, k++) {
                temp += rbuff[k];
                count2++;   
                if(count2 == 20) {
                    j->second.p_SHA.push_back(temp);
                    temp = "";
                    count2 = 0;
                }
            }
             
               
            snd_mssge(com_fd, "SERVER::FILE UPLOADED SUCCESSFULLY");
        }

        //******************************************************************************************


        if(cmd == "download_file") {

            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }
            string gid;
            string fpath;
            ss >> gid;
            ss >> fpath;
            string mssge = "";
            int count = 0;
              
                
            if(gid__ginfo[gid].fpath_finfo.find(fpath) == gid__ginfo[gid].fpath_finfo.end()) {
                snd_mssge(com_fd, "nosuchfile");
                continue;
            }
            auto i = gid__ginfo[gid].fpath_finfo.find(fpath);
            //mssge += i.pieces + ' ' + i.f_size + ' ' + i.has_file.size() + ' ';
            for(auto j = i->second.has_file.begin(); j != i->second.has_file.end(); j++) {
                if(uid__uinfo[*j].active){
                    count++;
                    mssge += uid__uinfo[*j].id + ' ' + uid__uinfo[*j].ip + ' ' + to_string(uid__uinfo[*j].port) + ' ';
                }
            }
            mssge = to_string(i->second.pieces) + ' ' + to_string(i->second.f_size) + ' ' + to_string(count) + ' ' + mssge;
            mssge += i->second.f_SHA;
            for(auto j = i->second.p_SHA.begin(); j != i->second.p_SHA.end(); j++) {
                mssge += *j;
            i->second.has_file.insert(_uid);
            }
             
               
            write(com_fd, mssge.c_str(), mssge.length());
        
        }

        //******************************************************************************************


        if(cmd == "logout") {
            
            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }
                
            uid__uinfo[_uid].active = false;
             
            lgin = NULL;

            break;
        }

        //*********************************************************************************************


        if(cmd == "stop_share") {

            if(!lgin) {
                snd_mssge(com_fd, "SERVER::PLEASE LOGIN");
                continue;
            }
            string gid;
            string fpath;
            ss >> gid;
            ss >> fpath;

                
            uid__uinfo[_uid].file_shares[gid].erase(fpath);
             

              
            gid__ginfo[gid].file_lst[_uid].erase(fpath);
            gid__ginfo[gid].fpath_finfo.erase(fpath);
               
            snd_mssge(com_fd, "SERVER::FILE HAS BEEN REMOVED");
        }
    }
    
    
    cli_num--;
 
}

void snd_mssge(int com_fd, char ch[]) {

    write(com_fd, ch, strlen(ch));
}

void error(string s){
    cout<<s;
    exit(1);
}
 