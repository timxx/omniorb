// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpConnection.cc           Created on: 19 Mar 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2005 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETARY INTERFACE ***
//

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <orbParameters.h>
#include <SocketCollection.h>
#include <tcpSocket.h>
#include <tcp/tcpConnection.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>

#if defined(NTArchitecture)
#  include <ws2tcpip.h>
#endif

#if defined(__vxWorks__)
#  include "selectLib.h"
#endif

OMNI_EXPORT_LINK_FORCE_SYMBOL(tcpConnection);

OMNI_NAMESPACE_BEGIN(omni)


/////////////////////////////////////////////////////////////////////////
int
tcpConnection::Send(void* buf, size_t sz,
		    unsigned long deadline_secs,
		    unsigned long deadline_nanosecs) {

  if (sz > orbParameters::maxSocketSend)
    sz = orbParameters::maxSocketSend;

  int tx;

  do {

    struct timeval t;

    if (deadline_secs || deadline_nanosecs) {
      SocketSetTimeOut(deadline_secs,deadline_nanosecs,t);
      if (t.tv_sec == 0 && t.tv_usec == 0) {
	// Already timeout.
	return 0;
      }
      else {
#if defined(USE_POLL)
	struct pollfd fds;
	fds.fd = pd_socket;
	fds.events = POLLOUT;
	tx = poll(&fds,1,t.tv_sec*1000+(t.tv_usec/1000));
#else
	fd_set fds, efds;
	FD_ZERO(&fds);
	FD_ZERO(&efds);
	FD_SET(pd_socket,&fds);
	FD_SET(pd_socket,&efds);
	tx = select(pd_socket+1,0,&fds,&efds,&t);
#endif
	if (tx == 0) {
	  // Time out!
	  return 0;
	}
	else if (tx == RC_SOCKET_ERROR) {
	  if (ERRNO == RC_EINTR)
	    continue;
	  else {
	    return -1;
	  }
	}
      }
    }

    // Reach here if we can write without blocking or we don't
    // care if we block here.
    if ((tx = ::send(pd_socket,(char*)buf,sz,0)) == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR)
	continue;
      else
	return -1;
    }
    else if (tx == 0)
      return -1;

    break;

  } while(1);

  return tx;

}

/////////////////////////////////////////////////////////////////////////
int
tcpConnection::Recv(void* buf, size_t sz,
		    unsigned long deadline_secs,
		    unsigned long deadline_nanosecs) {

  if (sz > orbParameters::maxSocketRecv)
    sz = orbParameters::maxSocketRecv;

  int rx;

  do {

    if (pd_shutdown)
      return -1;

    struct timeval t;

    if (deadline_secs || deadline_nanosecs) {
      SocketSetTimeOut(deadline_secs,deadline_nanosecs,t);
      if (t.tv_sec == 0 && t.tv_usec == 0) {
	// Already timeout.
	return 0;
      }
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      if (orbParameters::scanGranularity > 0 && 
	  t.tv_sec > orbParameters::scanGranularity) {
	t.tv_sec = orbParameters::scanGranularity;
      }
#endif
    }
    else {
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      t.tv_sec = orbParameters::scanGranularity;
      t.tv_usec = 0;
#else
      t.tv_sec = t.tv_usec = 0;
#endif
    }

    if (t.tv_sec || t.tv_usec) {
#if defined(USE_POLL)
      struct pollfd fds;
      fds.fd = pd_socket;
      fds.events = POLLIN;
      rx = poll(&fds,1,t.tv_sec*1000+(t.tv_usec/1000));
#else
      fd_set fds, efds;
      FD_ZERO(&fds);
      FD_ZERO(&efds);
      FD_SET(pd_socket,&fds);
      FD_SET(pd_socket,&efds);
      rx = select(pd_socket+1,&fds,0,&efds,&t);
#endif
      if (rx == 0) {
	// Time out!
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
	continue;
#else
	return 0;
#endif
      }
      else if (rx == RC_SOCKET_ERROR) {
	if (ERRNO == RC_EINTR)
	  continue;
	else {
	  return -1;
	}
      }
    }

    // Reach here if we can read without blocking or we don't
    // care if we block here.
    if ((rx = ::recv(pd_socket,(char*)buf,sz,0)) == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR)
	continue;
      else
	return -1;
    }
    else if (rx == 0)
      return -1;

    break;

  } while(1);

  return rx;
}

/////////////////////////////////////////////////////////////////////////
void
tcpConnection::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
  pd_shutdown = 1;
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpConnection::myaddress() {
  return (const char*)pd_myaddress;
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpConnection::peeraddress() {
  return (const char*)pd_peeraddress;
}

/////////////////////////////////////////////////////////////////////////
tcpConnection::tcpConnection(SocketHandle_t sock,
			     SocketCollection* belong_to) :
  SocketHolder(sock) {

  OMNI_SOCKADDR_STORAGE addr;
  SOCKNAME_SIZE_T l;

  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getsockname(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    pd_myaddress = (const char*)"giop:tcp:255.255.255.255:65535";
  }
  else {
    pd_myaddress = tcpSocket::addrToURI((sockaddr*)&addr, "giop:tcp");
  }

  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getpeername(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    pd_peeraddress = (const char*)"giop:tcp:255.255.255.255:65535";
  }
  else {
    pd_peeraddress = tcpSocket::addrToURI((sockaddr*)&addr, "giop:tcp");
  }

  SocketSetCloseOnExec(sock);

  belong_to->addSocket(this);
}

/////////////////////////////////////////////////////////////////////////
tcpConnection::~tcpConnection() {
  clearSelectable();
  pd_belong_to->removeSocket(this);
  CLOSESOCKET(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
void
tcpConnection::setSelectable(int now,
			     CORBA::Boolean data_in_buffer) {

  SocketHolder::setSelectable(now, data_in_buffer);
}

/////////////////////////////////////////////////////////////////////////
void
tcpConnection::clearSelectable() {

  SocketHolder::clearSelectable();
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpConnection::isSelectable() {
  return pd_belong_to->isSelectable(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpConnection::Peek() {
  return SocketHolder::Peek();
}


OMNI_NAMESPACE_END(omni)