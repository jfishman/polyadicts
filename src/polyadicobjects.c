
/*
** This file is part of polyadicts - addicted to data encapsulation.
**
** Polyadicts is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Polyadicts is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** and the GNU Lesser Public License along with polyadicts.  If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "polyadicobjects.h"

/**
 * PyPolyad
 */

void
PyPolyad_dealloc(PyPolyad* self)
{
    if (self->pack)
        polyad_free(self->pack);
    if (self->src) {
        PyBuffer_Release(self->src);
        PyMem_Del(self->src);
    }
    self->ob_base.ob_type->tp_free((PyObject*)self);
}

PyObject *
PyPolyad_FromBuffer(Py_buffer *view, size_t off, size_t len)
{
    if (len == 0)
        len = view->len;
    if (len > view->len || off > len) {
        errno = EINVAL;
        PyErr_SetFromErrno(PyExc_ValueError);
        return NULL;
    }

    /* allocate new pack object */
    PyPolyad *self;
    self = (PyPolyad*) PyPolyad_Type.tp_alloc(&PyPolyad_Type, 0);
    if (!self)
        return NULL;

    /* allocate Py_buffer to refcount shared memory region */
    self->src = PyMem_Malloc(sizeof(Py_buffer));
    if (!self->src) {
        PyPolyad_Type.tp_free(self);
        return NULL;
    }
    *self->src = *view;

    /* load and initialize pack pointers from data buffer */
    self->pack = polyad_load(len, view->buf + off, true);
    if (!self->pack) {
        if (errno == ENOMEM)
            PyErr_SetFromErrno(PyExc_MemoryError);
        else
            PyErr_SetFromErrno(PyExc_ValueError);
        PyMem_Free(self->src);
        PyPolyad_Type.tp_free(self);
        return NULL;
    }
    /* prevent access to the owned buffer reference */
    memset(view, 0, sizeof(Py_buffer));
    return (PyObject*) self;
}

PyObject *
PyPolyad_FromSequence(PyObject *src, const char *errmsg)
{
    if (NULL == (src = PySequence_Fast(src, errmsg)))
        return NULL;

    Py_ssize_t len = PySequence_Fast_GET_SIZE(src);
    struct polyad *pack;
    pack = polyad_prepare(len);
    if (!pack) {
        PyErr_SetFromErrno(PyExc_MemoryError);
        return NULL;
    }

    Py_ssize_t i;
    Py_buffer view[len];
    for (i = 0; i < len; i++) {
        PyObject *obj = PySequence_Fast_GET_ITEM(src, i);
        if (0 != PyObject_GetBuffer(obj, &view[i], PyBUF_SIMPLE))
            break;
        polyad_set(pack, i, view[i].len, view[i].buf, true);
    }

    if (i != len || 0 != polyad_finish(pack)) {
        if (!PyErr_Occurred())
            PyErr_SetFromErrno(PyExc_MemoryError);
        polyad_free(pack);
        pack = NULL;
    }

    /* release all open buffers */
    len = i;
    for (i = 0; i < len; i++)
        PyBuffer_Release(&view[i]);
    if (!pack)
        return NULL;

    /* allocate new PyPolyad object */
    PyPolyad *self;
    self = (PyPolyad*) PyPolyad_Type.tp_alloc(&PyPolyad_Type, 0);
    if (self) {
        self->pack = pack;
        self->src = NULL;
    }
    return (PyObject*) self;
}

PyObject *
PyPolyad_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *src;
    if (!PyArg_ParseTuple(args, "O", &src))
        return NULL;

    Py_buffer view;
    if (PyObject_CheckBuffer(src) &&
            0 == PyObject_GetBuffer(src, &view, PyBUF_SIMPLE)) {
        PyObject *pack = PyPolyad_FromBuffer(&view, 0, 0);
        if (!pack)
            PyBuffer_Release(&view);
        return pack;
    }

    return PyPolyad_FromSequence(src,
            "expected a sequence (encode) or bufferable (decode)");
}

/* PyPolyad buffer API */
int
PyPolyad_getbuffer(PyPolyad *self, Py_buffer *view, int flags)
{
    const struct iovec data = polyad_data(self->pack);
    return PyBuffer_FillInfo(view, (PyObject*)self, data.iov_base,
            data.iov_len, true, PyBUF_SIMPLE);
}

PyBufferProcs PyPolyad_as_buffer = {
    (getbufferproc)PyPolyad_getbuffer,
    NULL,
};

/* PyPolyad sequence API */
Py_ssize_t
PyPolyad_length(PyObject *self)
{
    return polyad_rank(((PyPolyad*)self)->pack);
}

PyObject*
PyPolyad_item(PyObject *obj_self, Py_ssize_t i)
{
    PyPolyad *self = (PyPolyad*) obj_self;
    if (i >= polyad_rank(self->pack)) {
        PyErr_SetString(PyExc_IndexError, "pack index out of range");
        return NULL;
    }

    Py_buffer view;
    if (0 == PyObject_GetBuffer(obj_self, &view, PyBUF_SIMPLE)) {
        const struct iovec data = polyad_item(self->pack, i);
        view.buf = data.iov_base;
        view.len = data.iov_len;
        return PyMemoryView_FromBuffer(&view);
    }
    return NULL;
}

PySequenceMethods PyPolyad_as_sequence = {
    (lenfunc)PyPolyad_length, /*sq_length*/
    NULL,                       /*sq_concat*/
    NULL,                       /*sq_repeat*/
    (ssizeargfunc)PyPolyad_item,  /*sq_item*/
    NULL,                       /*sq_ass_item*/
    NULL,                       /*sq_contains*/
    NULL,                       /*sq_inplace_concat*/
    NULL,                       /*sq_inplace_repeat*/
};

/* PyPolyad type definition */
PyTypeObject PyPolyad_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "polyadicts.polyad",             /*tp_name*/
    sizeof(PyPolyad),         /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor)PyPolyad_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    &PyPolyad_as_sequence,    /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    0,                          /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    &PyPolyad_as_buffer,      /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,         /*tp_flags*/
    "polyad(bufferable | sequence)", /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    0,                          /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    PyPolyad_tp_new,          /* tp_new */
};
