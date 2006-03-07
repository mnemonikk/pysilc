#ifndef PYSILC_MACROS_H
#define PYSILC_MACROS_H

#define PY_MOD_ADD_CLASS(mod, Obj) \
    if (PyType_Ready(&Py##Obj##_Type) < 0) { \
        printf("%s: Problem with Py%s_Type\n",""#mod"",""#Obj"");\
        return;\
    }\
    Py_INCREF(&Py##Obj##_Type); \
    PyModule_AddObject(mod, "" #Obj "", (PyObject *)&Py##Obj##_Type);

#define PY_MOD_DEFINE_ADD(mod, MOD, name) \
        PyModule_AddIntConstant(mod, "" #name "", MOD##_##name)

#define PY_MOD_ENUM_ADD(mod, MOD, name) \
        PYE_DEFINE_ADD(mod, MOD, name)

#define PYSILC_MEMBER_OBJ_DEF(obj, mem) \
   {""#mem"", T_OBJECT, offsetof(obj, mem), 0, ""#mem""}

#endif /* PYSILC_MACROS_H */

