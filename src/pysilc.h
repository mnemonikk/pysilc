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

#ifndef __PYSILC_H
#define __PYSILC_H

#include <Python.h>
#include "structmember.h"

#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

#include "pysilc_macros.h"
#include <silc.h>
#include <silcclient.h>
#include <silctypes.h>

typedef struct {
    PyObject_HEAD
//    SilcChannelId   *silcid;
    SilcChannelEntry silcobj;
} PySilcChannel;

typedef struct {
    PyObject_HEAD
//    SilcClientId    *silcid;
    SilcClientEntry  silcobj;
} PySilcUser;

typedef struct {
    PyObject_HEAD
    SilcPKCS        pkcs;
    SilcPublicKey   public;
    SilcPrivateKey  private;
} PySilcKeys;

typedef struct {
    PyObject_HEAD
    
    // members that are callable objects
    PyObject *say, *channel_message, *private_message,  *command;
    PyObject *connected, *disconnected;
        
    PyObject *notify_none;
    PyObject *notify_invite;
    PyObject *notify_join;
    PyObject *notify_leave;
    PyObject *notify_signoff;
    PyObject *notify_topic_set;
    PyObject *notify_nick_change;
    PyObject *notify_cmode_change;
    PyObject *notify_cumode_change;
    PyObject *notify_motd;
    PyObject *notify_channel_change;
    PyObject *notify_server_signoff;
    PyObject *notify_kicked;
    PyObject *notify_killed;
    PyObject *notify_error;
    PyObject *notify_watch;

    PyObject *command_reply_whois;
    PyObject *command_reply_whowas;
    PyObject *command_reply_identify;
    PyObject *command_reply_nick;
    PyObject *command_reply_list;
    PyObject *command_reply_topic;
    PyObject *command_reply_invite;
    PyObject *command_reply_kill;
    PyObject *command_reply_info;
    PyObject *command_reply_stats;
    PyObject *command_reply_ping;
    PyObject *command_reply_oper;
    PyObject *command_reply_join;
    PyObject *command_reply_motd;
    PyObject *command_reply_cmode;
    PyObject *command_reply_cumode;
    PyObject *command_reply_kick;
    PyObject *command_reply_ban;
    PyObject *command_reply_detach;
    PyObject *command_reply_watch;
    PyObject *command_reply_silcoper;
    PyObject *command_reply_leave;
    PyObject *command_reply_users;
    PyObject *command_reply_service;

    PyObject *command_reply_failed; // custom handler
    
    PySilcKeys *keys;
    
    // TODO: not used 
    PyObject *get_auth_method, 
        *verify_public_key, 
        *ask_passphrase, 
        *failure, 
        *key_agreement,
        *ftp,
        *detach;

    SilcClient             silcobj;
    SilcClientConnection   silcconn;
    SilcClientOperations   callbacks;

} PySilcClient;

/* ------------- pysilc module ---------------- */

static PyObject *pysilc_create_key_pair(PyObject *mod, PyObject *args, PyObject *kwds);
static PyObject *pysilc_load_key_pair(PyObject *mod, PyObject *args, PyObject *kwds);

static PyMethodDef pysilc_functions[] = {
    {
        "create_key_pair", 
        (PyCFunction)pysilc_create_key_pair, 
        METH_VARARGS|METH_KEYWORDS, 
        "create_key_pair(public_filename, private_filename,\n"
        "                identifier = None, passphrase = \"\"\n"
        "                pkcs_name = None, key_length = 2048)\n\n"
        "Create public and private key pair for use with SILC."
    },

    {
        "load_key_pair", 
        (PyCFunction)pysilc_load_key_pair, 
        METH_VARARGS|METH_KEYWORDS, 
        "load_key_pair(public_filename, private_filename, passphrase = \"\")"
        "\n\n"
        "Load a public and private key pair from files with an optional\n"
        "passphrase.\n\n"
        "If passphrase is None, then it will be prompted by calling the\n"
        "ask_passphrase callback. If passphrase is an empty string\n"
        "(eg. \"\"), then an empty passphrase will be passed."
    },    

    {NULL, NULL, 0, NULL},
};

char *pysilc_doc = "Python SILC Toolkit Bindings.";

/*  ---------------- pysilc channel ------------- */

static PyObject *PySilcChannel_New(SilcChannelEntry channel);
static void      PySilcChannel_Del(PyObject *object);
static PyObject *PySilcChannel_GetAttr(PyObject *self, PyObject *name);
static PyObject *PySilcChannel_Str(PyObject *self);
static int PySilcChannel_Compare(PyObject *self, PyObject *other);

static PyMethodDef pysilc_channel_methods[] = {
    {NULL, NULL, 0, NULL},
    
};

static PyMemberDef pysilc_channel_members[] = {
    {NULL, 0, 0, 0, NULL},
};

/*  ---------------- pysilc user object  ------------- */

static PyObject *PySilcUser_New(SilcClientEntry user);
static void PySilcUser_Del(PyObject *object);
static PyObject *PySilcUser_GetAttr(PyObject *self, PyObject *name);
static PyObject *PySilcUser_Str(PyObject *self);
static int PySilcUser_Compare(PyObject *self, PyObject *other);

static PyMethodDef pysilc_user_methods[] = {
    {NULL, NULL, 0, NULL},
};

static PyMemberDef pysilc_user_members[] = {
    {NULL, 0, 0, 0, NULL},
};

/*  ---------------- pysilc keys ------------- */

static PyObject *PySilcKeys_New(SilcPKCS pkcs, SilcPublicKey public, SilcPrivateKey private);
static void PySilcKeys_Del(PyObject *object);

static PyMethodDef pysilc_keys_methods[] = {
    {NULL, NULL, 0, NULL},
};

static PyMemberDef pysilc_keys_members[] = {
    {NULL, 0, 0, 0, NULL},
};

/*  ---------------- pysilc client ------------- */

static int  PySilcClient_Init(PyObject *self, PyObject *args, PyObject *kwds);
static void PySilcClient_Del(PyObject *obj);

static PyObject *pysilc_client_connect_to_server(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *pysilc_client_send_channel_message(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *pysilc_client_send_private_message(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *pysilc_client_command_call(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *pysilc_client_set_away_message(PyObject *self, PyObject *args);
static PyObject *pysilc_client_run_one(PyObject *self);
static PyObject *pysilc_client_remote_host(PyObject *self);
static PyObject *pysilc_client_user(PyObject *self);

static PyMethodDef pysilc_client_methods[] = {
    { 
        "connect_to_server", 
        (PyCFunction)pysilc_client_connect_to_server, 
        METH_VARARGS | METH_KEYWORDS, 
        "connect_to_server(host, port = 706) -> int\n\n"
        "Connect to SILC server. Returns -1 on error."
    },
    {
        "run_one", 
        (PyCFunction)pysilc_client_run_one, 
        METH_NOARGS, 
        "run_one()\n\n"
        "Run one iteration of the run loop."
    },
    {
        "send_channel_message", 
        (PyCFunction)pysilc_client_send_channel_message, 
        METH_VARARGS | METH_KEYWORDS, 
        "send_channel_message(channel, messsage, private_key = None,\n"
        "                     flags = 0, force_send = False)\n\n"
        "Send a message (Unicode string) to a channel (SilcChannel object).\n"
        "Setting 'force_send' to True means send the packet immediately\n."
        "TODO: flags and private_key support not implemented.\n"
    },
    {
        "send_private_message", 
        (PyCFunction)pysilc_client_send_private_message, 
        METH_VARARGS | METH_KEYWORDS, 
        "send_private_message(user, messsage, private_key = None,\n"
        "                     flags = 0, force_send = False)\n\n"
        "Send a message (Unicode string) to a user (SilcUser object).\n"
        "Setting 'force_send' to True means send the packet immediately\n."
        "TODO: flags and private_key support not implemented.\n"
    },
    {
        "command_call", 
        (PyCFunction)pysilc_client_command_call, 
        METH_VARARGS | METH_KEYWORDS, 
        "command_call(string) -> int\n\n"
        "Send a command call to the server and returns the result code of\n"
        "of the command."
    },
    {
        "set_away_message", 
        (PyCFunction)pysilc_client_set_away_message, 
        METH_VARARGS, 
        "set_away_message(message = None)\n\n"
        "Set away message. If message is None, away status is removed."
    },    
    {
        "remote_host", 
        (PyCFunction)pysilc_client_remote_host, 
        METH_NOARGS, 
        "remote_host() -> string\n\n"
        "Get remote hostname."
    },
    {
        "user", 
        (PyCFunction)pysilc_client_user, 
        METH_NOARGS, 
        "user() -> User\n\n"
        "Get current user."
    },    
    {NULL, NULL, 0, NULL},
};

static PyMemberDef pysilc_client_members[] = {
    /* callbacks */
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, say,
                          "say(message)\n\n"
                          "Callback function when server status needs to be\n"
                          "conveyed to the user. 'message' is a string"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, channel_message,
                          "channel_message(sender, channel, flags, message)" 
                          "\n\n"
                          "Callback function when client receives a channel\n"
                          "message. 'sender' is type SilcUser,\n"
                          "'channel' is type SilcChannel, 'flags' is an int\n"
                          "and 'message' is of type string."),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, private_message,
                          "private_message(sender, flags, message)\n\n"
                          "Callback function when client receives a private\n"
                          "message . 'sender' is type SilcUser, 'flags' is a\n"
                          "number and 'message' is of type string."),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command,
                          "command(success, command_no, command_name, message)"
                          "\n\n"
                          "Callback function after command is completed on\n"
                          "the server. 'success' is a boolean, 'command_no'\n"
                          "is an integer, and 'command_name' and 'message'\n"
                          "is a string."),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, connected,
                          "connected()\n\n"
                          "Callback function when client is connected\n"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, disconnected,
                          "disconnected(message)\n\n"
                          "Callback function when client is disconnected\n"
                          "'message' is a string"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_none,
                          "notify_none(string)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_invite,
                          "notify_invite(channel, channel_name, user)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_join,
                          "notify_join(inviter, channel)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_leave,
                          "notify_leave(user_leaving, channel)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_signoff,
                          "notify_signoff(user_signedoff, message)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_topic_set,
                          "notify_topic_set(type, user, channel, topic)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_nick_change,
                          "notify_nick_change(old_user, new_user)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_cmode_change,
                          "TODO: not implemented"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_cumode_change,
                          "TODO: not implemented"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_motd,
                          "notify_motd(message)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_channel_change,
                          "notify_channel_change(channel)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_server_signoff,
                          "notify_server_signoff()"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_kicked,
                          "notify_kicked(kicked, message, kicker, channel)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_killed,
                          "TODO: not implemented"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_error,
                          "notify_error(errcode, message)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_watch,
                          "notify_watch(user, new_nickname, usermode,"
                          "notification, public_key)"),

    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_whois,
                          "command_reply_whois(user, nickname, username,"
                          "  realname, usermode, idletime)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_whowas,
                          "command_reply_whowas(user, nickname, username,"
                          "  realname)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_identify,
                          "command_reply_identify(name, info)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_nick,
                          "command_reply_nick(user, nick, client_id)\n\n"
                          "TODO: client_id not implemented"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_list,
                          "command_reply_list(channel, name, topic, user_count)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_topic,
                          "command_reply_topic(channel, topic)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_invite,
                          "TODO: not implemented"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_kill,
                          "command_reply_kill(user)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_info,
                          "TODO: not implemented"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_stats,
                          "TODO: not implemented"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_ping,
                          "command_reply_ping()"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_oper,
                          "command_reply_oper()"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_join,
                          "command_reply_join(channel, channel_name, topic,"
                          "  hmac_name, 0, 0, list_of_users)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_motd,
                          "command_reply_motd(message)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_cmode,
                          "command_reply_cmode(channel, mode, user_limit, "
                          "None, None)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_cumode,
                          "command_reply_cumode(mode, channel, user)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_kick,
                          "command_reply_kick(channel, user)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_ban,
                          "command_reply_ban(channel, ban_list)\n"
                          "TODO: ban list not implemented"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_detach,
                          "command_reply_detach()"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_watch,
                          "command_reply_watch()"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_silcoper,
                          "command_reply_silcoper()"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_leave,
                          "command_reply_leave(channel)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_users,
                          "command_reply_users(channel, list_of_users)"),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_service,
                          "TODO: not implemented"),

    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_failed,
                          "command_reply_failed(command, command_name, status"
                          ", msg)"),    
    {NULL, 0, 0, 0, NULL},
};

#define PYSILC_CLIENT_DOC  "\
  SilcClient(keys, nickname = \"\", username = \"\",\n\ realname = \"\",\
             hostname = \"\")\n\n\
  A SILC Client. 'keys' is a SilcKeys representing a public private\n\
  key pair. 'nickname', 'username', 'realname' and 'hostname'\n\
  are details of the local client. If not supplied they will be\n\
  determined automatically.\n\n\
  A developer should subclass SilcClient and at the\n\
  very least implement the callbacks by overriding class members:\n\
  'say', 'channel_message', 'private_message', 'command',\n\
  'connected' and 'disconnected'."

static PyTypeObject PySilcClient_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0, /* ob_size */
    "SilcClient", /* tp_name */
    sizeof(PySilcClient), /* tp_basicsize */
    0, /* tp_itemsize */
    PySilcClient_Del, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_compare */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_has */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */    
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    PYSILC_CLIENT_DOC, /* tp_doc */
    0, /* tp_traverse */
    0, /* tp_call */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    pysilc_client_methods, /* tp_methods */
    pysilc_client_members, /* tp_members */
    0, /* tp_getset */
    0, /* tp_base */
    0, /* tp_dict */
    0, /* tp_descr_get */
    0, /* tp_descr_set */
    0, /* tp_dictoffset */
    PySilcClient_Init, /* tp_init */
    0, /* tp_alloc */
    PyType_GenericNew, /* tp_new */
};

#define PYSILC_CHANNEL_DOC "A Silc Channel Object.\n\n\
Attributes accessible:\n\n\
  channel_name = string\n\n\
  channel_id = string (64-160bit)\n\n\
  mode = int\n\n\
  topic = string\n\n\
  user_limit = int \n\n\
  resolve_cmd_ident = int"

static PyTypeObject PySilcChannel_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0, /* ob_size */
    "SilcChannel", /* tp_name */
    sizeof(PySilcChannel), /* tp_basicsize */
    0, /* tp_itemsize */
    PySilcChannel_Del, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    PySilcChannel_Compare, /* tp_compare */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_has */
    0, /* tp_call */
    PySilcChannel_Str, /* tp_str */
    PySilcChannel_GetAttr, /* tp_getattro */
    0, /* tp_setattro */    
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    PYSILC_CHANNEL_DOC, /* tp_doc */
    0, /* tp_traverse */
    0, /* tp_call */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    pysilc_channel_methods, /* tp_methods */
    pysilc_channel_members, /* tp_members */
    0, /* tp_getset */
    0, /* tp_base */
    0, /* tp_dict */
    0, /* tp_descr_get */
    0, /* tp_descr_set */
    0, /* tp_dictoffset */
    0, /* tp_init */
    0, /* tp_alloc */
    0, /* tp_new */
};

#define PYSILC_USER_DOC "A Silc User Object.\n\n\
Accessible Attributes:\n\n\
  nickname = string\n\n\
  username = string\n\n\
  hostname = string\n\n\
  server = string\n\n\
  realname = string\n\n\
  fingerprint = string\n\n\
  fingerprint_len = int\n\n\
  user_id = string (64/160bit)\n\n\
  mode = int\n\n\
  status = int\n\n\
  resolve_cmd_ident = int"

static PyTypeObject PySilcUser_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0, /* ob_size */
    "SilcUser", /* tp_name */
    sizeof(PySilcUser), /* tp_basicsize */
    0, /* tp_itemsize */
    PySilcUser_Del, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    PySilcUser_Compare, /* tp_compare */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_has */
    0, /* tp_call */
    PySilcUser_Str, /* tp_str */
    PySilcUser_GetAttr, /* tp_getattro */
    0, /* tp_setattro */    
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    PYSILC_USER_DOC, /* tp_doc */
    0, /* tp_traverse */
    0, /* tp_call */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    pysilc_user_methods, /* tp_methods */
    pysilc_user_members, /* tp_members */
    0, /* tp_getset */
    0, /* tp_base */
    0, /* tp_dict */
    0, /* tp_descr_get */
    0, /* tp_descr_set */
    0, /* tp_dictoffset */
    0, /* tp_init */
    0, /* tp_alloc */
    0, /* tp_new */
};

#define PYSILC_KEYS_DOC "Silc Key Pair. These are generated by\n\
silc.create_key_pair and/or silc.load_key_pair and is required\n\
by SilcClient."

static PyTypeObject PySilcKeys_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0, /* ob_size */
    "SilcKeys", /* tp_name */
    sizeof(PySilcKeys), /* tp_basicsize */
    0, /* tp_itemsize */
    PySilcKeys_Del, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_compare */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_has */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */    
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    PYSILC_KEYS_DOC, /* tp_doc */
    0, /* tp_traverse */
    0, /* tp_call */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    pysilc_user_methods, /* tp_methods */
    pysilc_user_members, /* tp_members */
    0, /* tp_getset */
    0, /* tp_base */
    0, /* tp_dict */
    0, /* tp_descr_get */
    0, /* tp_descr_set */
    0, /* tp_dictoffset */
    0, /* tp_init */
    0, /* tp_alloc */
    0, /* tp_new */
};


#endif /* __PYSILC_H */

