// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pysslTP.cc                 Created on: 2002/09/06
//                            Author    : Duncan Grisby (dgrisby)
//
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
//    SSL transport library

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
#include <omniORB4/sslContext.h>

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
  if (sslContext::full_peerdetails) {
    sslContext::PeerDetails* pd = (sslContext::PeerDetails*)conn->peerdetails();

    if (pd) {
      PyDict_SetItemString(d, (char*)"ssl_verified",
                           pd->verified() ? Py_True : Py_False);
    }
  }
}


extern "C" {

  static char set_CA_doc[] =
  "set_CA(ca_file, ca_path)\n"
  "\n"
  "Set the certificate authorities used for validation. ca_file is the path\n"
  "to a file containing one or more CA certificates; ca_path is the path to\n"
  "a directory containing CA certificate files, as accepted by OpenSSL.\n"
  "Either may be None.\n";

  static PyObject* pysslTP_set_CA(PyObject* self, PyObject* args)
  {
    const char* ca_file;
    const char* ca_path;

    if (!PyArg_ParseTuple(args, (char*)"zz", &ca_file, &ca_path)) return 0;

    if (sslContext::singleton) {
      try {
        sslContext::singleton->update_CA(ca_file, ca_path);
      }
      OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    }
    else {
      // Set the globals. We leak copies of the strings, because there
      // is no safe time to free them.
      sslContext::certificate_authority_file = (ca_file ?
                                                CORBA::string_dup(ca_file) : 0);
      sslContext::certificate_authority_path = (ca_path ?
                                                CORBA::string_dup(ca_path) : 0);
    }
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char set_key_doc[] =
  "set_key(key_file, key_password)\n"
  "\n"
  "Set the private key and its password.\n";

  static PyObject* pysslTP_set_key(PyObject* self, PyObject* args)
  {
    const char* key_file;
    const char* password;

    if (!PyArg_ParseTuple(args, (char*)"ss", &key_file, &password)) return 0;

    if (sslContext::singleton) {
      try {
        sslContext::singleton->update_key(key_file, password);
      }
      OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    }
    else {
      // Set the globals. We leak copies of the strings, because there
      // is no safe time to free them.
      sslContext::key_file          = CORBA::string_dup(key_file);
      sslContext::key_file_password = CORBA::string_dup(password);
    }
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char set_cipher_list_doc[] =
  "set_cipher_list(cipher_list)\n"
  "\n"
  "Set the OpenSSL cipher list string.\n";

  static PyObject* pysslTP_set_cipher_list(PyObject* self, PyObject* args)
  {
    const char* cipher_list;

    if (!PyArg_ParseTuple(args, (char*)"s", &cipher_list)) return 0;

    if (sslContext::singleton) {
      try {
        sslContext::singleton->update_cipher_list(cipher_list);
      }
      OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    }
    else {
      // Set the global.
      sslContext::cipher_list = CORBA::string_dup(cipher_list);
    }
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char reinit_doc[] =
  "reinit()\n"
  "\n"
  "Re-initialize the SSL transport to pick up new configuration.\n";
  
  static PyObject* pysslTP_reinit(PyObject* self,
                                  PyObject* args)
  {
    try {
      if (sslContext::singleton) {
        sslContext::singleton->reinit(0);
      }
      else {
        OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
                      CORBA::COMPLETED_NO);
      }
    }
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
    Py_INCREF(Py_None); return Py_None;
  }


  static char certificate_authority_file_doc[] =
  "certificate_authority_file(PEM filename)\n"
  "\n"
  "Deprecated. Use set_CA instead.\n"
  "\n"
  "Set the certificate authority file for SSL validation.\n";

  static PyObject* pysslTP_certificate_authority_file(PyObject* self,
						      PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      if (sslContext::certificate_authority_file)
	return String_FromString(sslContext::certificate_authority_file);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    const char *name;
    if (!PyArg_ParseTuple(args, (char*)"s", &name)) return 0;

    // Leak here, but we can't do anything else about it.
    sslContext::certificate_authority_file = CORBA::string_dup(name);

    if (sslContext::singleton)
      sslContext::singleton->update_CA(sslContext::certificate_authority_file,
                                       sslContext::certificate_authority_path);
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char certificate_authority_path_doc[] =
  "certificate_authority_path(path)\n"
  "\n"
  "Deprecated. Use set_CA instead.\n"
  "\n"
  "Set the path for certificate authority files for SSL validation.\n";

  static PyObject* pysslTP_certificate_authority_path(PyObject* self,
						      PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      if (sslContext::certificate_authority_path)
	return String_FromString(sslContext::certificate_authority_path);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    const char *name;
    if (!PyArg_ParseTuple(args, (char*)"s", &name)) return 0;

    // Leak here, but we can't do anything else about it.
    sslContext::certificate_authority_path = CORBA::string_dup(name);

    if (sslContext::singleton)
      sslContext::singleton->update_CA(sslContext::certificate_authority_file,
                                       sslContext::certificate_authority_path);
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char key_file_doc[] =
  "key_file(PEM filename)\n"
  "\n"
  "Deprecated. Use set_key instead.\n"
  "\n"
  "Set the key file for SSL encryption.\n";

  static PyObject* pysslTP_key_file(PyObject* self,
				    PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      if (sslContext::key_file)
	return String_FromString(sslContext::key_file);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    const char *name;
    if (!PyArg_ParseTuple(args, (char*)"s", &name)) return 0;

    // Leak here, but we can't do anything else about it.
    sslContext::key_file = CORBA::string_dup(name);

    if (sslContext::singleton)
      sslContext::singleton->update_key(sslContext::key_file,
                                        sslContext::key_file_password);
    
    Py_INCREF(Py_None); return Py_None;
  }

  static char key_file_password_doc[] =
  "key_file_password(password string)\n"
  "\n"
  "Deprecated. Use set_key instead.\n"
  "\n"
  "Set the password for the key file.\n";

  static PyObject* pysslTP_key_file_password(PyObject* self,
					     PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      if (sslContext::key_file_password)
	return String_FromString(sslContext::key_file_password);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    const char *pw;
    if (!PyArg_ParseTuple(args, (char*)"s", &pw)) return 0;

    // Leak here, but we can't do anything else about it.
    sslContext::key_file_password = CORBA::string_dup(pw);

    if (sslContext::singleton)
      sslContext::singleton->update_key(sslContext::key_file,
                                        sslContext::key_file_password);
    
    Py_INCREF(Py_None); return Py_None;
  }
    
  static PyMethodDef omnisslTP_methods[] = {
    {(char*)"set_CA",
     pysslTP_set_CA, METH_VARARGS,
     set_CA_doc},

    {(char*)"set_key",
     pysslTP_set_key, METH_VARARGS,
     set_key_doc},

    {(char*)"set_cipher_list",
     pysslTP_set_cipher_list, METH_VARARGS,
     set_cipher_list_doc},

    {(char*)"reinit",
     pysslTP_reinit, METH_VARARGS,
     reinit_doc},

    {(char*)"certificate_authority_file",
     pysslTP_certificate_authority_file, METH_VARARGS,
     certificate_authority_file_doc},

    {(char*)"certificate_authority_path",
     pysslTP_certificate_authority_path, METH_VARARGS,
     certificate_authority_path_doc},

    {(char*)"key_file",
     pysslTP_key_file, METH_VARARGS,
     key_file_doc},

    {(char*)"key_file_password",
     pysslTP_key_file_password, METH_VARARGS,
     key_file_password_doc},

    {0,0}
  };

#if (PY_VERSION_HEX < 0x03000000)

  void DLL_EXPORT init_omnisslTP()
  {
    PyObject* m = Py_InitModule((char*)"_omnisslTP", omnisslTP_methods);

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
    PyDict_SetItemString(callInfoFns, "ssl", pyfn);
  }

#else

  static struct PyModuleDef omnisslTPmodule = {
    PyModuleDef_HEAD_INIT,
    "_omnisslTP",
    "omniORBpy SSL transport",
    -1,
    omnisslTP_methods,
    NULL,
    NULL,
    NULL,
    NULL
  };

  PyMODINIT_FUNC
  PyInit__omnisslTP(void)
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
    PyDict_SetItemString(callInfoFns, "ssl", pyfn);

    return PyModule_Create(&omnisslTPmodule);
  }

#endif
};
