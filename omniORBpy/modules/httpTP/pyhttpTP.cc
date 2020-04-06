// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyhttpTP.cc                Created on: 2018/05/11
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2018      BMC Software.
//    Copyright (C) 2002-2018 Apasphere Ltd.
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library. If not, see http://www.gnu.org/licenses/
//
// Description:
//    HTTP transport library

#ifdef __WIN32__
#  define DLL_EXPORT _declspec(dllexport)
#else
#  define DLL_EXPORT
#endif

#if defined(__VMS)
#  include <Python.h>
#else
#  include PYTHON_INCLUDE
#endif

#include <omniORB4/CORBA.h>
#include <omniORB4/httpContext.h>
#include <omniORB4/giopEndpoint.h>

#include "../omnipy.h"

static omniORBpyAPI* api;


static inline void
setDictEntryIfValid(PyObject* d, const char* name, const char* val)
{
  if (val) {
    PyObject* s = String_FromString(val);
    PyDict_SetItemString(d, (char*)name, s);
    Py_DECREF(s);
  }
}

static void
setCallInfo(PyObject* d, giopConnection* conn)
{
  httpContext::PeerDetails* pd = (httpContext::PeerDetails*)conn->peerdetails();

  if (pd) {
    setDictEntryIfValid(d, "http_host", pd->host_header());
    if (pd->crypto())
      setDictEntryIfValid(d, "http_identity", pd->crypto()->peerIdent());

    if (pd->ssl()) {
      PyDict_SetItemString(d, (char*)"ssl_verified",
                           pd->verified() ? Py_True : Py_False);
    }
  }
}


extern "C" {

  static char set_proxy_doc[] =
  "set_proxy(proxy_url [, username, password])\n"
  "\n"
  "Set a web proxy, with optional username and password.\n";

  static PyObject* pyhttpTP_set_proxy(PyObject* self, PyObject* args)
  {
    const char* url;
    const char* username = 0;
    const char* password = 0;

    if (!PyArg_ParseTuple(args, (char*)"z|zz", &url, &username, &password))
      return 0;

    if (httpContext::singleton) {
      try {
        httpContext::singleton->update_proxy(url, username, password);
      }
      OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    }
    else {
      // Set the globals. We leak copies of the strings, because there
      // is no safe time to free them.
      httpContext::proxy_url      = url ?      CORBA::string_dup(url)      : 0;
      httpContext::proxy_username = username ? CORBA::string_dup(username) : 0;
      httpContext::proxy_password = password ? CORBA::string_dup(password) : 0;
    }
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char set_CA_doc[] =
  "set_CA(ca_file, ca_path)\n"
  "\n"
  "Set the certificate authorities used for validation. ca_file is the path\n"
  "to a file containing one or more CA certificates; ca_path is the path to\n"
  "a directory containing CA certificate files, as accepted by OpenSSL.\n"
  "Either may be None.\n";

  static PyObject* pyhttpTP_set_CA(PyObject* self, PyObject* args)
  {
    const char* ca_file;
    const char* ca_path;

    if (!PyArg_ParseTuple(args, (char*)"zz", &ca_file, &ca_path)) return 0;

    if (httpContext::singleton) {
      try {
        httpContext::singleton->update_CA(ca_file, ca_path);
      }
      OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    }
    else {
      // Set the globals. We leak copies of the strings, because there
      // is no safe time to free them.
      httpContext::certificate_authority_file = (ca_file ?
                                                CORBA::string_dup(ca_file) : 0);
      httpContext::certificate_authority_path = (ca_path ?
                                                CORBA::string_dup(ca_path) : 0);
    }
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char set_key_doc[] =
  "set_key(key_file, key_password)\n"
  "\n"
  "Set the private key and its password.\n";

  static PyObject* pyhttpTP_set_key(PyObject* self, PyObject* args)
  {
    const char* key_file;
    const char* password;

    if (!PyArg_ParseTuple(args, (char*)"ss", &key_file, &password)) return 0;

    if (httpContext::singleton) {
      try {
        httpContext::singleton->update_key(key_file, password);
      }
      OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    }
    else {
      // Set the globals. We leak copies of the strings, because there
      // is no safe time to free them.
      httpContext::key_file          = CORBA::string_dup(key_file);
      httpContext::key_file_password = CORBA::string_dup(password);
    }
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char set_cipher_list_doc[] =
  "set_cipher_list(cipher_list)\n"
  "\n"
  "Set the OpenSSL cipher list string.\n";

  static PyObject* pyhttpTP_set_cipher_list(PyObject* self, PyObject* args)
  {
    const char* cipher_list;

    if (!PyArg_ParseTuple(args, (char*)"s", &cipher_list)) return 0;

    if (httpContext::singleton) {
      try {
        httpContext::singleton->update_cipher_list(cipher_list);
      }
      OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    }
    else {
      // Set the global.
      httpContext::cipher_list = CORBA::string_dup(cipher_list);
    }
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char reinit_doc[] =
  "reinit()\n"
  "\n"
  "Re-initialize the HTTP transport to pick up new configuration.\n";
  
  static PyObject* pyhttpTP_reinit(PyObject* self,
                                  PyObject* args)
  {
    try {
      if (httpContext::singleton) {
        httpContext::singleton->reinit(0);
      }
      else {
        OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
                      CORBA::COMPLETED_NO);
      }
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    Py_INCREF(Py_None); return Py_None;
  }

  static PyMethodDef omnihttpTP_methods[] = {
    {(char*)"set_proxy",
     pyhttpTP_set_proxy, METH_VARARGS,
     set_proxy_doc},

    {(char*)"set_CA",
     pyhttpTP_set_CA, METH_VARARGS,
     set_CA_doc},

    {(char*)"set_key",
     pyhttpTP_set_key, METH_VARARGS,
     set_key_doc},

    {(char*)"set_cipher_list",
     pyhttpTP_set_cipher_list, METH_VARARGS,
     set_cipher_list_doc},

    {(char*)"reinit",
     pyhttpTP_reinit, METH_VARARGS,
     reinit_doc},

    {0,0}
  };

#if (PY_VERSION_HEX < 0x03000000)

  void DLL_EXPORT init_omnihttpTP()
  {
    PyObject* m = Py_InitModule((char*)"_omnihttpTP", omnihttpTP_methods);

    // Get the omniORBpy C++ API, which is used in
    // OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    PyObject* omnipy = PyImport_ImportModule((char*)"_omnipy");
    PyObject* pyapi  = PyObject_GetAttrString(omnipy, (char*)"API");
    api              = (omniORBpyAPI*)PyCObject_AsVoidPtr(pyapi);
    Py_DECREF(pyapi);

    // Set callInfo handler
    omniPy::PyRefHolder callInfoFns(
                          PyObject_GetAttrString(omnipy, (char*)"callInfoFns"));
    if (!callInfoFns.valid())
      return;

    omniPy::PyRefHolder pyfn(PyCObject_FromVoidPtr((void*)setCallInfo, 0));
    PyDict_SetItemString(callInfoFns, "http", pyfn);
  }

#else

  static struct PyModuleDef omnihttpTPmodule = {
    PyModuleDef_HEAD_INIT,
    "_omnihttpTP",
    "omniORBpy HTTP transport",
    -1,
    omnihttpTP_methods,
    NULL,
    NULL,
    NULL,
    NULL
  };

  PyMODINIT_FUNC
  PyInit__omnihttpTP(void)
  {
    // Get the omniORBpy C++ API, which is used in
    // OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    api = (omniORBpyAPI*)PyCapsule_Import("_omnipy.API", 0);

    // Set callInfo handler
    PyObject* omnipy = PyImport_ImportModule((char*)"_omnipy");
    omniPy::PyRefHolder callInfoFns(
                          PyObject_GetAttrString(omnipy, (char*)"callInfoFns"));
    if (!callInfoFns.valid())
      return 0;
 
    omniPy::PyRefHolder pyfn(PyCapsule_New((void*)setCallInfo, 0, 0));
    PyDict_SetItemString(callInfoFns, "http", pyfn);

    return PyModule_Create(&omnihttpTPmodule);
  }

#endif
};
