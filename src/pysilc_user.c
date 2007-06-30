/*
 *
 * PySilc - Python SILC Toolkit Bindings
 *
 * Copyright (c) 2006, Alastair Tse <alastair@liquidx.net>
 * All rights reserved.
 *
 * This program is free software; you can redistributed it and/or modify 
 * it under the terms of the BSD License. See LICENSE in the distribution
 * for details or http://www.liquidx.net/pysilc/.
 *
 */

#include "pysilc.h"

static PyObject *PySilcUser_New(SilcClientEntry user)
{
    PySilcUser *pyuser = (PySilcUser *)PyObject_New(PySilcUser, &PySilcUser_Type);
    if (!pyuser)
        return NULL;
    
    pyuser->silcobj = user;             // TODO: maybe we need to do a clone?
    pyuser->silcobj->context = pyuser;  // TODO: self ref should be weak ref?
    PyObject_Init((PyObject *)pyuser, &PySilcUser_Type);
    return (PyObject *)pyuser;
}
static void PySilcUser_Del(PyObject *object)
{
    ((PySilcUser *)object)->silcobj = NULL;
    PyObject_Del(object);
}

static PyObject *PySilcUser_GetAttr(PyObject *self, PyObject *name)
{
    // expose the following attributes as readonly
    // - char *nickname
    // - char *username
    // - char *hostname
    // - char *server
    // - char *realname
    // - unsigned char *fingerprint;
    //   SilcUInt32 finderprint_len;
    //
    // - 64/160 bit user id
    // - unsigned int mode
    // - (TODO) attrs;
    // - (TODO) public_key;
    // - int status
    // - (TODO) channels
    
    int result;
    PyObject *temp = NULL, *value = NULL;
    PySilcUser *pyuser = (PySilcUser *)self;
  
    if (!pyuser->silcobj)
        goto cleanup;
  
    // check for nickname
    temp = PyString_FromString("nickname");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        if (pyuser->silcobj->nickname)
            value = PyString_FromString(pyuser->silcobj->nickname);
        else {
            value = Py_None;
            Py_INCREF(value);
        }
        goto cleanup;
    }
  
    // check for username
    Py_DECREF(temp);
    temp = PyString_FromString("username");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        if (pyuser->silcobj->username)
            value = PyString_FromString(pyuser->silcobj->username);
        else {
            value = Py_None;
            Py_INCREF(value);
        }
        goto cleanup;
    }

    // check for hostname
    Py_DECREF(temp);
    temp = PyString_FromString("hostname");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        if (pyuser->silcobj->hostname)
            value = PyString_FromString(pyuser->silcobj->hostname);
        else {
            value = Py_None;
            Py_INCREF(value);
        }
        goto cleanup;
    }
    
    // check for server
    Py_DECREF(temp);
    temp = PyString_FromString("server");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        if (pyuser->silcobj->server)
            value = PyString_FromString(pyuser->silcobj->server);
        else {
            value = Py_None;
            Py_INCREF(value);
        }
        goto cleanup;
    }    
  
    // check for realname
    Py_DECREF(temp);
    temp = PyString_FromString("realname");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        if (pyuser->silcobj->realname)
            value = PyString_FromString(pyuser->silcobj->realname);
        else {
            value = Py_None;
            Py_INCREF(value);
        }
        goto cleanup;
    }
    
    // check for fingerprint
    Py_DECREF(temp);
    temp = PyString_FromString("fingerprint");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        if (pyuser->silcobj->fingerprint)
            value = PyString_FromStringAndSize(pyuser->silcobj->fingerprint, 20);
        else {
            value = Py_None;
            Py_INCREF(value);
        }
        goto cleanup;
    }
  
    // check for user id
    Py_DECREF(temp);
    temp = PyString_FromString("user_id");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        char buf[224];
        memcpy(&buf, &(pyuser->silcobj->id), 224);
        value = PyString_FromStringAndSize(buf, 224);
        goto cleanup;
    }
    
    // check for mode
    Py_DECREF(temp);
    temp = PyString_FromString("mode");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        value = PyInt_FromLong(pyuser->silcobj->mode);
        goto cleanup;
    }
    
    // check for status
    Py_DECREF(temp);
    temp = PyString_FromString("status");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        value = PyInt_FromLong(pyuser->silcobj->status);
        goto cleanup;
    }
    
cleanup:
    Py_XDECREF(temp);
    if (value)
        return value;
    else
        return PyObject_GenericGetAttr(self, name);    
}

static PyObject *PySilcUser_Str(PyObject *self)
{
    PySilcUser *pyuser = (PySilcUser *)self;
    if (pyuser->silcobj) {
        PyObject *str = PyString_FromFormat("%s <%s@%s> on %s", 
            pyuser->silcobj->nickname,
            pyuser->silcobj->username, 
            pyuser->silcobj->hostname,
            pyuser->silcobj->server);
        return str;
    }
    return PyObject_Str(self);
}

static int PySilcUser_Compare(PyObject *self, PyObject *other)
{
    if (!PyObject_IsInstance(other, (PyObject *)&PySilcUser_Type)) {
        PyErr_SetString(PyExc_TypeError, "Can only compare with SilcUser.");
        return -1;
    }
    
    int result = 0;
    PyObject *user_name = PyObject_GetAttrString(self, "user_name");
    PyObject *other_name = PyObject_GetAttrString(self, "user_name");
    if (!user_name || !other_name) {
        PyErr_SetString(PyExc_RuntimeError, "Does not have user name");
        return -1;
    }
    
    result = PyObject_Compare(user_name, other_name);
    Py_DECREF(user_name);
    Py_DECREF(other_name);
    return result;
}
