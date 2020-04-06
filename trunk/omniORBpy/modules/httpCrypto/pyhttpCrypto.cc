// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyhttpTP.cc                Created on: 2018/07/13
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
//    HTTP crypto library

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
#include <omniORB4/httpCrypto.h>

#include "../omnipy.h"

static omniORBpyAPI* api;

extern "C" {

  static char init_doc[] =
  "init(ident, private_key, is_filename, key_lifetime=3600)\n"
  "\n"
  "Initialise the HTTP crypto library.\n"
  "  ident        -- unique string that identifies this process.\n"
  "  private_key  -- PEM file or string for this process' RSA private key.\n"
  "  is_filename  -- if True, private_key is a filename; if False, it is a\n"
  "                  string containing the PEM contents.\n"
  "  key_lifetime -- the number of seconds for which an AES session key is\n"
  "                  retained.\n";

  static PyObject* pyhttpCrypto_init(PyObject* self, PyObject* args)
  {
    const char* ident;
    const char* private_key;
    int         is_filename;
    int         key_lifetime = 3600;

    if (!PyArg_ParseTuple(args, (char*)"ssi|i", &ident, &private_key,
                          &is_filename, &key_lifetime))
      return 0;

    try {
      httpCryptoManager_AES_RSA* manager = new httpCryptoManager_AES_RSA();
      manager->init(ident, private_key, is_filename, key_lifetime);

      httpContext::crypto_manager = manager;
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char addClient_doc[] =
  "addClient(ident, public_key, is_filename)\n"
  "\n"
  "Add knowledge of a client, or replace the existing client's key if the\n"
  "ident is already known.\n"
  "\n"
  "  ident       -- unique string that identifies the client.\n"
  "  public_key  -- PEM file / string for the client's public key.\n"
  "  is_filename -- if True, public_key is a filename; if False, it is a\n"
  "                 string containing the PEM contents.\n";

  static PyObject* pyhttpCrypto_addClient(PyObject* self, PyObject* args)
  {
    const char* ident;
    const char* public_key;
    int         is_filename;

    if (!PyArg_ParseTuple(args, (char*)"ssi", &ident, &public_key,
                          &is_filename))
      return 0;

    try {
      httpCryptoManager_AES_RSA* manager =
        (httpCryptoManager_AES_RSA*)httpContext::crypto_manager;

      if (!manager)
        OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
                      CORBA::COMPLETED_NO);
      
      manager->addClient(ident, public_key, is_filename);
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char removeClient_doc[] =
  "removeClient(ident)\n"
  "\n"
  "Remove the client with the specified ident. Returns True if there was a\n"
  "client to remove; False if it was not known.\n";

  static PyObject* pyhttpCrypto_removeClient(PyObject* self, PyObject* args)
  {
    const char* ident;

    if (!PyArg_ParseTuple(args, (char*)"s", &ident))
      return 0;

    CORBA::Boolean ret;
    
    try {
      httpCryptoManager_AES_RSA* manager =
        (httpCryptoManager_AES_RSA*)httpContext::crypto_manager;

      if (!manager)
        OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
                      CORBA::COMPLETED_NO);
      
      ret = manager->removeClient(ident);
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    
    PyObject* r_o = ret ? Py_True : Py_False;
    Py_INCREF(r_o);
    return r_o;
  }

  static char addServer_doc[] =
  "addServer(url, public_key, is_filename)\n"
  "\n"
  "Add knowledge of a server, or replace the existing server's key if the\n"
  "URL is already known.\n"
  "\n"
  "  url         -- URL for the server.\n"
  "  public_key  -- PEM file / string for the server's public key.\n"
  "  is_filename -- if True, public_key is a filename; if False, it is a\n"
  "                 string containing the PEM contents.\n";

  static PyObject* pyhttpCrypto_addServer(PyObject* self, PyObject* args)
  {
    const char* url;
    const char* public_key;
    int         is_filename;

    if (!PyArg_ParseTuple(args, (char*)"ssi", &url, &public_key,
                          &is_filename))
      return 0;

    try {
      httpCryptoManager_AES_RSA* manager =
        (httpCryptoManager_AES_RSA*)httpContext::crypto_manager;

      if (!manager)
        OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
                      CORBA::COMPLETED_NO);
      
      manager->addServer(url, public_key, is_filename);
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char removeServer_doc[] =
  "removeServer(url)\n"
  "\n"
  "Remove the server with the specified URL. Returns True if there was a\n"
  "server to remove; False if it was not known.\n";

  static PyObject* pyhttpCrypto_removeServer(PyObject* self, PyObject* args)
  {
    const char* url;

    if (!PyArg_ParseTuple(args, (char*)"s", &url))
      return 0;

    CORBA::Boolean ret;
    
    try {
      httpCryptoManager_AES_RSA* manager =
        (httpCryptoManager_AES_RSA*)httpContext::crypto_manager;

      if (!manager)
        OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
                      CORBA::COMPLETED_NO);
      
      ret = manager->removeServer(url);
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    
    PyObject* r_o = ret ? Py_True : Py_False;
    Py_INCREF(r_o);
    return r_o;
  }


  static PyMethodDef omnihttpCrypto_methods[] = {
    {(char*)"init",
     pyhttpCrypto_init, METH_VARARGS,
     init_doc},

    {(char*)"addClient",
     pyhttpCrypto_addClient, METH_VARARGS,
     addClient_doc},

    {(char*)"removeClient",
     pyhttpCrypto_removeClient, METH_VARARGS,
     removeClient_doc},

    {(char*)"addServer",
     pyhttpCrypto_addServer, METH_VARARGS,
     addServer_doc},

    {(char*)"removeServer",
     pyhttpCrypto_removeServer, METH_VARARGS,
     removeServer_doc},

    {0,0}
  };

#if (PY_VERSION_HEX < 0x03000000)

  void DLL_EXPORT init_omnihttpCrypto()
  {
    PyObject* m = Py_InitModule((char*)"_omnihttpCrypto",
                                omnihttpCrypto_methods);

    // Get the omniORBpy C++ API, which is used in
    // OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    PyObject* omnipy = PyImport_ImportModule((char*)"_omnipy");
    PyObject* pyapi  = PyObject_GetAttrString(omnipy, (char*)"API");
    api              = (omniORBpyAPI*)PyCObject_AsVoidPtr(pyapi);
    Py_DECREF(pyapi);
  }

#else

  static struct PyModuleDef omnihttpCryptomodule = {
    PyModuleDef_HEAD_INIT,
    "_omnihttpCrypto",
    "omniORBpy Crypto library",
    -1,
    omnihttpCrypto_methods,
    NULL,
    NULL,
    NULL,
    NULL
  };

  PyMODINIT_FUNC
  PyInit__omnihttpCrypto(void)
  {
    // Get the omniORBpy C++ API, which is used in
    // OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    api = (omniORBpyAPI*)PyCapsule_Import("_omnipy.API", 0);

    return PyModule_Create(&omnihttpCryptomodule);
  }

#endif
};
