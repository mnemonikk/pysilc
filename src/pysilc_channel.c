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

static PyObject *PySilcChannel_New(SilcChannelEntry channel)
{
    PySilcChannel *pychannel = (PySilcChannel *)PyObject_New(PySilcChannel, &PySilcChannel_Type);
    if (!pychannel)
        return NULL;
    
    pychannel->silcobj = channel;           // TODO: maybe we need to do a clone?
    pychannel->silcobj->context = pychannel; // TODO: self ref should be weak ref?
    PyObject_Init((PyObject *)pychannel, &PySilcChannel_Type);
    return (PyObject *)pychannel;
}
static void PySilcChannel_Del(PyObject *object)
{
    ((PySilcChannel *)object)->silcobj = NULL;
    PyObject_Del(object);
}

static PyObject *PySilcChannel_GetAttr(PyObject *self, PyObject *name)
{
    // expose the following attributes as readonly
    // - char *channel_name
    // - 64/160 bit channel id
    // - unsigned int mode
    // - char * topic
    // - (TODO) founder_key
    // - unsigned int user_limit
    // - (TODO) user_list
    
    int result;
    PyObject *temp = NULL, *value = NULL;
    PySilcChannel *pychannel = (PySilcChannel *)self;
  
    if (!pychannel->silcobj)
        goto cleanup;
  
    // check for topic
    temp = PyString_FromString("topic");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        if (pychannel->silcobj->topic)
            value = PyString_FromString(pychannel->silcobj->topic);
        else {
            value = Py_None;
            Py_INCREF(value);
        }
        goto cleanup;
    }
    
    // check for channel_name
    Py_DECREF(temp);
    temp = PyString_FromString("channel_name");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        if (pychannel->silcobj->channel_name)
            value = PyString_FromString(pychannel->silcobj->channel_name);
        else {
            value = Py_None;
            Py_INCREF(value);
        }
        goto cleanup;
    }

    // check for channel id
    Py_DECREF(temp);    
    temp = PyString_FromString("channel_id");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        char buf[160];
        memcpy(&buf, &(pychannel->silcobj->id), 160);
        value = PyString_FromStringAndSize(buf, 160);
        goto cleanup;
    }
    
    // check for mode
    Py_DECREF(temp);    
    temp = PyString_FromString("mode");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        value = PyInt_FromLong(pychannel->silcobj->mode);
        goto cleanup;
    }
    
    // check for user_limit
    Py_DECREF(temp);    
    temp = PyString_FromString("user_limit");
    if (PyObject_Cmp(temp, name, &result) == -1)
        goto cleanup;
    if (result == 0) {
        value = PyInt_FromLong(pychannel->silcobj->user_limit);
        goto cleanup;
    }
    
cleanup:
    Py_XDECREF(temp);
    if (value)
        return value;
    else
        return PyObject_GenericGetAttr(self, name);    
}

static PyObject *PySilcChannel_Str(PyObject *self)
{
    return PyObject_GetAttrString(self, "channel_name");
}

static int PySilcChannel_Compare(PyObject *self, PyObject *other)
{
    if (!PyObject_IsInstance(other, (PyObject *)&PySilcChannel_Type)) {
        PyErr_SetString(PyExc_TypeError, "Can only compare with SilcChannel.");
        return -1;
    }
    
    int result = 0;
    PyObject *channel_name = PyObject_GetAttrString(self, "channel_name");
    PyObject *other_name = PyObject_GetAttrString(self, "channel_name");
    if (!channel_name || !other_name) {
        PyErr_SetString(PyExc_RuntimeError, "Does not have channel name");
        return -1;
    }
    
    result = PyObject_Compare(channel_name, other_name);
    Py_DECREF(channel_name);
    Py_DECREF(other_name);
    return result;
}


static PyObject *PySilcKeys_New(SilcPublicKey public, SilcPrivateKey private)
{
    PySilcKeys *pykeys = (PySilcKeys *)PyObject_New(PySilcKeys, &PySilcKeys_Type);
    if (!pykeys)
        return NULL;
    
    pykeys->private = private;
    pykeys->public = public;
    
    PyObject_Init((PyObject *)pykeys, &PySilcKeys_Type);
    return (PyObject *)pykeys;
}
static void PySilcKeys_Del(PyObject *object)
{
    // TODO: free them properly
    PyObject_Del(object);
}
