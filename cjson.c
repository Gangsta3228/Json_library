#include <Python.h>

static PyObject* cjson_loads(PyObject* self, PyObject* args);
static PyObject* cjson_dumps(PyObject* self, PyObject* args);

static PyMethodDef cjson_methods[] = {
    {"loads", cjson_loads, METH_VARARGS, "Parse JSON string"},
    {"dumps", cjson_dumps, METH_VARARGS, "Serialize dictionary to JSON string"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cjson_module = {
    PyModuleDef_HEAD_INIT,
    "cjson",
    "JSON parsing and serialization",
    -1,
    cjson_methods
};

PyMODINIT_FUNC PyInit_cjson(void) {
    return PyModule_Create(&cjson_module);
}

static PyObject* parse_value(const char** json_str);

static PyObject* parse_object(const char** json_str) {
    PyObject* dict = PyDict_New();
    if (!dict) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create dictionary");
        return NULL;
    }

    (*json_str)++; 

    while (**json_str != '\0') {
        
        if (**json_str != '"') {
            PyErr_Format(PyExc_TypeError, "Expected object key (string), found '%c'", **json_str);
            Py_DECREF(dict);
            return NULL;
        }
        (*json_str)++; 

        const char* key_start = *json_str;
        while (**json_str != '"') {
            (*json_str)++;
        }
        PyObject* key = PyUnicode_FromStringAndSize(key_start, *json_str - key_start);
        if (!key) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create string object");
            Py_DECREF(dict);
            return NULL;
        }
        (*json_str)++; 

        
        while (**json_str == ' ' || **json_str == ':') {
            (*json_str)++;
        }

        
        PyObject* value = parse_value(json_str);
        if (!value) {
            Py_DECREF(key);
            Py_DECREF(dict);
            return NULL;
        }

        
        if (PyFloat_Check(value)) {
            double number = PyFloat_AsDouble(value);
            if (number == (long)number) {
                Py_DECREF(value);
                value = PyLong_FromDouble(number);
            }
        }

        
        if (PyDict_SetItem(dict, key, value) < 0) {
            Py_DECREF(key);
            Py_DECREF(value);
            Py_DECREF(dict);
            PyErr_SetString(PyExc_RuntimeError, "Failed to set item in dictionary");
            return NULL;
        }

        Py_DECREF(key);
        Py_DECREF(value);

        
        while (**json_str == ' ' || **json_str == ',') {
            (*json_str)++;
        }

        
        if (**json_str == '}') {
            (*json_str)++;
            break;
        }
    }

    return dict;
}

static PyObject* parse_array(const char** json_str) {
    PyObject* list = PyList_New(0);
    if (!list) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create list");
        return NULL;
    }

    (*json_str)++; 

    while (**json_str != '\0') {
        
        PyObject* value = parse_value(json_str);
        if (!value) {
            Py_DECREF(list);
            return NULL;
        }

        
        if (PyList_Append(list, value) < 0) {
            Py_DECREF(value);
            Py_DECREF(list);
            PyErr_SetString(PyExc_RuntimeError, "Failed to append item to list");
            return NULL;
        }

        Py_DECREF(value);

        
        while (**json_str == ' ' || **json_str == ',') {
            (*json_str)++;
        }

        
        if (**json_str == ']') {
            (*json_str)++;
            break;
        }
    }

    return list;
}

static PyObject* parse_value(const char** json_str) {
    if (**json_str == '{') {
        return parse_object(json_str);
    } else if (**json_str == '[') {
        return parse_array(json_str);
    } else if (**json_str == '"') {
        (*json_str)++; 

        const char* value_start = *json_str;
        while (**json_str != '"') {
            (*json_str)++;
        }
        PyObject* value = PyUnicode_FromStringAndSize(value_start, *json_str - value_start);
        if (!value) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create string object");
            return NULL;
        }

        (*json_str)++; 
        return value;
    } else if (**json_str == '-' || (**json_str >= '0' && **json_str <= '9')) {
        char* endptr;
        char* str_end;
        long long_value = strtol(*json_str, &endptr, 10);
        double double_value = strtod(*json_str, &str_end);

        if (endptr == str_end) {
            
            PyObject* value = PyLong_FromLong(long_value);
            if (!value) {
                PyErr_SetString(PyExc_RuntimeError, "Failed to create int object");
                return NULL;
            }
            *json_str = endptr;
            return value;
        } else {
            
            PyObject* value = PyFloat_FromDouble(double_value);
            if (!value) {
                PyErr_SetString(PyExc_RuntimeError, "Failed to create float object");
                return NULL;
            }
            *json_str = str_end;
            return value;
        }
    } else if (**json_str == 't' && strncmp(*json_str, "true", 4) == 0) {
        PyObject* value = PyBool_FromLong(1);
        if (!value) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create boolean object");
            return NULL;
        }
        *json_str += 4; 
        return value;
    } else if (**json_str == 'f' && strncmp(*json_str, "false", 5) == 0) {
        PyObject* value = PyBool_FromLong(0);
        if (!value) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create boolean object");
            return NULL;
        }
        *json_str += 5; 
        return value;
    } else if (**json_str == 'n' && strncmp(*json_str, "null", 4) == 0) {
        PyObject* value = Py_None;
        Py_INCREF(value);
        *json_str += 4; 
        return value;
    } else {
        PyErr_Format(PyExc_TypeError, "Unexpected token '%c'", **json_str);
        return NULL;
    }
}

static PyObject* cjson_loads(PyObject* self, PyObject* args) {
    const char* json_string;

    if (!PyArg_ParseTuple(args, "s", &json_string)) {
        return NULL;
    }

    
    const char* json_str = json_string;
    PyObject* result = parse_value(&json_str);

    
    if (result == NULL || *json_str != '\0') {
        Py_XDECREF(result);
        PyErr_Format(PyExc_TypeError, "Invalid JSON string: '%s'", json_string);
        return NULL;
    }

    return result;
}

static PyObject* serialize_value(PyObject* value);

static PyObject* serialize_object(PyObject* object) {
    PyObject* dict_items = PyDict_Items(object);
    if (!dict_items) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get dictionary items");
        return NULL;
    }

    PyObject* json_str = PyUnicode_FromString("{");
    if (!json_str) {
        Py_DECREF(dict_items);
        PyErr_SetString(PyExc_RuntimeError, "Failed to create string object");
        return NULL;
    }

    Py_ssize_t size = PyList_Size(dict_items);
    for (Py_ssize_t i = 0; i < size; i++) {
        PyObject* item = PyList_GetItem(dict_items, i);
        PyObject* key = PyTuple_GetItem(item, 0);
        PyObject* value = PyTuple_GetItem(item, 1);

        PyObject* serialized_key = serialize_value(key);
        if (!serialized_key) {
            Py_DECREF(dict_items);
            Py_DECREF(json_str);
            return NULL;
        }

        PyObject* serialized_value = serialize_value(value);
        if (!serialized_value) {
            Py_DECREF(dict_items);
            Py_DECREF(json_str);
            Py_DECREF(serialized_key);
            return NULL;
        }

        PyObject* pair_str = PyUnicode_FromFormat("%s: %s", PyUnicode_AsUTF8(serialized_key),
                                                 PyUnicode_AsUTF8(serialized_value));
        if (!pair_str) {
            Py_DECREF(dict_items);
            Py_DECREF(json_str);
            Py_DECREF(serialized_key);
            Py_DECREF(serialized_value);
            PyErr_SetString(PyExc_RuntimeError, "Failed to create string object");
            return NULL;
        }

        Py_DECREF(serialized_key);
        Py_DECREF(serialized_value);

        PyUnicode_Append(&json_str, pair_str);
        Py_DECREF(pair_str);

        if (i < size - 1) {
            PyUnicode_Append(&json_str, PyUnicode_FromString(", "));
        }
    }

    Py_DECREF(dict_items);

    PyUnicode_Append(&json_str, PyUnicode_FromString("}"));

    return json_str;
}

static PyObject* serialize_list(PyObject* list) {
    Py_ssize_t size = PyList_Size(list);

    PyObject* json_str = PyUnicode_FromString("[");
    if (!json_str) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create string object");
        return NULL;
    }

    for (Py_ssize_t i = 0; i < size; i++) {
        PyObject* value = PyList_GetItem(list, i);

        PyObject* serialized_value = serialize_value(value);
        if (!serialized_value) {
            Py_DECREF(json_str);
            return NULL;
        }

        PyUnicode_Append(&json_str, serialized_value);
        Py_DECREF(serialized_value);

        if (i < size - 1) {
            PyUnicode_Append(&json_str, PyUnicode_FromString(","));
        }
    }

    PyUnicode_Append(&json_str, PyUnicode_FromString("]"));

    return json_str;
}

static PyObject* serialize_value(PyObject* value) {
    if (PyUnicode_Check(value)) {
        return PyUnicode_FromFormat("\"%s\"", PyUnicode_AsUTF8(value));
    } else if (PyFloat_Check(value)) {
        char* str_value = PyOS_double_to_string(PyFloat_AsDouble(value), 'r', 0, Py_DTSF_ADD_DOT_0, NULL);
        PyObject* json_str = PyUnicode_FromString(str_value);
        PyMem_Free(str_value);
        return json_str;
    } else if (PyLong_Check(value)) {
        PyObject* str_value = PyObject_Str(value);
        if (!str_value) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create string object");
            return NULL;
        }
        PyObject* json_str = PyUnicode_FromFormat("%s", PyUnicode_AsUTF8(str_value));
        Py_DECREF(str_value);
        return json_str;
    } else if (PyBool_Check(value)) {
        if (PyObject_IsTrue(value)) {
            return PyUnicode_FromString("true");
        } else {
            return PyUnicode_FromString("false");
        }
    } else if (PyDict_Check(value)) {
        return serialize_object(value);
    } else if (PyList_Check(value)) {
        return serialize_list(value);
    } else if (value == Py_None) {
        return PyUnicode_FromString("null");
    } else {
        PyErr_SetString(PyExc_TypeError, "Unsupported value type");
        return NULL;
    }
}

static PyObject* cjson_dumps(PyObject* self, PyObject* args) {
    PyObject* object;

    if (!PyArg_ParseTuple(args, "O", &object)) {
        return NULL;
    }

    
    PyObject* json_str = serialize_value(object);
    if (!json_str) {
        return NULL;
    }

    return json_str;
}