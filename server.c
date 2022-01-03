#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "errors.h"

typedef struct{
    struct user* fromUser;
    char text[10000];
    int  newMsg;
} message;

typedef struct {
    char username[10];
    char passwd[10];
    int numMsg;
    message *messages[10];
} user;

user *users[10];
int numberUsers = 0;

void trimNL(char* arr, int length) {
    for (int i = 0; i < length; ++i) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}



void authServ(int newsockfd) {

    char buffer[10];
    char name[10];
    char psswd[10];
    int n;
    int userFound = 0;

    bzero(buffer,10); //vynulujem buffer
    n = read(newsockfd, buffer, 10);
    trimNL(buffer,sizeof (buffer));
    strcpy(name,buffer);

    bzero(buffer,10); //vynulujem buffer
    n = read(newsockfd, buffer, 10);
    trimNL(buffer,sizeof (buffer));
    strcpy(psswd,buffer);

    for (int i = 0; i < numberUsers; ++i) {
        if((strcmp(users[i]->username,name) == 0) && (strcmp(users[i]->passwd,psswd) == 0)) {
            userFound = 1;
            break;
        }
    }
    if(userFound == 0) {
        const char* msg = "User not found!";
        n = write(newsockfd, msg, strlen(msg)+1);
        if (n < 0)
        {
            perror("Error writing to socket");
        }
    }

    if (userFound == 1) {
        const char* msg = "User sucesfully logged in";
        n = write(newsockfd, msg, strlen(msg)+1);
        if (n < 0)
        {
            perror("Error writing to socket");
        }
    }
}

void updateAccountsLoad() {
    FILE *filePointer ;
    char line[50];
    filePointer = fopen("users.txt", "r") ;

    while( fgets ( line, 50, filePointer ) != NULL )
    {
        char name[10], psswd[10];
        sscanf(line, "%s %s", name, psswd);
        trimNL(name,sizeof(name));
        trimNL(psswd,sizeof(psswd));
        if(numberUsers < 10) {
            user *new = (user *) malloc(sizeof (user));
            strcpy(new->username, name);
            strcpy(new->passwd,psswd);
            users[numberUsers] = new;
        }
        numberUsers++;
    }

    fclose(filePointer);
}

void updateAccountsSave() {
    FILE *filePointer;
    filePointer = fopen("users.txt", "w");

    for (int i = 0; i < numberUsers; ++i) {
        fputs(users[i]->username,filePointer);
        fputs(" ", filePointer);
        fputs(users[i]->passwd,filePointer);
        fputs("\n", filePointer);
    }

    fclose(filePointer);
}

/*void addMessage(struct user* toUser, char* text, struct user* fromUser)
{
    message *newMessage = (message *) malloc(sizeof (message));
    newMessage->newMsg = 1;
    strcpy(newMessage->text,text);
    newMessage->fromUser = fromUser;
    for (int i = 0; i < numberUsers; ++i) {
        if (users[i] == toUser) {
            users[i]->messages[users[i]->numMsg] = newMessage;
            users[i]->numMsg++;
            return;
        }
    }

}*/

void registerUser(int newsockfd) {
    user *new = (user *) malloc(sizeof (user));
    char buffer[10];
    int n;

    bzero(buffer,10); //vynulujem buffer
    n = read(newsockfd, buffer, 10);
    trimNL(buffer,sizeof (buffer));
    strcpy(new->username, buffer);

    bzero(buffer,10); //vynulujem buffer
    n = read(newsockfd, buffer, 10);
    trimNL(buffer,sizeof (buffer));
    strcpy(new->passwd,buffer);

    for (int i = 0; i < 10; ++i) {
        if(!users[i]) {
            users[i] = new;
            numberUsers++;
            updateAccountsSave();
            break;
        }
    }
    printf("New user: %s\n",new->username);
    printf("Current users: \n");
    for (int i = 0; i < numberUsers; ++i) {
        printf("%s\n",users[i]->username);
    }
    const char* msg = "User sucesfully registered";
    n = write(newsockfd, msg, strlen(msg)+1);
    if (n < 0)
    {
        perror("Error writing to socket");
    }
}

void welcomeServ(int newsockfd) {
    char buffer[10];
    int option;
    char* msg = "Welcome to chat app";
    chScWErr(write(newsockfd, msg, strlen(msg)+1));

    bzero(buffer,10); //vynulujem buffer
    chScRErr(read(newsockfd, buffer, 10));
    trimNL(buffer,1);
    option = atoi(buffer);
    switch (option) {
        case 1:
            msg = "Option 1\n";
            chScWErr(write(newsockfd, msg, strlen(msg)+1));
            registerUser(newsockfd);
            break;
        case 2:
            msg = "Option 2\n";
            chScWErr(write(newsockfd, msg, strlen(msg)+1));
            authServ(newsockfd);
            break;
        case 3:
            msg = "Option 3\n";
            chScWErr(write(newsockfd, msg, strlen(msg)+1));

            exit(0);
            break;

        default:
            msg = "Choose one option!\n";
            chScWErr(write(newsockfd, msg, strlen(msg)+1));
            welcomeServ(newsockfd);
    }

}

int server(int argc, char *argv[])
{
    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char buffer[256];

    if (argc < 2)
    {
        fprintf(stderr,"usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr)); //vynuluje strukuturu servaddr
    serv_addr.sin_family = AF_INET; // cely internet
    serv_addr.sin_addr.s_addr = INADDR_ANY; // povolene ip adresy - teraz vsetky
    serv_addr.sin_port = htons(atoi(argv[1])); //nastavi port litle to big endian

    chScCRErr(sockfd = socket(AF_INET, SOCK_STREAM, 0)); // vytvori socket
    chScBDErr(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))); // na socket namapujem strukturu (tento socket bude pracovat so spojeniami z celeho internetu na tomto porte)


    listen(sockfd, 5); //pasivny socket (nie na komunikaciu, ale na pripojenie pouzivatela) n:kolko klientov sa moze pripojit v jeden moment
    cli_len = sizeof(cli_addr);

    chScACErr(newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len)); //blokujuce systemove volanie, ked sa niekto pripoji, vrati novy socket na komunikaciu s pripojenym klientom


    //--------------------------------jadro aplikacie--------------------------------------------------------------------

    updateAccountsLoad();

    welcomeServ(newsockfd);
    //authServ(newsockfd);
    //registerUser(newsockfd);



    for (;;) {
        signal(SIGPIPE, SIG_IGN);

        bzero(buffer,256); //vynulujem buffer
        n = read(newsockfd, buffer, 255); //precitam data zo socketu a ulozim do buffra, je to blokujuce volanie, cakam dokedy klient nezada spravu
        if (n < 0)
        {
            perror("Error reading from socket");
            return 4;
        }
        //addMessage(users[0], buffer, users[0]);
        printf("Here is the message: %s\n", buffer);

        const char* msg = "I got your message";
        n = write(newsockfd, msg, strlen(msg)+1);
        if (n < 0)
        {
            perror("Error writing to socket");
            return 5;
        }
    }

    //--------------------------------jadro aplikacie--------------------------------------------------------------------

    close(newsockfd); // uzatvaram az ked chcem ukoncit komunikaciu
    close(sockfd);

    return 0;
}