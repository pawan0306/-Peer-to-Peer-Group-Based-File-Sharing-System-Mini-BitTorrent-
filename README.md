# Peer-to-Peer Group Based File Sharing System (Mini BitTorrent) 
Developed a P2P multimedia file-sharing network using TCP that allows uploading and downloading files using from multiple available seeders. It also supports multithreaded client/server and tracker.

### Prerequisites
1. G++ compiler
   * ```sudo apt-get install g++```
2. openssl for SHA hashing
   * ```sudo apt-get install openssl```

#### To run the Tracker

```
./tracker tracker_info.txt tracker_no 

eg : ./tracker tracker_info.txt 1
```

#### To run the Client

```
./client ./client <IP>:<PORT> tracker_info.txt

eg : ./client 127.0.0.1:7600 tracker_info.txt
```

#### Working

1. At Least one tracker will always be online.
2. Client needs to create an account (userid and password) in order to be part of the
network.
3. Client can create any number of groups(groupid should be different) and hence will
be owner of those groups
4. Client needs to be part of the group from which it wants to download the file
5. Client will send join request to join a group
6. Owner Client Will Accept/Reject the request
7. After joining group ,client can see list of all the shareable files in the group
8. Client can share file in any group (note: file will not get uploaded to tracker but only
the <ip>:<port> of the client for that file)
9. Client can send the download command to tracker with the group name and
filename and tracker will send the details of the group members which are currently
sharing that particular file
10. After fetching the peer info from the tracker, client will communicate with peers about
the portions of the file they contain and hence accordingly decide which part of data
to take from which peer (You need to design your own Piece Selection Algorithm)
11. As soon as a piece of file gets downloaded it should be available for sharing
12. After logout, the client should temporarily stop sharing the currently shared files till
the next login
13. All trackers need to be in sync with each other

#### Functionality for client

* Create User Account: `create_user <user_id> <passwd>`
* Login: `login  <user_id> <passwd>`
* Create Group: `create_group <group_id>`
* Join Group: `join_group <group_id>`
* Leave Group: `leave_group <group_id>`
* List pending join: `list_requests <group_id>`
* Accept Group Joining Request: `accept_request <group_id> <user_id>`
* List All Group In Network: `list_groups`
* List All sharable Files In Group: `list_files <group_id>`
* Upload File: `upload_file <file_path> <group_id>`
* Download File: `download_file <group_id> <file_name> <destination_path>`
* Logout: `logout`
* Show_downloads: `show_downloads`
  * Output format:
    * [D] [grp_id] filename
    * [C] [grp_id] filename
    * D(Downloading), C(Complete)
* Stop sharing: `stop_share <group_id> <file_name>`

