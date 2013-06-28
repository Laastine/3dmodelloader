#include <stdio.h>
#include <vector>
#include "handleZLib.h"
#include <zlib.h>

unsigned int decompressFile(char *inFileName, char *outFileName)
{
	gzFile inFile = gzopen(inFileName, "rb");
	FILE *outFile = fopen(outFileName, "wb");
	if (!inFile || !outFile)
		return -1;

	vector<char> outBuffer;
	int numRead = 0;
	while ((numRead = gzread(inFile, &outBuffer, outBuffer.size())) > 0)
	{
		fwrite(&outBuffer, 1, numRead, outFile);
	}



	gzclose(inFile);
	fclose(outFile);
}

unsigned int compressFile(char *inFileName, char *outFileName)
{
    FILE *inFile = fopen(inFileName, "rb");
    gzFile outFile = gzopen(outFileName, "wb");
    if (!inFile || !outFile) return -1;

    vector<char> inBuffer;
    int numRead = 0;
    unsigned long total_read = 0, total_wrote = 0;
	while ((numRead = fread(&inBuffer, 1, inBuffer.size(), inFile)) > 0)
	{
       total_read += numRead;
	   gzwrite(outFile, &inBuffer, numRead);
    }
	fclose(inFile);
	gzclose(outFile);
}