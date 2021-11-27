# myftpd

#consist of a protocol file, client program and server program
#still on-going and update should be posted soon

#protocol specification file
- currentTime() function is used to get current time and print it into log
- it can be shared by both client and server

#a basic server program completed on 23 Nov 2021
- opcode 602218 is used to determine whether EOF is encounter
- opcode 0 is used to allow the client to download or upload file
- opcode 1 is used to deny the client to download or upload file
- opcode 2 is used to represent any unexcepted error while download or upload file
- opcode # is used to send an indicator to the server/client for cd/lcd command that no param is inputted

#by Russell and TY from Murdoch Singapore
