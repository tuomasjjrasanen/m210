#include <Python.h>
#include <linux/input.h>
#include <sys/ioctl.h>

static PyObject *input_grab_event_device(PyObject *self, PyObject *args)
{
        int fd;

        if (!PyArg_ParseTuple(args, "I", &fd))
                return NULL;
        if (ioctl(fd, EVIOCGRAB, 1) == -1)
                return PyErr_SetFromErrno(PyExc_IOError);
        return Py_None;
}

static PyObject *input_release_event_device(PyObject *self, PyObject *args)
{
        int fd;

        if (!PyArg_ParseTuple(args, "I", &fd))
                return NULL;
        if (ioctl(fd, EVIOCGRAB, 0) == -1)
                return PyErr_SetFromErrno(PyExc_IOError);
        return Py_None;
}

static PyMethodDef inputMethods[] = {
        {"grab_event_device", input_grab_event_device, METH_VARARGS,
         "grab_event_device(filedescriptor)\n\n"
         "Grab event device."
        },

        {"release_event_device", input_release_event_device, METH_VARARGS,
         "release_event_device(filedescriptor)\n\n"
         "Release event device."
        },

        {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initinput(void)
{
        PyObject *module;
        module = Py_InitModule3("input", inputMethods,
                                "Python API to Linux input devices.");
}
