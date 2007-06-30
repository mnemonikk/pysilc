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

#define PYSILC_GET_CLIENT_OR_DIE(source, destination)\
   PySilcClient *destination = (PySilcClient *)source->application;\
    if (!destination)\
        return;

#define PYSILC_NEW_USER_OR_DIE(source, destination)\
    PySilcUser *destination = (PySilcUser *)PySilcUser_New(source);\
    if (!destination)\
        return;

#define PYSILC_NEW_CHANNEL_OR_DIE(source, destination)\
    PySilcChannel *destination = (PySilcChannel *)PySilcChannel_New(source);\
    if (!destination)\
        return;

#define PYSILC_NEW_USER_OR_BREAK(source, destination)\
    destination = PySilcUser_New(source);\
    if (!destination)\
    break;

#define PYSILC_NEW_CHANNEL_OR_BREAK(source, destination)\
    destination = PySilcChannel_New(source);\
    if (!destination)\
        break;

#define PYSILC_GET_CALLBACK_OR_BREAK(name)\
    callback = PyObject_GetAttrString((PyObject *)pyclient, name);\
    if (!PyCallable_Check(callback))\
        break;

#define PYSILC_SILCBUFFER_TO_PYLIST(source, destination, Type) \
    do { } while (0);

static void _pysilc_client_connect_callback(SilcClient client,
                                            SilcClientConnection conn,
                                            SilcClientConnectionStatus status,
                                            SilcStatus error,
                                            const char *message,
                                            void *context)
{

    if ((status == SILC_CLIENT_CONN_SUCCESS) || (status == SILC_CLIENT_CONN_SUCCESS_RESUME))
        PyObject *result = NULL, *callback = NULL;
        PYSILC_GET_CLIENT_OR_DIE(client, pyclient);

        if (error != SILC_STATUS_OK) {
            // TODO: raise an exception and abort
            // call silc_client_close_connection(client, conn);
            pyclient->silcconn = NULL;
            goto cleanup1; 
        }

        pyclient->silcconn = conn;

        callback = PyObject_GetAttrString((PyObject *)pyclient, "connected");
        if (!PyCallable_Check(callback))
            goto cleanup1;
        if ((result = PyObject_CallObject(callback, NULL)) == 0)
            PyErr_Print();
        cleanup1:
            Py_XDECREF(callback);
            Py_XDECREF(result);
    }
    else if (status == SILC_CLIENT_CONN_DISCONNECTED) {
        PyObject *result = NULL, *callback = NULL, *args = NULL;
        PYSILC_GET_CLIENT_OR_DIE(client, pyclient);

        if (status != SILC_STATUS_OK) {
            // TODO: raise an exception and abort
            // call silc_client_close_connection(client, conn);
        }

        // TODO: we're not letting the user know about ClientConnection atm.
        pyclient->silcconn = NULL;
        callback = PyObject_GetAttrString((PyObject *)pyclient, "disconnected");
        if (!PyCallable_Check(callback))
            goto cleanup2;

        if (!(args = Py_BuildValue("(s)", message)))
            goto cleanup2;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        cleanup2:
            Py_XDECREF(callback);
            Py_XDECREF(args);
            Py_XDECREF(result);
        }
    }
    else {
        PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
        PyObject *callback = NULL, *result = NULL;  

        callback = PyObject_GetAttrString((PyObject *)pyclient, "failure");
        if (!PyCallable_Check(callback))
            goto cleanup3;
        // TODO: pass on protocol, failure parameters
        if ((result = PyObject_CallObject(callback, NULL)) == 0)
            PyErr_Print();
        cleanup3:
            Py_XDECREF(callback);
            Py_XDECREF(result);
    }
}

static void _pysilc_client_callback_say(SilcClient client,
                                        SilcClientConnection conn,
                                        SilcClientMessageType type,
                                        char *msg, ...) {

    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    PyObject *result = NULL, *args = NULL, *callback = NULL;
    
    callback = PyObject_GetAttrString((PyObject *)pyclient, "say");
    if (!PyCallable_Check(callback))
        goto cleanup;

    if (!(args = Py_BuildValue("(s)", msg)))
        goto cleanup;
        
    if ((result = PyObject_CallObject(callback, args)) == 0)
        PyErr_Print();        
    
cleanup:    
    Py_XDECREF(callback);
    Py_XDECREF(args);
    Py_XDECREF(result);
}

static void _pysilc_client_callback_command(SilcClient client, 
                                            SilcClientConnection conn,
                                            bool success,
                                            SilcCommand command, 
                                            SilcStatus status) 
{
    PyObject *callback = NULL, *args = NULL, *result = NULL;
    
    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    callback = PyObject_GetAttrString((PyObject *)pyclient, "command");
    if (!PyCallable_Check(callback))
        goto cleanup;
        
    if (!(args = Py_BuildValue("(biss)", success, command,  
                               silc_get_command_name(command),
                               silc_get_status_message(status))))
        goto cleanup;
    if ((result = PyObject_CallObject(callback, args)) == 0)
        PyErr_Print();    
cleanup:
    Py_XDECREF(callback);
    Py_XDECREF(result);
    Py_XDECREF(args);
}


static void _pysilc_client_callback_channel_message(SilcClient client, 
                                                    SilcClientConnection conn,
                                                    SilcClientEntry sender, 
                                                    SilcChannelEntry channel,
                                                    SilcMessagePayload payload,
                                                    SilcChannelPrivateKey key, 
                                                    SilcMessageFlags flags,
                                                    const unsigned char *message,
                                                    SilcUInt32 message_len) {
    
    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    PYSILC_NEW_USER_OR_DIE(sender, pysender);
    PYSILC_NEW_CHANNEL_OR_DIE(channel, pychannel);
    PyObject *result = NULL, *args = NULL, *callback = NULL;
    
    callback = PyObject_GetAttrString((PyObject *)pyclient, "channel_message");
    if (!PyCallable_Check(callback))
        goto cleanup;
        
    if (!(args = Py_BuildValue("(OOis#)", pysender, pychannel, flags, message, message_len)))
        goto cleanup;
    if ((result = PyObject_CallObject(callback, args)) == 0)
        PyErr_Print();        
    
cleanup:
    Py_XDECREF(callback);
    Py_XDECREF(args);
    Py_XDECREF(result);
}

    
static void _pysilc_client_callback_private_message(SilcClient client, 
                                                    SilcClientConnection conn,
                                                    SilcClientEntry sender, 
                                                    SilcMessagePayload payload,
                                                    SilcMessageFlags flags,
                                                    const unsigned char *message,
                                                    SilcUInt32 message_len) {
    
    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    PYSILC_NEW_USER_OR_DIE(sender, pysender);
    PyObject *result = NULL, *args = NULL, *callback = NULL;
        
    callback = PyObject_GetAttrString((PyObject *)pyclient, "private_message");
    if (!PyCallable_Check(callback))
        goto cleanup;

    if (!(args = Py_BuildValue("(Ois#)", pysender, flags, message, message_len)))
        goto cleanup;
    if ((result = PyObject_CallObject(callback, args)) == 0)
        PyErr_Print();

cleanup:
    Py_XDECREF(callback);
    Py_XDECREF(args);
    Py_XDECREF(result);
}

typedef struct _PySilcClient_Callback_Join_Context
{
   char *channel_name;
   char *topic;
   char *hmac_name;
   PyObject *pychannel;
   SilcUInt32 channel_mode;
   SilcUInt32 user_limit; 
} PySilcClient_Callback_Join_Context;

static void _pysilc_client_callback_command_reply_join_finished(SilcClient client,
                                                                SilcClientConnection conn,
                                                                SilcClientEntry *user_list,
                                                                SilcUInt32 user_count,
                                                                void * context)
{
    PyObject *result = NULL, *callback = NULL, *args = NULL;
    PyObject *pytopic = NULL, *pyhmac_name = NULL, *users = NULL;
    PySilcClient_Callback_Join_Context *join_context = NULL;
    
    if (!context)
        return;
        
    join_context = (PySilcClient_Callback_Join_Context *)context;

    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    callback = PyObject_GetAttrString((PyObject *)pyclient, "command_reply_join");
    if (!PyCallable_Check(callback))
        goto cleanup;

    // extract all the users
    SilcUInt32 i = 0;
    users = PyTuple_New(user_count);    
    for (i = 0; i < user_count; i++) {
        PyObject *u = PySilcUser_New(user_list[i]);
        PyTuple_SetItem(users, i, u);
        // TODO: we don't DECREF because PyTuple doesn't incr ref count.
    }
    
    // prepare some possibly NULL values
    if (join_context->topic == NULL) {
        pytopic = Py_None;
        Py_INCREF(Py_None);
    }
    else {
        pytopic = PyString_FromString(join_context->topic);
        free(join_context->topic);
    }
    if (join_context->hmac_name == NULL) {
        pyhmac_name = Py_None;
        Py_INCREF(Py_None);
    }
    else {
        pyhmac_name = PyString_FromString(join_context->hmac_name);
        free(join_context->hmac_name);
    }
    
    if (!(args = Py_BuildValue("(OsOOiiO)", join_context->pychannel,
                                         join_context->channel_name,
                                         pytopic,
                                         pyhmac_name,
                                         0, 0, users)))
        goto cleanup;
    
    if ((result = PyObject_CallObject(callback, args)) == 0)
        PyErr_Print();

    cleanup:
    if (join_context != NULL) {
        if (join_context->channel_name) 
            free(join_context->channel_name);
        Py_XDECREF(join_context->pychannel);
        free(join_context);
    }
    Py_XDECREF(users);
    Py_XDECREF(pytopic);
    Py_XDECREF(pyhmac_name);
    Py_XDECREF(callback);
    Py_XDECREF(args);
    Py_XDECREF(result);
}                                                                


static void _pysilc_client_callback_notify(SilcClient client, 
                                           SilcClientConnection conn,
                                           SilcNotifyType type, ...) {

    PyObject *args = NULL, *result = NULL, *pyuser = NULL, *pychannel = NULL;
    PyObject *callback = NULL, *pyarg = NULL;
    SilcIdType idtype;
    SilcUInt32 mode;
    void *entry = NULL;
    char *topic = NULL;

    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    va_list va;
    va_start(va, type);

    switch(type) {
    case SILC_NOTIFY_TYPE_NONE:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_none");
        if (!(args = Py_BuildValue("(s)", (char *)va_arg(va, char*))))
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
        
    case SILC_NOTIFY_TYPE_INVITE:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_invite");
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        char *channel_name = va_arg(va, char *);
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        if ((args = Py_BuildValue("(OsO)", pychannel, channel_name, pyuser)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;

    case SILC_NOTIFY_TYPE_JOIN:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_join");
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        if ((args = Py_BuildValue("(OO)", pyuser, pychannel)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;

    case SILC_NOTIFY_TYPE_LEAVE:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_leave");
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        if ((args = Py_BuildValue("(OO)", pyuser, pychannel)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;
    case SILC_NOTIFY_TYPE_SIGNOFF:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_signoff");
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        char *msg = va_arg(va, char *);
        if (!msg) 
            msg = "";
        if ((args = Py_BuildValue("(Os)", pyuser, msg)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;
    case SILC_NOTIFY_TYPE_TOPIC_SET:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_topic_set");
        idtype = va_arg(va, int);
        entry = va_arg(va, void *);
        topic = va_arg(va, char *);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);

        switch (idtype) {
            case SILC_ID_CLIENT:
                PYSILC_NEW_USER_OR_BREAK(entry, pyarg);
                break;
            case SILC_ID_CHANNEL:
                PYSILC_NEW_CHANNEL_OR_BREAK(entry, pyarg);
                break;
            case SILC_ID_SERVER:
                pyarg = Py_None;
                Py_INCREF(pyarg); // TODO: no server type
                break;
        }
        
        args = Py_BuildValue("(iOOs)", 
            idtype, 
            pyarg, 
            pychannel, 
            topic);
            
        if (args == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;

    case SILC_NOTIFY_TYPE_NICK_CHANGE:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_nick_change");
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyarg);
        if ((args = Py_BuildValue("(OO)", pyuser, pyarg)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;

    case SILC_NOTIFY_TYPE_CMODE_CHANGE:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_cmode_change");
        idtype = va_arg(va, int);
        entry = va_arg(va, void *);
        mode = va_arg(va, SilcUInt32);
        char *cipher_name = va_arg(va, char *);
        char *hmac_name = va_arg(va, char *);
        char *passphrase = va_arg(va, char *);
        SilcPublicKey founder_key = va_arg(va, SilcPublicKey);
        SilcBuffer channel_pubkeys = va_arg(va, SilcBuffer);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        
        switch (idtype) { 
            case SILC_ID_CLIENT:
                PYSILC_NEW_USER_OR_BREAK(entry, pyarg);
                break;
            case SILC_ID_CHANNEL:
                PYSILC_NEW_CHANNEL_OR_BREAK(entry, pyarg);
                break;
            case SILC_ID_SERVER:
                pyarg = Py_None; // TODO: no server objects
                Py_INCREF(Py_None);
                break;
        }

        args = Py_BuildValue("(iOOissss)",
            idtype,
            pyarg, 
            mode,
            cipher_name,
            hmac_name,
            passphrase,
            Py_None,
            Py_None,
            pychannel);

        if (args == NULL)
            break;
            
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;

    case SILC_NOTIFY_TYPE_CUMODE_CHANGE:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_cumode_change");
        idtype = va_arg(va, int);
        entry = va_arg(va, void *);
        mode = va_arg(va, SilcUInt32);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        switch (idtype) { 
        case SILC_ID_CLIENT:
            PYSILC_NEW_USER_OR_BREAK(entry, pyarg);
            break;
        case SILC_ID_CHANNEL:
            PYSILC_NEW_CHANNEL_OR_BREAK(entry, pyarg);
            break;
        case SILC_ID_SERVER:
            pyarg = Py_None; // TODO: no server objects
            Py_INCREF(Py_None);
            break;
        }
        
        args = Py_BuildValue("(iOiOO)",
                             idtype,
                             pyarg,
                             mode,
                             pychannel,
                             pyuser);
             
        if (args == NULL) 
            break;
        
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;

    case SILC_NOTIFY_TYPE_MOTD:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_motd");
        if ((args = Py_BuildValue("(s)", va_arg(va, char *))) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    case SILC_NOTIFY_TYPE_CHANNEL_CHANGE:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_channel_change");
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        if ((args = Py_BuildValue("(O)", pychannel)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
        
    case SILC_NOTIFY_TYPE_SERVER_SIGNOFF:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_server_signoff");
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();        
        break;
        
    case SILC_NOTIFY_TYPE_KICKED:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_kicked");
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyarg);
        char *message = va_arg(va, char *);
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        
        if ((args = Py_BuildValue("(OsOO)", pyarg, message, pyuser, pychannel)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;

    case SILC_NOTIFY_TYPE_KILLED:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_killed");
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        char *kill_message = va_arg(va, char *);
        idtype = va_arg(va, int);
        entry = va_arg(va, void *);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        switch (idtype) { 
        case SILC_ID_CLIENT:
             PYSILC_NEW_USER_OR_BREAK(entry, pyarg);
             break;
        case SILC_ID_CHANNEL:
            PYSILC_NEW_CHANNEL_OR_BREAK(entry, pyarg);
            break;
        case SILC_ID_SERVER:
            pyarg = Py_None; // TODO: no server objects
            Py_INCREF(Py_None);
            break;
        }

        args = Py_BuildValue("(OsOO)",
         pyuser, // client that was killed
         kill_message,
         pyarg, // the killer, either a SilcClient or SilcChannel or None
         pychannel);
 
        if (args == NULL) 
            break;

        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;
        
    case SILC_NOTIFY_TYPE_ERROR:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_error");
        int error = va_arg(va, int);
        if ((args = Py_BuildValue("(is)", error, silc_get_status_message(error))) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    case SILC_NOTIFY_TYPE_WATCH:
        PYSILC_GET_CALLBACK_OR_BREAK("notify_watch");
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        char *new_nick = va_arg(va, char *);
        SilcUInt32 user_mode = va_arg(va, SilcUInt32);
        SilcNotifyType notification = va_arg(va, int);
        void *dummy = va_arg(va, SilcPublicKey); // TODO
        if ((args = Py_BuildValue("(OsiiO)", pyuser, new_nick, user_mode, notification, Py_None)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    }

 cleanup:
    va_end(va); 
    Py_XDECREF(callback);
    Py_XDECREF(pyuser);
    Py_XDECREF(pychannel);
    Py_XDECREF(pyarg);
    Py_XDECREF(result);
    Py_XDECREF(args);
}




static void _pysilc_client_callback_command_reply(SilcClient client, 
                                                  SilcClientConnection conn,
                                                  SilcCommand command, 
                                                  SilcStatus status,
                                                  SilcStatus error, ...)
{
    PyObject *args = NULL, *result = NULL, *pyuser = NULL, *pychannel = NULL;
    PyObject *callback = NULL, *pyarg = NULL;

    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    va_list va;
    va_start(va, error);
    
    if (status != SILC_STATUS_OK) {
        // we encounter an error, return the command and error
        callback = PyObject_GetAttrString((PyObject *)pyclient, 
                                          "command_reply_failed");
        if (!PyCallable_Check(callback))
            return;
        if (!(args = Py_BuildValue("(isis)", command, 
                                   silc_get_command_name(command), 
                                   error,
                                   silc_get_status_message(error)))) {
            Py_DECREF(callback);
            return;
        }
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();

        Py_DECREF(callback);
        Py_DECREF(args);
        return;
    }
    
    switch(command) {
    case SILC_COMMAND_WHOIS:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_whois");
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        char *nickname, *username, *realname;
        SilcUInt32 usermode, idletime;
        unsigned char *fingerprint;
        void *dummy;
        nickname = va_arg(va, char *);
        username = va_arg(va, char *);
        realname = va_arg(va, char *);
        dummy = va_arg(va, void *);
        usermode = va_arg(va, SilcUInt32);
        idletime = va_arg(va, SilcUInt32);
        fingerprint = va_arg(va, unsigned char *);
        // TODO: fill in fingerprint, channels, channel_usermodes, attrs
        if ((args = Py_BuildValue("(Osssii)", pyuser, nickname, username, realname, usermode, idletime)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
     }   
    case SILC_COMMAND_WHOWAS:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_whowas");        
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        char *nickname, *username, *realname;
        nickname = va_arg(va, char *);
        username = va_arg(va, char *);
        realname = va_arg(va, char *);
        if ((args = Py_BuildValue("(Osss)", pyuser, nickname, username, realname)) == NULL)
             break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();             
        break;
    }
    case SILC_COMMAND_IDENTIFY:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_identify");        
        void *entry = va_arg(va, void *); // TODO: what is this?
        char *name = va_arg(va, char *);
        char *info = va_arg(va, char *);
        if ((args = Py_BuildValue("(ss)", name, info)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    }
    case SILC_COMMAND_NICK:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_nick");        
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);
        char *nickname = va_arg(va, char *);
        SilcClientID *info = va_arg(va, SilcClientID *); // TODO: pass this?
        if ((args = Py_BuildValue("(Oss)", pyuser, nickname, "")) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    }
    case SILC_COMMAND_LIST:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_list");        
        pychannel = PySilcChannel_New(va_arg(va, SilcChannelEntry));
        char *channel_name = va_arg(va, char *);
        char *channel_topic = va_arg(va, char *);        
        SilcUInt32 user_count = va_arg(va, SilcUInt32);
        if (channel_name == NULL && channel_topic == NULL) {
            // no channels on server
            if ((args = Py_BuildValue("(OOOi)", Py_None, Py_None, Py_None, 0)) == NULL)
                break;
        }
        else {
            if ((args = Py_BuildValue("(Ossi)", pychannel, channel_name, channel_topic, user_count)) == NULL)
                break;
        }
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();        
        break;
    }  
    case SILC_COMMAND_TOPIC:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_topic");        
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        char *channel_topic = va_arg(va, char *);        
        if ((args = Py_BuildValue("(Os)", pychannel, channel_topic)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    }
    case SILC_COMMAND_INVITE:
        /* TODO: extracting from payload is weird

        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_invite");
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);

        // get invite list
        SilcBuffer invitelist = va_arg(va, SilcBuffer);
        int invite_count;
        SILC_GET16_MSB(invite_count, invitelist);
        SilcArgumentPayload invites = silc_argument_payload_parse(invitelist->data, 
                                                                  invitelist->len, 
                                                                  invite_count);
        PyObject *pyargs = PyTuple_New(invite_count);
        char *invitee; 
        int i = 0;
        SilcUInt32 len, silctype;
        while (invitee = silc_argument_get_next_arg(invites, &silctype, &len)) {
            PyTuple_SetItem(pyargs, i, PyString_FromString(invitee));
            i++;
        }

        if ((args = Py_BuildValue("(OO)", pychannel, pyargs)) == NULL)
            
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        */
        break;

    case SILC_COMMAND_KILL:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_kill");        
        pyuser = PySilcUser_New(va_arg(va, SilcClientEntry));
        if (!pyuser) {
            // spec says this can be null
            pyuser = Py_None;
            Py_INCREF(pyuser);
        }
        if ((args = Py_BuildValue("(O)", pyuser)) == NULL)
             break;
         if ((result = PyObject_CallObject(callback, args)) == 0)
             PyErr_Print();             
        break;
    }
    case SILC_COMMAND_INFO:
        // TODO: unimplemented
        break;
        
    case SILC_COMMAND_STATS:
        // TODO: unimplemented
        break;
        
    case SILC_COMMAND_PING:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_ping");    
        printf("command ping callback found\n")    ;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();        
        break;
    }
    case SILC_COMMAND_OPER:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_oper");        
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();        
        break;
    }
    case SILC_COMMAND_JOIN:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_join");        
        PySilcClient_Callback_Join_Context *context = malloc(sizeof(PySilcClient_Callback_Join_Context));
        memset(context, 0, sizeof(PySilcClient_Callback_Join_Context));
        if (!context)
            break;
            
        char *tmpstr = NULL;
        int ignored;
        void *dummy;
        SilcUInt32 client_count;
        SilcBuffer client_list;
        
        
        tmpstr = va_arg(va, char *);
        if (tmpstr)
            context->channel_name = strdup(tmpstr);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        context->pychannel = pychannel;
        Py_INCREF(pychannel);
        context->channel_mode = va_arg(va, SilcUInt32);
        ignored = va_arg(va, int);  // ignored: ignore        
        dummy = va_arg(va, void *); // ignored: key_payload
        dummy = va_arg(va, void *); // NULL
        dummy = va_arg(va, void *); // NULL
        tmpstr = va_arg(va, char *);
        if (tmpstr)
            context->topic = strdup(tmpstr);
        tmpstr = va_arg(va, char *);
        if (tmpstr)
            context->hmac_name = strdup(tmpstr);
        client_count = va_arg(va, SilcUInt32);
        client_list = va_arg(va, SilcBuffer);
        dummy = va_arg(va, void *); // TODO: SilcBuffer client_mode_list
        dummy = va_arg(va, void *); // TODO: SilcPublicKey founder_key
        dummy = va_arg(va, void *); // TODO: SilcBuffer channel_pubkeys
        context->user_limit = va_arg(va, SilcUInt32);
            
        silc_client_get_clients_by_list(client, conn, 
                                        client_count, client_list,
                                        _pysilc_client_callback_command_reply_join_finished,
                                        context);
        break;
    }
    case SILC_COMMAND_MOTD:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_join");
        char *motd = va_arg(va, char *);
        if ((args = Py_BuildValue("(s)", motd)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    }
    case SILC_COMMAND_CMODE:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_cmode");
        SilcUInt32 mode, user_limit;
        void *dummy;
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        mode = va_arg(va, SilcUInt32);
        dummy = va_arg(va, void *); // TODO: founder_key
        dummy = va_arg(va, void *); // TODO: SilcBuffer channel_pubkeys;
        user_limit = va_arg(va, SilcUInt32);
        
        if ((args = Py_BuildValue("(OiiOO)", pychannel, mode, user_limit, Py_None, Py_None)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    }     
    case SILC_COMMAND_CUMODE:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_cumode");
        SilcUInt32 mode = va_arg(va, SilcUInt32);
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);        
        if ((args = Py_BuildValue("(iOO)", mode, pychannel, pyuser)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();
        break;
    }
    case SILC_COMMAND_KICK:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_kick");
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        PYSILC_NEW_USER_OR_BREAK(va_arg(va, SilcClientEntry), pyuser);        
        if ((args = Py_BuildValue("(OO)", pychannel, pyuser)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    }
    case SILC_COMMAND_BAN:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_ban");
         PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
         void *dummy = va_arg(va, SilcBuffer); //TODO: ban_list
         if ((args = Py_BuildValue("(OO)", pychannel, Py_None)) == NULL)
             break;
         if ((result = PyObject_CallObject(callback, args)) == 0)
             PyErr_Print();             
         break;
    }
    case SILC_COMMAND_DETACH:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_detach");
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();        
        break;
    }   
    case SILC_COMMAND_WATCH:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_watch");
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();        
        break;   
    }   
    case SILC_COMMAND_SILCOPER:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_silcoper");
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();        
        break;        
    }
    case SILC_COMMAND_LEAVE:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_leave");
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        if ((args = Py_BuildValue("(O)", pychannel)) == NULL)
            break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();            
        break;
    } 
    case SILC_COMMAND_USERS:
    {
        PYSILC_GET_CALLBACK_OR_BREAK("command_reply_users");
        PYSILC_NEW_CHANNEL_OR_BREAK(va_arg(va, SilcChannelEntry), pychannel);
        
        // get all users from this channel .. tedious
        SilcUInt32 user_count = va_arg(va, SilcUInt32);
        pyuser = PyTuple_New(user_count); // hijack pyuser so we get autocleanup
        int i = 0;
        SilcHashTableList hash_list;
        SilcClientEntry user, cached;
        SilcChannelUser user_channel;
        PyObject *u = NULL;
        SilcChannelEntry channel = ((PySilcChannel *)pychannel)->silcobj;

        printf("user: %d\n", user_count);
        
        if (channel && channel->user_list) {
            silc_hash_table_list(channel->user_list, &hash_list);
            while (silc_hash_table_get(&hash_list, (void *)&user, (void *)&user_channel)) {
                cached = silc_client_get_client_by_id(client, conn, &(user->id));
                if (cached) {
                    u = PySilcUser_New(cached);
                    PyTuple_SetItem(pyuser, i, u);
                    // TODO: reference count is not incr, so we don't DECREF it. check.
                }
                else {
                    PyTuple_SetItem(pyuser, i, Py_None);
                }
                i++;
            }
            silc_hash_table_list_reset(&hash_list);
        }
        
        // end getting users
        
        if ((args = Py_BuildValue("(OO)", pychannel, pyuser/*list*/)) == NULL)
               break;
        if ((result = PyObject_CallObject(callback, args)) == 0)
            PyErr_Print();        
        break;
    }   
    case SILC_COMMAND_SERVICE:
        // TODO: implement me
        break;
    }
cleanup:
    va_end(va);
    Py_XDECREF(callback);
    Py_XDECREF(result);
    Py_XDECREF(pychannel);
    Py_XDECREF(pyuser);
}

static void _pysilc_client_callback_verify_key(SilcClient client,
                                               SilcClientConnection conn,
                                               SilcConnectionType conn_type,
                                               SilcPublicKey public_key,
                                               SilcVerifyPublicKey completion,
                                               void *context)
{
    // TODO: implement me
    completion(TRUE, context);   
}

static void _pysilc_client_callback_get_auth_method(SilcClient client,
                                                    SilcClientConnection conn,
                                                    char *hostname,
                                                    SilcUInt16 port,
                                                    SilcAuthMethod auth_method,
                                                    SilcGetAuthMeth completion,
                                                    void *context)
{
    // TODO: implement this properly
    completion(SILC_AUTH_PUBLIC_KEY, NULL, 0, context);
}

static bool _pysilc_client_callback_key_agreement(SilcClient client, 
                                            SilcClientConnection conn,
                                            SilcClientEntry client_entry, 
                                            const char *hostname,
                                            SilcUInt16 port, 
                                            SilcKeyAgreementCallback *completion,
                                            void **context)
{
    // TODO :implement me
    return FALSE;
}                                        

static void _pysilc_client_callback_ftp(SilcClient client,
                                        SilcClientConnection conn,
                                        SilcClientEntry client_entry, 
                                        SilcUInt32 session_id,
                                        const char *hostname, SilcUInt16 port)
{
    // TODO :implement me
}

static void _pysilc_client_callback_ask_passphrase(SilcClient client, 
                                                   SilcClientConnection conn,
                                                   SilcAskPassphrase completion, 
                                                   void *context)
{
    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    PyObject *callback = NULL, *result = NULL;
    char *passphrase;
	int length;

    callback = PyObject_GetAttrString((PyObject *)pyclient, "ask_passphrase");
	
    if (!PyCallable_Check(callback))
        goto cleanup;
	
    if ((result = PyObject_CallObject(callback, NULL)) == 0)
        PyErr_Print();

	int success = PyString_AsStringAndSize(result, &passphrase, &length);
	if (success < 0)
		goto cleanup;
	
	completion((unsigned char *)passphrase, length, context);

cleanup:
    Py_XDECREF(callback);
    Py_XDECREF(result);
}

static void _pysilc_client_callback_detach(SilcClient client, 
                                            SilcClientConnection conn,
                                            const unsigned char *detach_data,
                                            SilcUInt32 detach_data_len)
{
    PYSILC_GET_CLIENT_OR_DIE(client, pyclient);
    PyObject *result = NULL, *callback = NULL, *args = NULL;
    callback = PyObject_GetAttrString((PyObject *)pyclient, "detach");
    if (!PyCallable_Check(callback))
        goto cleanup;
        
    if (!(args = Py_BuildValue("(s#)", detach_data, detach_data_len)))
        goto cleanup;
    if ((result = PyObject_CallObject(callback, args)) == 0)
        PyErr_Print();    
    
cleanup:
    Py_XDECREF(callback);
    Py_XDECREF(args);
    Py_XDECREF(result);
}                                            
