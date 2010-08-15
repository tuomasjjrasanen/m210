#include <Python.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>

static PyObject *hidraw_get_devinfo(PyObject *self, PyObject *args)
{
        int fd;
        struct hidraw_devinfo devinfo;

        memset(&devinfo, 0, sizeof(struct hidraw_devinfo));

        if (!PyArg_ParseTuple(args, "I", &fd))
                return NULL;
        if (ioctl(fd, HIDIOCGRAWINFO, &devinfo) == -1)
                return PyErr_SetFromErrno(PyExc_IOError);
        return Py_BuildValue("{sIshsh}",
                             "bustype", devinfo.bustype,
                             "vendor", devinfo.vendor,
                             "product", devinfo.product);
}

static PyMethodDef hidrawMethods[] = {
        {"get_devinfo", hidraw_get_devinfo, METH_VARARGS,
         "get_devinfo(filedescriptor)\n\n"
         "Return device information dictionary."
        },

        {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC inithidraw(void)
{
        PyObject *module;
        module = Py_InitModule3("hidraw", hidrawMethods,
                                "Python API to Linux hidraw devices.");
}
