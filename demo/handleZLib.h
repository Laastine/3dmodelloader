#ifndef HANDLEZLIB_H
#define HANDLEZLIB_H

#include <iostream>

using namespace std;

unsigned long fileSize(char *filename);

unsigned int decompressFile(char *infilename, char *outfilename);

unsigned int compressFile(char *infilename, char *outfilename);

#endif