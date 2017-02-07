#ifndef ARGPASSHEADER_H
#define ARGPASSHEADER_H

//structure for sharing informations about arguments
struct arguments {
    char *args[2];
    int silent, verbose;
    char dirUpKey, actS, dirS;
    char *configFile;
};

struct arguments arguments;

void argumentsInit();
void argumentsApply(int argc,char** argv);

#endif