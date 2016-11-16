/* 
   Socket.cpp

   Copyright (C) 2002-2004 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/
#include <iostream>

#include "stdafx.h"

#include "Socket.h"
#include <errno.h>

using namespace std;

#pragma warning(disable:4996)

int Socket::nofSockets_= 0;

void Socket::Start() {
  if (!nofSockets_) {
    WSADATA info;
    if (WSAStartup(MAKEWORD(2,0), &info)) {
      throw "Could not start WSA";
    }
  }
  ++nofSockets_;
}

void Socket::End() {
  WSACleanup();
}

Socket::Socket() : s_(0) {
  Start();
  // UDP: use SOCK_DGRAM instead of SOCK_STREAM
  s_ = socket(AF_INET,SOCK_STREAM,0);

  if (s_ == INVALID_SOCKET) {
    throw "INVALID_SOCKET";
  }

  refCounter_ = new int(1);
}

Socket::Socket(SOCKET s) : s_(s) {
  Start();
  refCounter_ = new int(1);
};

Socket::~Socket() {
  if (! --(*refCounter_)) {
    Close();
    delete refCounter_;
  }


  --nofSockets_;
  if (!nofSockets_) End();
}

Socket::Socket(const Socket& o) {
  refCounter_=o.refCounter_;
  (*refCounter_)++;
  s_         =o.s_;


  nofSockets_++;
}

Socket& Socket::operator =(Socket& o) {
  (*o.refCounter_)++;


  refCounter_=o.refCounter_;
  s_         =o.s_;


  nofSockets_++;


  return *this;
}

void Socket::Close() {
  closesocket(s_);
}

int Socket::recvtimeout(SOCKET s, char *buf, int len, int timeout)
{
    fd_set fds;
    int n;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(s, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    // wait until timeout or data received
    n = select((int) s+1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error

    // data must be here, so do a normal recv()
    return recv(s, buf, len, 0);
}

string_type Socket::ReceiveBytes() {
  string_type ret;
  char buf[1024];
 
 for ( ; ; ) {
  u_long arg = 0;
  if (ioctlsocket(s_, FIONREAD, &arg) != 0)
   break;
  if (arg == 0)
   break;
  if (arg > 1024)
   arg = 1024;
  int rv = recv (s_, buf, arg, 0);
  if (rv <= 0)
   break;
  string_type t;
  t.assign (buf, rv);
  ret += t;
 }
 
  return ret;
}

int Socket::ReceiveLineToMemory(INT_DATA_BLOB* pData, int timeout, int bytesPerSample) {
	#define READ_BUFFER_LENGTH		0x10000		// 65KB
//	#define READ_BUFFER_LENGTH		10		
	#define BASE_OFFSET				0x1000		// 4086B - MAXIMUM VALUE OF 12bit SAMPLE - SHIFT TO PREVENT NEGATIVE VALUES 
	int         status = 0;
	int         retStat = 0;  
	int			endMark = 0;
	BYTE		data[READ_BUFFER_LENGTH];	
    int			endState = 0;
	int			finalData = 0;
	int			offset = 0;
	int			totalReaded = 0;

	// NOTE: TWO BYTES PER SAMPLE IS ASSUMED!
	_ASSERT(bytesPerSample < 3);
	
	pData->dwActLen = 0;
	
    // GET FIRST BLOCK
    memset(data, 0, sizeof(data));
    retStat = recvtimeout(s_, (char*) data, READ_BUFFER_LENGTH, timeout);
    while (retStat > 0) {
		totalReaded += retStat; 
		// FORMAT DATA INTO INT
		int i = 0;
		for (i = 0; i < retStat; i = i + bytesPerSample) { 
			finalData = 0xff - (data[i] & 0xff);     // low 8bits of measurement

            if (bytesPerSample == 2) {
				int high = 0;                                   // high 4 bits of measurement
				if ((data[i + 1] & 0xf) < 8) {         // 0 is the middle value, 1 to 7 is signalizing decreased (-1 to -7 * 256) part of the sample
					high = - (data[i + 1] & 0xf);      // 8 to 15 are increasing part - e.g., 0xf is actualy +1 to higher 4 bits
				}
				else {
					high = 0xf - (data[i + 1] & 0xf) + 1;
				}
				finalData += (high << 8) + BASE_OFFSET;		// MOVE EXISTING CONTENT TO HIGHER ADDRESS + ADD BASE VALUE 4096 TO PREVENT VALUES LOWER THAN 0  
			}

			pData->pData[offset] = finalData;			
			offset++;
        }
        
        if (i != retStat) {
			// SOME UNPROCESSED BYTES LEFT, PUT IT AT THE BEGINING OF NEXT BLOCK OF DATA
			BYTE	leftByte = data[i - 1];
			memset(data, 0, sizeof(data));
			// SET REMAINING BYTE
			data[0] = leftByte;
			// RECEIVE NEXT BLOCK
			retStat = recvtimeout(s_, (char*) data + 1, READ_BUFFER_LENGTH - 1, timeout);
			// ADD REMAINING BYTE 
			retStat++;
        } 
        else {
			// GET NEXT BLOCK
			memset(data, 0, sizeof(data));
			retStat = recvtimeout(s_, (char*) data, READ_BUFFER_LENGTH, timeout);
		}
    }
    
	pData->dwActLen = offset - 4;	// DO NOT COUNT LAST 8 BYTES AS A MEASUREMNET - SHOULD BE '88 77 66 55 \n @ @ \n'
    
    if (retStat < 0) status = -1;
    
    return status;
}


string_type Socket::ReceiveLine(int timeout) {
  string_type ret;
  int         status = 0;
   while (1) {
     char r;
    
     if (timeout == 0) status = recv(s_, &r, 1, 0);
     else status = recvtimeout(s_, &r, 1, timeout);
     
     switch(status) {
       case 0: // not connected anymore;
         return "";
       case -1:
          if (errno == EAGAIN) {
             return ret;
          } else {
            // not connected anymore
           return "";
         }
       case -2: // timeout
         return ret;   
     }

     ret += r;
     if (r == '\n')  return ret;
   }
}

string_type Socket::ReceiveResponse(string_type endSeq, int timeout) {
  string_type ret;
  string_type line;
  
  // FIRST LINE READ EVERY TIME (SHOULD BE THERE)  
  line = ReceiveLine();
  // end when zero line found or endSeq character
  while (line != "" && line.find(endSeq) == string_type::npos) {
    ret += line;
    line = ReceiveLine(timeout);
  }
  ret += line;
  
  return ret;  
}

int Socket::ReceiveLineToFile(string_type filePath, int timeout, int* pWrittenValues) {
  string_type ret;
  int         status = 0;
  int         retStat = 0;  
  FILE        *file; 
//  char        buffer[10000]; 
  int         bufferLen = 10000;   
  int         bufferOffset = 0;  
  int           bufferLength = 10000;
  BOOL        bStop = FALSE;

    if (pWrittenValues != NULL) *pWrittenValues = 0;
  
    if ((file = fopen((LPCTSTR) filePath.c_str(), "a")) != NULL) { 
        unsigned char    data = 0;
        char    dataStr[10];
        int     endState = 0;
        while (1) {
/*
            char    r;
            char    tmp;
            if (timeout == 0) retStat = recv(s_, &r, 1, 0);
            else retStat = recvtimeout(s_, &r, 1, timeout);


            if (retStat == 0 || retStat == -1 || retStat == -2) break; 

            // replace space by newline
            tmp = (r != ' ') ? r : '\n'; 
            
            fwrite(&tmp, 1, 1, file); 
            if (r == '\n')  break;

/**/
/*            
            bufferLength = 4;
            retStat = recvtimeout(s_, buffer, bufferLength, timeout);
            if (retStat == 0 || retStat == -1 || retStat == -2) break; 
            // replace space by newline
            for (int i = 0; i < bufferLength; i++) {
                if (buffer[i] == '\n') {
                    bStop = TRUE;
                    bufferLength = i+1; 
                    break;
                }
                else buffer[i] = (buffer[i] != ' ') ? buffer[i] : '\n'; 
            }
            
            fwrite(buffer, 1, bufferLength, file); 
            
            if (bStop) break;
/**/            


            retStat = recvtimeout(s_, (char*) &data, sizeof(data), timeout);
            if (retStat == 0 || retStat == -1 || retStat == -2) break; 
            
            // CHECK FOR END STATE (MAGIC 0x88776655 EXPECTED)
/*
            _ASSERT(data != 0x88);
            _ASSERT(data != 0x77);
            _ASSERT(data != 0x66);
            _ASSERT(data != 0x55);
/**/            
            switch (endState) {
                case 0: if (data == 0x88) endState = 1; else endState = 0; break;
                case 1: if (data == 0x77) endState = 2; else endState = 0; break;
                case 2: if (data == 0x66) endState = 3; else endState = 0; break;
                case 3: if (data == 0x55) endState = 4; else endState = 0; break;
                default: endState = 0;
            }
            if (endState == 4) {
                // END OF DATA DETECTED, READ ADDITIONAL ONE BYTE - SHOULD BE \n AND FINISH
               retStat = recvtimeout(s_, dataStr, 1, timeout);
                _ASSERT(dataStr[0] == '\n');
                break;
            }
            else {
                //sprintf_s(dataStr, 10, "%03d\n", 0xff-(data&0xff));
                sprintf_s(dataStr, 10, "%03d\n", 0xff - data);
                fwrite(dataStr, 1, strlen(dataStr), file); 
                if (pWrittenValues != NULL) *pWrittenValues = *pWrittenValues + 1;
/*            
                sprintf_s(dataStr, 10, "%03d\n", 0xff-(data&0xff));
                fwrite(dataStr, 1, strlen(dataStr), file); 
                sprintf_s(dataStr, 10, "%03d\n", 0xff-((data>>8)&0xff));
                fwrite(dataStr, 1, strlen(dataStr), file); 
                sprintf_s(dataStr, 10, "%03d\n", 0xff-((data>>16)&0xff));
                fwrite(dataStr, 1, strlen(dataStr), file); 
                sprintf_s(dataStr, 10, "%03d\n", 0xff-((data>>24)&0xff));
                fwrite(dataStr, 1, strlen(dataStr), file); 
/**/                
            }

        }

        // flush remaining data to file
        //fwrite(buffer, 1, bufferOffset, file); 
        
        // trailing #
        //dataStr[0] = '#';
        //fwrite(dataStr, 1, 1, file); 
        
        fclose(file);
    }
    else status = 16;
    
    if (retStat < 0) status = -1;
    
    return status;
}

void Socket::SendLine(string_type s) {
  s += '\n';
  send(s_,s.c_str(),(int) s.length(),0);
}

void Socket::SendBytes(const string_type& s) {
  send(s_,s.c_str(),(int) s.length(),0);
}

SocketServer::SocketServer(int port, int connections, TypeSocket type) {
  sockaddr_in sa;

  memset(&sa, 0, sizeof(sa));

  sa.sin_family = PF_INET;             
  sa.sin_port = htons(port);          
  s_ = socket(AF_INET, SOCK_STREAM, 0);
  if (s_ == INVALID_SOCKET) {
    throw "INVALID_SOCKET";
  }

  if(type==NonBlockingSocket) {
    u_long arg = 1;
    ioctlsocket(s_, FIONBIO, &arg);
  }

  /* bind the socket to the internet address */
  if (bind(s_, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) {
    closesocket(s_);
    throw "INVALID_SOCKET";
  }
  
  listen(s_, connections);                               
}

Socket* SocketServer::Accept() {
  SOCKET new_sock = accept(s_, 0, 0);
  if (new_sock == INVALID_SOCKET) {
          int rc = WSAGetLastError();
          if(rc==WSAEWOULDBLOCK) {
                  return 0; // non-blocking call, no request pending
          }
          else {
            throw "Invalid Socket";
      }
  }



  Socket* r = new Socket(new_sock);
  return r;
}

SocketClient::SocketClient(const string_type& host, int port) : Socket() {
  string_type error;


  hostent *he;
  if ((he = gethostbyname(host.c_str())) == 0) {
    error = strerror(errno);
    throw error;
  }


  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = *((in_addr *)he->h_addr);
  memset(&(addr.sin_zero), 0, 8); 


  if (::connect(s_, (sockaddr *) &addr, sizeof(sockaddr))) {
    error = strerror(WSAGetLastError());
    throw error;
  }
}

SocketSelect::SocketSelect(Socket const * const s1, Socket const * const s2, TypeSocket type) {
  FD_ZERO(&fds_);
  FD_SET(const_cast<Socket*>(s1)->s_,&fds_);
  if(s2) {
    FD_SET(const_cast<Socket*>(s2)->s_,&fds_);
  }     


  TIMEVAL tval;
  tval.tv_sec  = 0;
  tval.tv_usec = 1;


  TIMEVAL *ptval;
  if(type==NonBlockingSocket) {
    ptval = &tval;
  }
  else { 
    ptval = 0;
  }


  if (select (0, &fds_, (fd_set*) 0, (fd_set*) 0, ptval) 
      == SOCKET_ERROR) throw "Error in select";
}

bool SocketSelect::Readable(Socket const * const s) {
  if (FD_ISSET(s->s_,&fds_)) return true;
  return false;
}


