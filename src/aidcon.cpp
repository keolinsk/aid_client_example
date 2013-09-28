/*
    This file is a part of the AID (Another Image Debugger) project.

    Copyright (C) 2013  Olinski Krzysztof E.

    This program is free software: you can redistribute it 
    and/or modify it under the terms of the GNU General Public License 
    as published by the Free Software Foundation, either version 3 of the License, 
    or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
    or FITNESS FOR A PARTICULAR PURPOSE. 
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.  
    If not, see <http://www.gnu.org/licenses/>.
*/

#include "./inc/aidcon.h"s
#include "string.h"

#include <stdio.h>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define closeSocket(socketHandle) closesocket(socketHandle)
#define sleepMe(interval) Sleep(interval*1000)
#else
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define closeSocket(socketHandle) close(socketHandle)
#define sleepMe(interval) sleep(interval)
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#endif

#ifndef TRUE
#define FALSE (0)
#define TRUE !FALSE
#endif

namespace aidcon
{

using namespace std;

/* Internal constants */
const char               ACK_CHAR[]               = "$";         // 'transmission allowed' char
const char               MAGIC_CHARS[]            = "AID0";      // magic chars
const int                MAX_RETRY_COUNT          = 2;           // retry attempts in case of connection fail
const int                RETRY_SECS               = 2;           // retry interval

const char               defHostName[]            = "127.0.0.1"; // default host name
const unsigned short     defPortNumber            = 5999;        // default port
const unsigned int       defTimeout               = 60000;       // default timeout
const          int       MAX_CHUNK_SIZE           = 0x10000000;    // max chunk size of data to be sent

#ifdef _WIN32
WSADATA __oWSAData = {0};
int     __iWSAError = -1;
#endif

struct dHeader
{
    unsigned int width;                 // image width
    unsigned int height;                // image height
    unsigned int formatStrLength;       // length of the pixel format string
    unsigned int sizeInBytes;           // size of image source data
    unsigned int nameLength;            // length of the image name string
    unsigned int notesLength;           // length of the image notes string
    unsigned int rowStrideInBits;       // source data row stride in bits 
    float        normGain[4];           // gain factors for the RGBA channels
    float        normBias[4];           // bias factors for the RGBA channels
    unsigned int auxFiltering;          // aux filtering flags
};

void printError()
{
    cerr << "[aidcon] Failed with error "
#ifdef WIN32
         << WSAGetLastError() << endl;
    WSACleanup();
#else
         << errno << endl;
#endif
}

void dummyHandle(int)
{
   return;
}

#define STRLENGTH(stringptr, limit)\
{\
    strlength = (int)(strchr(stringptr, '\0') - stringptr);\
    if(strlength <0) strlength = 0;\
    else if((unsigned int)strlength > limit) strlength = limit - 1;\
}

#define SEND(socketHandle, buffer, size)\
    if(send(socketHandle, (char*)buffer, size,0) < (int)size)\
    {\
        printError();\
        goto __exit_failed;\
    }

//##################################################################################################
CTransim::CTransim()
{

    portNumber = defPortNumber;
    strcpy(hostName, defHostName);
    timeout = defTimeout;

    chGain[0] = chGain[1] = chGain[2] = chGain[3] = 1.0f;
    chBias[0] = chBias[1] = chBias[2] = chBias[3] = 0.0f;
	auxFlags = 0;

#ifdef WIN32
    if(__iWSAError != 0)
    {
        memset(&__oWSAData, 0, sizeof(__oWSAData));
        if ((__iWSAError = WSAStartup(MAKEWORD(2, 2), &__oWSAData)) != 0)
        {
            cerr << "[aidcon] Creating the WSAStartup failed with the error: " << __iWSAError << endl;
        }
    }
#endif
}

//##################################################################################################
CTransim::~CTransim()
{
#ifdef WIN32
   WSACleanup();
#endif
}

//##################################################################################################
void CTransim::setPar(unsigned int portNumber, const char* IPStr, int timeoutInMS)
{
    this->portNumber = portNumber;

    size_t ssize = strlen(defHostName);
    if(ssize>MAX_HOST_NAME)
    {
        cerr << "[aidcon] Host name is too long..." << endl;
        return;
    }
    strncpy(this->hostName, IPStr, ssize);
    this->timeout = timeoutInMS;
}

//##################################################################################################
void CTransim::setGain(int channelNumber, float fvalue)
{
    if((channelNumber >=0)&&(channelNumber <4))
        chGain[channelNumber] = fvalue;
}

//##################################################################################################
void CTransim::setBias(int channelNumber, float fvalue)
{
    if((channelNumber >=0)&&(channelNumber <4))
        chBias[channelNumber] = fvalue;
}

//##################################################################################################
void CTransim::setFlags(unsigned int flags)
{
  auxFlags = flags;
}

//##################################################################################################
int CTransim::sendImg(unsigned int width, unsigned int height, const char*  format, int sizeInBytes, void* dataBytes, const char* name, const char* notes, unsigned int rowStrideInBits)
{

#ifdef WIN32
    SOCKET socketHandle;
#else
    int socketHandle;
    signal(SIGPIPE, dummyHandle);
#endif
    struct sockaddr_in remoteSocketInfo;
    struct linger      so_linger;
    struct hostent    *hEntPtr;
    int                auxCounter = 0;
    int                strlength;
    struct dHeader     myHeader;
    struct timeval     tv;
    fd_set             fds;

    tv.tv_sec = 0;   tv.tv_usec = 250000;
    char rebuf[2] = "";

    memset(&remoteSocketInfo, '\0', sizeof(sockaddr_in));

    if((hEntPtr = gethostbyname(hostName)) == NULL)
       {
          cerr << "[aidcon] DNS Error." << endl;
          return RES_ERROR;
       }

    if((socketHandle = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      {
         closeSocket(socketHandle);
         printError();
         return RES_ERROR;
      }

    memcpy((char *)&remoteSocketInfo.sin_addr, hEntPtr->h_addr, hEntPtr->h_length);
    remoteSocketInfo.sin_family = AF_INET;
    remoteSocketInfo.sin_port = htons((u_short)portNumber);

    so_linger.l_onoff = TRUE;
    so_linger.l_linger = 30;
    if(setsockopt(socketHandle, SOL_SOCKET, SO_LINGER, (const char*)&so_linger, sizeof(so_linger)))
    {
        cerr << "[aidcon] Socket configuration failed." << endl;
        return RES_ERROR;
    }

    while(connect(socketHandle, (struct sockaddr *)&remoteSocketInfo, sizeof(sockaddr_in)) < 0 )
    {
        auxCounter++;
        if( auxCounter > MAX_RETRY_COUNT)
        {
            cerr << "[aidcon] Connection failed." << endl;
            closeSocket(socketHandle);
            return RES_ERROR;
        }
        sleepMe(RETRY_SECS);
    }

    //Wait for 'transmission allowed' char.
    FD_ZERO(&fds) ;
    FD_SET(socketHandle, &fds);

    int retry_for_tranmission_allowed_char = 10000;

    while(rebuf[0] != ACK_CHAR[0])
    {
        if(select(socketHandle+1, &fds, NULL, NULL, &tv ))
            if(recv(socketHandle, rebuf, 1, 0)<=0)
            {
                cerr << "[aidcon] Data receive error." << endl;
                goto __exit_failed;
            }   
        if(--retry_for_tranmission_allowed_char==0)
        {
            cerr << "[aidcon] Prompt char timeout." << endl;
            goto __exit_failed;
        }
    }

    myHeader.width = width;
    myHeader.height = height;
    STRLENGTH(format, FORMAT_STRING_LENGTH);
    myHeader.formatStrLength = strlength;
    myHeader.sizeInBytes = sizeInBytes;
    STRLENGTH(name, IMG_NAME_MAX_LENGTH);
    myHeader.nameLength = strlength;
    STRLENGTH(notes, IMG_NOTES_MAX_LENGTH);
    myHeader.notesLength = strlength;
    myHeader.rowStrideInBits = rowStrideInBits;
    
    myHeader.normGain[0] = chGain[0];
    myHeader.normGain[1] = chGain[1];
    myHeader.normGain[2] = chGain[2];
    myHeader.normGain[3] = chGain[3];

    myHeader.normBias[0] = chBias[0];
    myHeader.normBias[1] = chBias[1];
    myHeader.normBias[2] = chBias[2];
    myHeader.normBias[3] = chBias[3];

	myHeader.auxFiltering = auxFlags;

    SEND(socketHandle, &MAGIC_CHARS, strlen(MAGIC_CHARS));
    SEND(socketHandle, &myHeader, sizeof(dHeader));
    if(myHeader.formatStrLength > 0)
        SEND(socketHandle, format, myHeader.formatStrLength);
    if(myHeader.nameLength > 0)
        SEND(socketHandle, name, myHeader.nameLength);
    if(myHeader.notesLength > 0)
        SEND(socketHandle, notes, myHeader.notesLength);

    while(sizeInBytes > 0)
    {
        auxCounter = (sizeInBytes>MAX_CHUNK_SIZE)?MAX_CHUNK_SIZE:sizeInBytes;
        SEND(socketHandle, dataBytes, auxCounter);
        *(unsigned int*)&dataBytes += auxCounter;
        sizeInBytes -= auxCounter;
    }

    closeSocket(socketHandle);
    return RES_OK;

__exit_failed:
    closeSocket(socketHandle);
    return RES_ERROR;
}

}//end of namespace aidcon
