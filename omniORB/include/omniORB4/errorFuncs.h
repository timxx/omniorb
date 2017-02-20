// -*- Mode: C++; -*-
//                            Package   : omniORB
// errorFuncs.h               Created on: 2015/10/15
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2015-2017 Apasphere Ltd
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
//    *** PROPRIETARY INTERFACE ***

#ifndef __omni_errorFuncs_h__
#define __omni_errorFuncs_h__

class cdrStream;

extern void _CORBA_new_operator_return_null();
extern void _CORBA_bound_check_error();
extern void _CORBA_marshal_sequence_range_check_error(cdrStream&);
extern _CORBA_Boolean _CORBA_use_nil_ptr_as_nil_objref();
extern void _CORBA_null_string_ptr(_CORBA_Boolean);
extern void _CORBA_invoked_nil_pseudo_ref();
extern void _CORBA_invoked_nil_objref();

extern _CORBA_Boolean _CORBA_use_nil_ptr_as_nil_pseudo_objref(
                                                         const char* objType);
extern void _CORBA_bad_param_freebuf();


#endif // __omni_errorFuncs_h__
