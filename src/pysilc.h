#ifndef __PYSILC_H
#define __PYSILC_H

#include <Python.h>
#include "structmember.h"

#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

#include "pysilc_macros.h"
#include <silcincludes.h>
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
    PyObject *get_auth_method, *verify_public_key, *ask_passphrase, *failure, *key_agreement;
    PyObject *ftp, *detach;

    SilcClient             silcobj;
    SilcClientConnection   silcconn;
    SilcClientOperations   callbacks;
} PySilcClient;

static PyObject *pysilc_create_key_pair(PyObject *mod, PyObject *args, PyObject *kwds);
static PyObject *pysilc_load_key_pair(PyObject *mod, PyObject *args, PyObject *kwds);

static PyMethodDef pysilc_functions[] = {
    {"create_key_pair", (PyCFunction)pysilc_create_key_pair, METH_VARARGS|METH_KEYWORDS, "Create Key Pair"},
    {"load_key_pair", (PyCFunction)pysilc_load_key_pair, METH_VARARGS|METH_KEYWORDS, "Load Key Pair"},    
    {NULL, NULL, 0, NULL},
};

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

static PyObject *PySilcKeys_New(SilcPKCS pkcs, SilcPublicKey public, SilcPrivateKey private);
static void PySilcKeys_Del(PyObject *object);

static PyMethodDef pysilc_keys_methods[] = {
    {NULL, NULL, 0, NULL},
};

static PyMemberDef pysilc_keys_members[] = {
    {NULL, 0, 0, 0, NULL},
};

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
    {"connect_to_server", (PyCFunction)pysilc_client_connect_to_server, METH_VARARGS | METH_KEYWORDS, "Connect to SILC server."},
    {"run_one", (PyCFunction)pysilc_client_run_one, METH_NOARGS, "Run one iteration of the run loop."},
    {"send_channel_message", (PyCFunction)pysilc_client_send_channel_message, METH_VARARGS | METH_KEYWORDS, "Send a message to a channel."},
    {"send_private_message", (PyCFunction)pysilc_client_send_private_message, METH_VARARGS | METH_KEYWORDS, "Send a private message"},
    {"command_call", (PyCFunction)pysilc_client_command_call, METH_VARARGS | METH_KEYWORDS, "Send a command string to the server"},
    {"set_away_message", (PyCFunction)pysilc_client_set_away_message, METH_VARARGS, "Set AWay. Pass None to remove away status"},    
    {"remote_host", (PyCFunction)pysilc_client_remote_host, METH_NOARGS, "Get Remote Host Name"},
    {"user", (PyCFunction)pysilc_client_user, METH_NOARGS, "Get Current User"},    
    {NULL, NULL, 0, NULL},
};

static PyMemberDef pysilc_client_members[] = {
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, say),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, channel_message),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, private_message),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, connected),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, disconnected),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_none),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_invite),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_join),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_leave),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_signoff),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_topic_set),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_nick_change),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_cmode_change),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_cumode_change),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_motd),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_channel_change),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_server_signoff),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_kicked),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_killed),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_error),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, notify_watch),

    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_whois),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_whowas),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_identify),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_nick),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_list),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_topic),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_invite),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_kill),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_info),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_stats),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_ping),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_oper),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_join),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_motd),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_cmode),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_cumode),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_kick),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_ban),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_detach),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_watch),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_silcoper),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_leave),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_users),
    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_service),

    PYSILC_MEMBER_OBJ_DEF(PySilcClient, command_reply_failed),
    
    {NULL, 0, 0, 0, NULL},
};

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
    "Silc Client", /* tp_doc */
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

static PyTypeObject PySilcChannel_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0, /* ob_size */
    "Channel", /* tp_name */
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
    "Silc Channel", /* tp_doc */
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

static PyTypeObject PySilcUser_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0, /* ob_size */
    "User", /* tp_name */
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
    "Silc User", /* tp_doc */
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
    "Silc Keys", /* tp_doc */
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

