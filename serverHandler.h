//
// Created by pc on 3. 1. 2022.
//

#ifndef CHAT_SERVERHANDLER_H
#define CHAT_SERVERHANDLER_H
void welcomeServ(int newsockfd);
void loggedMenuServ(int newsockfd);
void msgMenuServ(int newsockfd, char *username);
#endif //CHAT_SERVERHANDLER_H
