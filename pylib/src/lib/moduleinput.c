/*
  m210
  Copyright © 2010 Tuomas Räsänen (tuos) <tuos@codegrove.org>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
