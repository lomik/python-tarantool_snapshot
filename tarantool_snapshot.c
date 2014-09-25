#include <Python.h>

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <tarantool/tnt.h>
#include <tarantool/tnt_net.h>
#include <tarantool/tnt_snapshot.h>

static PyObject *SnapshotError;

typedef struct {
    PyObject_HEAD
    struct tnt_stream stream;
    struct tnt_iter iter;
    struct tnt_iter iter_tuple;
    int open_exception;
} SnapshotIterator;

static PyMethodDef SnapshotIterator_Methods[] = {
    {NULL}  /* Sentinel */
};

static int SnapshotIterator_init(SnapshotIterator *self, PyObject *args);
PyObject* SnapshotIterator_del(SnapshotIterator *self);
PyObject* SnapshotIterator_iter(SnapshotIterator *self);
PyObject* SnapshotIterator_iternext(SnapshotIterator *self);

static PyTypeObject SnapshotIterator_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "tarantool_snapshot.SnapshotIterator",      /*tp_name*/
    sizeof(SnapshotIterator),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)SnapshotIterator_del,             /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER,
        /*
         * tp_flags: Py_TPFLAGS_HAVE_ITER tells python to
         * use tp_iter and tp_iternext fields.
        */
    "SnapshotIterator",                /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    (getiterfunc)SnapshotIterator_iter,  /* tp_iter: __iter__() method */
    (iternextfunc)SnapshotIterator_iternext,  /* tp_iternext: next() method */
    SnapshotIterator_Methods,          /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)SnapshotIterator_init,   /* tp_init */
    0                          /* tp_alloc */
};

static int SnapshotIterator_init(SnapshotIterator *self, PyObject *args) {
    char *filename = NULL;
    //size_t len = 0;
    self->open_exception = 0;
    
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        return 1;
    }
    
    tnt_snapshot(&(self->stream));
    
    if (tnt_snapshot_open(&(self->stream), filename) == -1) {
        self->open_exception = 1;
        return 1;
    }

    tnt_iter_storage(&(self->iter), &(self->stream));
    
    return 0;
}

PyObject* SnapshotIterator_iter(SnapshotIterator* self) {
    if (self->open_exception) {
        PyErr_Format(SnapshotError, "Can't open snapshot");
        return NULL;
    }
    tnt_iter_storage(&(self->iter), &(self->stream));
    Py_INCREF(self);
    return (PyObject*)self;
}

PyObject* SnapshotIterator_iternext(SnapshotIterator* self) {
    if (tnt_next(&(self->iter))) {
        struct tnt_iter_storage *is = TNT_ISTORAGE(&(self->iter));
        struct tnt_stream_snapshot* ss = TNT_SSNAPSHOT_CAST(TNT_ISTORAGE_STREAM(&(self->iter)));
        
        PyObject *tuple = PyList_New(0);
        PyObject *s;
        
        tnt_iter(&(self->iter_tuple), &is->t);
        while (tnt_next(&(self->iter_tuple))) {
            char *data = TNT_IFIELD_DATA(&(self->iter_tuple));
            uint32_t size = TNT_IFIELD_SIZE(&(self->iter_tuple));
            s = PyString_FromStringAndSize(data, size);
            PyList_Append(tuple, s);
            Py_DECREF(s);
            //printf("size: %d\n", size);
        }
        if (self->iter_tuple.status == TNT_ITER_FAIL){
            PyErr_Format(SnapshotError, "parsing error");
            return NULL;
        }
        tnt_iter_free(&(self->iter_tuple));
        
        PyObject* tuple_as_tuple = PyList_AsTuple(tuple);
        PyObject* ret = Py_BuildValue("(I,O)", ss->log.current.row_snap.space, tuple_as_tuple);
        Py_DECREF(tuple_as_tuple);
        Py_DECREF(tuple);
        return ret;
    }
    if (self->iter.status == TNT_ITER_FAIL) {
        PyErr_Format(SnapshotError, "Parsing failed: %s", tnt_snapshot_strerror(&(self->stream)));
        return NULL;
    }
    return NULL;
}

PyObject* SnapshotIterator_del(SnapshotIterator* self) {
    tnt_iter_free(&(self->iter));
    tnt_stream_free(&(self->stream));
    PyObject_Del(self);
    
    Py_RETURN_NONE;
}

static PyMethodDef TarantoolSnapshot_Module_Methods[] = {
    {NULL}  /* Sentinel */
};

PyMODINIT_FUNC inittarantool_snapshot(void) {
    PyObject *m;

    SnapshotIterator_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&SnapshotIterator_Type) < 0)  return;

    m = Py_InitModule("tarantool_snapshot", TarantoolSnapshot_Module_Methods);
    if (!m) {
        return;
    }

    Py_INCREF(&SnapshotIterator_Type);
    PyModule_AddObject(m, "iter", (PyObject *)&SnapshotIterator_Type);

    SnapshotError = PyErr_NewException("tarantool_snapshot.SnapshotError", NULL, NULL);
    Py_INCREF(SnapshotError);
    PyModule_AddObject(m, "SnapshotError", SnapshotError); 
}

int main(int argc, char **argv) {
    Py_SetProgramName(argv[0]);
    Py_Initialize();
    inittarantool_snapshot();
    return 0;
}
