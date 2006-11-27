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

#include "pysilc_channel.c"
#include "pysilc_user.c"
#include "pysilc_callbacks.c"

void initsilc() {    
    PyObject *mod = Py_InitModule3("silc", pysilc_functions, pysilc_doc);
    silc_pkcs_register_default();
    silc_hash_register_default();
    silc_cipher_register_default();
    silc_hmac_register_default();
    PY_MOD_ADD_CLASS(mod, SilcClient);
    PY_MOD_ADD_CLASS(mod, SilcChannel);    
    PY_MOD_ADD_CLASS(mod, SilcUser);
    PyModule_AddIntConstant(mod, "SILC_ID_CLIENT", SILC_ID_CLIENT);
    PyModule_AddIntConstant(mod, "SILC_ID_CHANNEL", SILC_ID_CHANNEL);
    PyModule_AddIntConstant(mod, "SILC_ID_SERVER", SILC_ID_SERVER);
}

static int PySilcClient_Init(PyObject *self, PyObject *args, PyObject *kwds)
{
    PySilcClient *pyclient = (PySilcClient *)self;
    pyclient->callbacks.say =               _pysilc_client_callback_say;
    pyclient->callbacks.channel_message =   _pysilc_client_callback_channel_message;
    pyclient->callbacks.private_message =   _pysilc_client_callback_private_message;
    pyclient->callbacks.notify =            _pysilc_client_callback_notify;
    pyclient->callbacks.command =           _pysilc_client_callback_command;
    pyclient->callbacks.command_reply =     _pysilc_client_callback_command_reply;
    pyclient->callbacks.connected =         _pysilc_client_callback_connected;
    pyclient->callbacks.disconnected =      _pysilc_client_callback_disconnected;
    pyclient->callbacks.get_auth_method =   _pysilc_client_callback_get_auth_method;
    pyclient->callbacks.verify_public_key = _pysilc_client_callback_verify_key;
    pyclient->callbacks.ask_passphrase =    _pysilc_client_callback_ask_passphrase;
    pyclient->callbacks.failure =           _pysilc_client_callback_failure;
    pyclient->callbacks.key_agreement =     _pysilc_client_callback_key_agreement;
    pyclient->callbacks.ftp =               _pysilc_client_callback_ftp;
    pyclient->callbacks.detach =            _pysilc_client_callback_detach;

    char *nickname = NULL, *username = NULL, *realname = NULL, *hostname = NULL;
    static char *kwlist[] = {"keys", "nickname", "username", "realname", "hostname", NULL};

    PySilcKeys *keys;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ssss", kwlist, 
                                     &keys, &nickname, &username, &realname, 
                                     &hostname)) {
        return -1;
    }
    
    pyclient->silcobj = silc_client_alloc(&(pyclient->callbacks), NULL, pyclient, silc_version_string);
    if (!pyclient->silcobj) {
        PyErr_SetString(PyExc_AssertionError, "Failed to Initialise Silc Client Object");
        return -1;
    }
    
    if (!PyObject_TypeCheck(keys, &PySilcKeys_Type))
        return -1;
        
    pyclient->silcconn = NULL;
    
    if (nickname)
        pyclient->silcobj->nickname = strdup(nickname);
    if (username)
        pyclient->silcobj->username = strdup(username);
    else
        pyclient->silcobj->username = silc_get_username();
    if (realname)
        pyclient->silcobj->realname = strdup(realname);
    else
        pyclient->silcobj->realname = silc_get_real_name();
    if (hostname)
        pyclient->silcobj->hostname = strdup(hostname);
    else
        pyclient->silcobj->hostname = silc_net_localhost();

    pyclient->silcobj->pkcs         = keys->pkcs;
    pyclient->silcobj->public_key   = keys->public;
    pyclient->silcobj->private_key  = keys->private;
    
    pyclient->keys = keys;
    Py_INCREF(keys);
    
    silc_client_init(pyclient->silcobj);
    return 0;
}

static void PySilcClient_Del(PyObject *obj)
{
    printf("SilcClient.__del__\n");
    PySilcClient *pyclient = (PySilcClient *)obj;
    if (pyclient->silcobj) {
        silc_client_stop(pyclient->silcobj);
        if (pyclient->silcobj->username)
            free(pyclient->silcobj->username);          
        if (pyclient->silcobj->realname)
            free(pyclient->silcobj->realname);        
        if (pyclient->silcobj->hostname)
            free(pyclient->silcobj->hostname);        
        silc_client_free(pyclient->silcobj);
    }
    Py_XDECREF(pyclient->keys);
    obj->ob_type->tp_free(obj);
}

static PyObject *pysilc_client_connect_to_server(PyObject *self, PyObject *args, PyObject *kwds)
{
    int result;
    unsigned int port = 706;
    char *host;
    static char *kwlist[] = {"host", "port", NULL};
    PySilcClient *pyclient = (PySilcClient *)self;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|I", kwlist, &host, &port))
        return NULL;
        
    if (!pyclient || !pyclient->silcobj) {
        PyErr_SetString(PyExc_RuntimeError, "SILC Client Not Initialised");
        return NULL;
    }
            
    result = silc_client_connect_to_server(pyclient->silcobj, NULL, port, host, NULL);
    if (result != -1) {
        Py_INCREF(self);
        return PyInt_FromLong(result);
    }
    
    return PyInt_FromLong(result);
}

static PyObject *pysilc_client_run_one(PyObject *self)
{
    PySilcClient *pyclient = (PySilcClient *)self;    
    if (!pyclient || !pyclient->silcobj) {
           PyErr_SetString(PyExc_RuntimeError, "SILC Client Not Initialised");
           return NULL;
    }
    silc_client_run_one(pyclient->silcobj);
    Py_RETURN_NONE;
}

static PyObject *pysilc_client_remote_host(PyObject *self)
{
    PySilcClient *pyclient = (PySilcClient *)self;    
    if (!pyclient || !pyclient->silcconn) {
           PyErr_SetString(PyExc_RuntimeError, "SILC Client Not Connected");
           return NULL;
    }
    
    return PyString_FromString(pyclient->silcconn->remote_host);
}

static PyObject *pysilc_client_user(PyObject *self)
{
    PySilcClient *pyclient = (PySilcClient *)self;    
    if (!pyclient || !pyclient->silcconn) {
           PyErr_SetString(PyExc_RuntimeError, "SILC Client Not Connected");
           return NULL;
    }
    
    PyObject *myself = PySilcUser_New(pyclient->silcconn->local_entry);
    if (!myself) {
        Py_RETURN_NONE;
    }
    return myself;
}


static PyObject *pysilc_client_send_channel_message(PyObject *self, PyObject *args, PyObject *kwds)
{
    PySilcChannel *channel;
    char *message = NULL;
    int length = 0;
    int result = 0;
    PyObject *private_key = NULL; // TODO: ignored at the moment
    unsigned int defaultFlags = SILC_MESSAGE_FLAG_UTF8;
    unsigned int flags = 0;
    bool force_send = 1;
    PySilcClient *pyclient = (PySilcClient *)self;    

    static char *kwlist[] = {"channel", "msg", "private_key", "flags", "force_send", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oes#|OIb", kwlist, &channel, "utf-8", &message, &length, &private_key, &flags, &force_send))
        return NULL;
        
    if (!PyObject_IsInstance((PyObject *)channel, (PyObject *)&PySilcChannel_Type))
        return NULL;
        
    if (!pyclient || !pyclient->silcobj) {
        PyErr_SetString(PyExc_RuntimeError, "SILC Client Not Initialised");
        return NULL;
     }
        
    result = silc_client_send_channel_message(pyclient->silcobj, 
                                              pyclient->silcconn, 
                                              channel->silcobj, 
                                              NULL,
                                              flags | defaultFlags,
                                              message, length, 
                                              force_send);
    
    return PyInt_FromLong(result);
}


static PyObject *pysilc_client_send_private_message(PyObject *self, PyObject *args, PyObject *kwds)
{
    PySilcUser *user;
    char *message = NULL;
    int length = 0;
    int result = 0;
    unsigned int defaultFlags = SILC_MESSAGE_FLAG_UTF8;
    unsigned int flags = 0;
    bool force_send = 1;
    PySilcClient *pyclient = (PySilcClient *)self;    
    
    
    static char *kwlist[] = {"user", "message", "flags", "force_send", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oes#|Ib", kwlist, &user, "utf-8", &message, &length, &flags, &force_send))
        return NULL;
        
    if (!PyObject_IsInstance((PyObject *)user, (PyObject *)&PySilcUser_Type))
        return NULL;
        
    if (!pyclient || !pyclient->silcobj) {
        PyErr_SetString(PyExc_RuntimeError, "SILC Client Not Initialised");
        return NULL;
     }
        
    result = silc_client_send_private_message(pyclient->silcobj, 
                                              pyclient->silcconn, 
                                              user->silcobj, 
                                              flags | defaultFlags,
                                              message, 
                                              length, 
                                              force_send);
    
    return PyInt_FromLong(result);
}

static PyObject *pysilc_client_command_call(PyObject *self, PyObject *args, PyObject *kwds)
{
    char *message;
    int result;
    PySilcClient *pyclient = (PySilcClient *)self;    
    
    if (!pyclient || !pyclient->silcobj) {
       PyErr_SetString(PyExc_RuntimeError, "SILC Client Not Initialised");
       return NULL;
    }
    
    if (!PyArg_ParseTuple(args, "s", &message))
        return NULL;
    
    result = silc_client_command_call(pyclient->silcobj, pyclient->silcconn, message);
    return PyInt_FromLong(result);
}


static PyObject *pysilc_client_set_away_message(PyObject *self, PyObject *args)
{
    char *message;
    int length;
    PyObject *temp = NULL;
    PySilcClient *pyclient = (PySilcClient *)self;    
    
    if (!pyclient || !pyclient->silcobj) {
       PyErr_SetString(PyExc_RuntimeError, "SILC Client Not Initialised");
       return NULL;
    }
    
    if (!PyArg_ParseTuple(args, "|O", &temp))
        return NULL;
        
    if ((temp == Py_None) || (temp == NULL)) {
        silc_client_set_away_message(pyclient->silcobj, pyclient->silcconn, NULL);    
        Py_RETURN_NONE;
    }
    
    if (!PyArg_ParseTuple(args, "s#", &message, length))
        return NULL;
    
    if (length < 1)
        silc_client_set_away_message(pyclient->silcobj, pyclient->silcconn, NULL);    
    else
        silc_client_set_away_message(pyclient->silcobj, pyclient->silcconn, message);
        
    Py_RETURN_NONE;
}

static PyObject *pysilc_create_key_pair(PyObject *mod, PyObject *args, PyObject *kwds)
{
	PyObject *passphrase_obj = Py_None;
    char *pkcs_name = NULL;
    char *pub_filename , *prv_filename;
    char *passphrase = NULL;
    char *pub_identifier = NULL;
    
    SilcUInt32      key_length = 2048;
    SilcPKCS        pkcs;
    SilcPublicKey   public_key;
    SilcPrivateKey  private_key;
    
    static char *kwlist[] = {"public_filename", "private_filename", "identifier", "passphrase", "pkcs_name", "key_length", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss|sOsi", kwlist,
            &pub_filename, &prv_filename, &pub_identifier,
			&passphrase_obj, &pkcs_name, &key_length))
        return NULL;
    
	if (passphrase_obj == Py_None) {
		passphrase = NULL;
	}
	else if (PyString_Check(passphrase_obj)) {
		passphrase = PyString_AsString(passphrase_obj);
	}
	else {
		PyErr_SetString(PyExc_TypeError, "passphrase should either be None or String Type");
		return NULL;
	}	
	
    bool result = silc_create_key_pair(pkcs_name, key_length, pub_filename, 
                                       prv_filename, pub_identifier, passphrase,
                                       &pkcs, &public_key, &private_key, 0);
    if (!result) {
        PyErr_SetString(PyExc_RuntimeError, "Unable to generate keys.");
        return NULL;
    }
        
    return PySilcKeys_New(pkcs, public_key, private_key);
}

static PyObject *pysilc_load_key_pair(PyObject *mod, PyObject *args, PyObject *kwds)
{
	PyObject *passphrase_obj = Py_None;
	char *passphrase = NULL;
    char *pub_filename , *prv_filename;
	
    SilcPKCS        pkcs;
    SilcPublicKey   public_key;
    SilcPrivateKey  private_key;
    
    static char *kwlist[] = {"public_filename", "private_filename", "passphrase", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss|O", kwlist,
            &pub_filename, &prv_filename, &passphrase_obj))
        return NULL;
	
	if (passphrase_obj == Py_None) {
		passphrase = NULL;
	}
	else if (PyString_Check(passphrase_obj)) {
		passphrase = PyString_AsString(passphrase_obj);
	}
	else {
		PyErr_SetString(PyExc_TypeError, "passphrase should either be None or String Type");
		return NULL;
	}

	// Use the passphrase passed.
    bool result = silc_load_key_pair(pub_filename, prv_filename, 
									 passphrase,
									 &pkcs, &public_key, &private_key);
	
    if (!result) {
		PyErr_SetString(PyExc_RuntimeError, "Unable to load keys.");
		return NULL;
    }
        
    return PySilcKeys_New(pkcs, public_key, private_key);
}
