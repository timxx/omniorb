// -*- Mode: C++; -*-
//                            Package   : omniORB
// callDescriptor.h           Created on: 12/98
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003-2010 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//    Object that encapsulates a call to be performed
//

#ifndef __OMNIORB_CALLDESCRIPTOR_H__
#define __OMNIORB_CALLDESCRIPTOR_H__

class omniObjRef;
class omniServant;
class omniCurrent;

OMNI_NAMESPACE_BEGIN(omni)
class omniOrbPOA;
class giopAddress;
OMNI_NAMESPACE_END(omni)

//////////////////////////////////////////////////////////////////////
///////////////////////// omniCallDescriptor /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniCallDescriptor {
public:
  typedef void (*LocalCallFn)(omniCallDescriptor*, omniServant*);

  struct InterceptorFn {
    LocalCallFn    fn;
    InterceptorFn* next;
  };

  inline omniCallDescriptor(LocalCallFn lcfn, const char* op_,
			    size_t op_len_, _CORBA_Boolean oneway,
			    const char*const* user_excns_,
			    int n_user_excns_,
                            _CORBA_Boolean is_upcall_)
    : pd_do_call(sd_interceptor_call ? sd_interceptor_call : lcfn),
      pd_local_call(lcfn),
      pd_op(op_), pd_oplen(op_len_),
      pd_user_excns(user_excns_),
      pd_n_user_excns(n_user_excns_),
      pd_is_oneway(oneway),
      pd_is_upcall(is_upcall_),
      pd_contains_values(0),
      pd_first_address_used(0),
      pd_current_address(0),
      pd_objref(0),
      pd_poa(0),
      pd_localId(0),
      pd_deadline_secs(0),
      pd_deadline_nanosecs(0) {}

  virtual ~omniCallDescriptor() {}

  //////////////////////////////////////////////////
  // Methods to implement call on the client side //
  //////////////////////////////////////////////////

  virtual void initialiseCall(cdrStream&);

  virtual void marshalArguments(cdrStream&);
  // Defaults to no arguments.

  virtual void unmarshalReturnedValues(cdrStream&);
  // Defaults to no arguments and returns void.

  virtual void userException(cdrStream& stream, _OMNI_NS(IOP_C)* iop_client,
			     const char*);
  // Called on the client side to handle a user exception.
  // Defaults to no user exceptions, and thus throws CORBA::UNKNOWN.
  // Most versions of this throw either a user exception or
  // CORBA::UNKNOWN, but it is permitted to return successfully. This
  // is done by DII, for example.
  // If iop_client is non-zero, must call iop_client->RequestCompleted().

  void validateUserException(const CORBA::UserException& ex);
  // Called to validate a user exception in a local call. If the call
  // descriptor knows the exception is not valid for the call, throws
  // CORBA::UNKNOWN. If the exception is valid, or the call descriptor
  // does not know, returns.

  //////////////////////////////////////////////////
  // Methods to implement call on the server side //
  //////////////////////////////////////////////////

  virtual void unmarshalArguments(cdrStream&);
  // Defaults to no arguments.

  virtual void marshalReturnedValues(cdrStream&);
  // Defaults to no arguments and returns void.

  ////////////////
  // Invocation //
  ////////////////

  // Called by stub code to perform a local call.
  inline void doLocalCall(omniServant* servant) {
    pd_do_call(this, servant);
  }

  // Interceptor functions *must* call this to continue the call.
  inline void interceptedCall(omniServant* servant) {
    LocalCallFn lcfn;
    if (pd_interceptor_stack) {
      lcfn = pd_interceptor_stack->fn;
      pd_interceptor_stack = pd_interceptor_stack->next;
    }
    else
      lcfn = pd_local_call;

    lcfn(this, servant);
  }

  ///////////////
  // Accessors //
  ///////////////

  inline const char* op() const            { return pd_op;           }
  inline size_t op_len() const             { return pd_oplen;        }
  inline _CORBA_Boolean is_oneway() const  { return pd_is_oneway;    }
  inline void set_oneway(_CORBA_Boolean o) { pd_is_oneway = o;       }
  inline const char* const* user_excns()   { return pd_user_excns;   }
  inline int n_user_excns()                { return pd_n_user_excns; }
  inline _CORBA_Boolean is_upcall() const  { return pd_is_upcall;    }

  inline _CORBA_Boolean haslocalCallFn() const {
    return (pd_local_call) ? 1 : 0;
  }

  inline const _OMNI_NS(giopAddress)* firstAddressUsed() { 
    return pd_first_address_used;
  }

  inline void firstAddressUsed(const _OMNI_NS(giopAddress)* a) { 
    pd_first_address_used = a;
  }

  inline const _OMNI_NS(giopAddress)* currentAddress() { 
    return pd_current_address;
  }

  inline void currentAddress(const _OMNI_NS(giopAddress)* a) { 
    pd_current_address = a;
  }

  inline void getDeadline(unsigned long& secs, unsigned long& nanosecs) const {
    secs = pd_deadline_secs;
    nanosecs = pd_deadline_nanosecs;
  }

  inline void setDeadline(unsigned long secs, unsigned long nanosecs) {
    pd_deadline_secs = secs;
    pd_deadline_nanosecs = nanosecs;
  }

  inline void containsValues(_CORBA_Boolean v) {
    pd_contains_values = v;
  }
  inline _CORBA_Boolean containsValues() {
    return pd_contains_values;
  }
  // If pd_contains_values is true, the arguments / return values
  // contain valuetypes. The spec requires that values are always
  // passed by value, so local calls must make copies. Since values
  // can be shared by separate arguments, the whole argument list must
  // be copied in a single operation. We do this by marshalling via a
  // temprary memory buffer. This is slow, so it can be turned off by
  // setting the copyValuesInLocalCalls parameter to false.


  /////////////////////
  // Current support //
  /////////////////////

  inline void objref(omniObjRef* o)           { pd_objref = o; }
  inline omniObjRef* objref()                 { return pd_objref; }
  inline void poa(_OMNI_NS(omniOrbPOA*) poa_) { pd_poa = poa_; }
  inline _OMNI_NS(omniOrbPOA*) poa()          { return pd_poa; }
  inline void localId(omniLocalIdentity* lid) { pd_localId = lid; }
  inline omniLocalIdentity* localId()         { return pd_localId; }


  //////////////////////////////
  // Interceptor registration //
  //////////////////////////////

  static void addInterceptor(LocalCallFn func);
  static void removeInterceptor(LocalCallFn func);


private:
  LocalCallFn                  pd_do_call;
  LocalCallFn                  pd_local_call;
  const char*                  pd_op;
  size_t                       pd_oplen;
  const char*const*            pd_user_excns;
  int                          pd_n_user_excns;
  _CORBA_Boolean               pd_is_oneway;
  _CORBA_Boolean               pd_is_upcall;
  _CORBA_Boolean               pd_contains_values;

  const _OMNI_NS(giopAddress)* pd_first_address_used;
  const _OMNI_NS(giopAddress)* pd_current_address;
  // state holder for the giop transport in relation to this call. Not
  // manipulated by this class other than the access functions.
  // Initialised to 0 in ctor.

  /////////////////////
  // Current support //
  /////////////////////

  omniCurrent*                 pd_current;
  omniCallDescriptor*          pd_current_next;
  // The omniCurrent object maintains a stack of call descriptors,
  // representing nested colocated calls. These pointers are
  // maintained by functions in omniCurrent.

  omniObjRef*                  pd_objref;
  // This is set on the client side to indicate the object reference
  // invoked on. It is used for two things: 1. to check if a servant
  // found by the in process identity can be used in a direct local
  // call; 2. as the return value from PortableServer::Current::
  // get_reference(). In the case of a remote upcall, this is zero,
  // and an objref is constructed from information in the local id on
  // request.

  _OMNI_NS(omniOrbPOA*)        pd_poa;
  omniLocalIdentity*           pd_localId;
  // Both always set on the way through the POA during an upcall.

  ////////////////////////////
  // Deadline for this call //
  ////////////////////////////

  // This is a state holder for the call. Not manipulated by this class
  // other than the access functions. Initialised to 0 in ctor.
  unsigned long                pd_deadline_secs;
  unsigned long                pd_deadline_nanosecs;


  ///////////////////////
  // Call interceptors //
  ///////////////////////

  static LocalCallFn           sd_interceptor_call;
  // Function used in place of the normal local call function if
  // interceptors are registered. Initialises the interceptor stack
  // and calls the first interceptor in the stack.

  static void setupInterception(omniCallDescriptor* cd, omniServant* servant);
  // Function assigned to sd_interceptor_call when interceptors are registered.

  static InterceptorFn*        sd_interceptor_stack;
  InterceptorFn*               pd_interceptor_stack;
  // Global interceptor stack and this call descriptor's pointer into it.


  /////////////////////
  // Not implemented //
  /////////////////////

  omniCallDescriptor(const omniCallDescriptor&);
  omniCallDescriptor& operator = (const omniCallDescriptor&);
  // Not implemented.
};


//////////////////////////////////////////////////////////////////////
/////////////////////////// omniStdCallDesc //////////////////////////
//////////////////////////////////////////////////////////////////////

//  This just provides a namespace for pre-defined call descriptors.

class omniStdCallDesc {
public:

  // Mangled signature: void
  typedef omniCallDescriptor void_call;

  // Mangled signature: _cCORBA_mObject_i_cstring
  class _cCORBA_mObject_i_cstring : public omniCallDescriptor {
  public:
    inline _cCORBA_mObject_i_cstring(LocalCallFn lcfn, const char* op_,
		     size_t oplen, const char* a_0,
		     _CORBA_Boolean upcall=0) :
      omniCallDescriptor(lcfn, op_, oplen, 0, 0, 0, upcall),
      arg_0( OMNI_CONST_CAST(char*, a_0) ) {}
 
    inline ~_cCORBA_mObject_i_cstring() {
      if (is_upcall()) { 
	_CORBA_String_helper::free(arg_0);
      }
    }

    void marshalArguments(cdrStream&);
    void unmarshalReturnedValues(cdrStream&);
    void unmarshalArguments(cdrStream&);
    void marshalReturnedValues(cdrStream&);
 
    inline CORBA::Object_ptr result() { return pd_result._retn(); }
 
    char* arg_0;
    CORBA::Object_var pd_result;
  };

};


//////////////////////////////////////////////////////////////////////
///////////////////// omniLocalOnlyCallDescriptor ////////////////////
//////////////////////////////////////////////////////////////////////

// This class is needed to support calls to objects which may only
// reside in the local address space.  eg. ServantLocator,
// ServantActivator, AdapterActivator.

class omniLocalOnlyCallDescriptor : public omniCallDescriptor {
public:
  omniLocalOnlyCallDescriptor(LocalCallFn lcfn, const char* op_,
			      size_t op_len_, _CORBA_Boolean is_oneway_ = 0)
    : omniCallDescriptor(lcfn, op_, op_len_, is_oneway_, 0, 0, 0) {}

  // Only useful as client side descriptor. No set up for server side upcall.

  // We only need to override this one -- as it will throw an
  // exception, so the other members won't get called.
  void marshalArguments(cdrStream&);
};

#endif  // __OMNIORB_CALLDESCRIPTOR_H__
