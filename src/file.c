#include<stdio.h>
#include<stdlib.h>

#include"file.h"
#include"errors.h"
#include"data.h"
#include"string.h"

char** getLines(FILE* file) {
    int lines_to_allocate = 64;
    int line_size_to_allocate = 100;
    
    if (file==NULL) {
        fileError();
    }

    char** lines = (char**)malloc(sizeof(char*)*lines_to_allocate);
    if (lines==NULL) {
        memError();
    }
    
    int i;
    for (i = 0; 1; i++) {
        //do we need to allocate more lines?
        if (i >= lines_to_allocate) {
            lines_to_allocate = lines_to_allocate * 2;
            lines = (char**)realloc(lines,sizeof(char*)*lines_to_allocate);
            if (lines==NULL) {
                memError();
            }
        }
        
        //allocate line, and make sure we succeed
        lines[i] = malloc(line_size_to_allocate);
        if (lines[i]==NULL) {
            memError();
        }
        
        //get line and verify, that we aren't done
        if (fgets(lines[i],line_size_to_allocate-1,file)==NULL) {
            break;
        }
        
        //get rid of CR/LF at EOL
        int j;
        for (j = strlen(lines[i])-1; j >= 0 && (lines[i][j] == '\n' || lines[i][j] == '\r'); j--)
            ;
        lines[i][j+1]='\0';
    }
    configLines = i+1;
    
    return lines;   
}

Dir* importData(char* path) {
    FILE* file;
    char** lines;
    Dir* root = NULL;
    
    printf("%s %s\n","Loading structure from:",path); 
    file = fopen(path,"r"); //TODO: add errorhandling here, missing folders give segfault
    lines = getLines(file);
    fclose(file);
    
    Dir* currDir = NULL;
    for (int i = 0; lines[i] != NULL; i++) {
        if (lines[i][0] == '#') {//it's a comment
            continue;
        }
        else if (isDirDecl(lines[i])) {
            Dir* dir = newDirFromDecl(lines[i]);
            dir = addDir(dir); //dir might be replaced with an already existing dir
            currDir = dir;
            if(root==NULL) {
                root = dir;
            }
        }
        else if (isDirRef(lines[i])) {
            Dir* dir = newDirFromRef(lines[i],currDir);
            addDir(dir);
        }
        else if (isActRef(lines[i])) {
            Act* act = newActFromRef(lines[i],currDir);
            addAct(act);
        }        
    }
    
    printf("%s %i\n%s %i\n","Dirs:", savedDirs, "Acts:", savedActs);

	return root;
}