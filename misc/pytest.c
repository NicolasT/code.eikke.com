/* pytest.c
 * Copyright (C) 2005 Nicolas Trangez <eikke@eikke.com>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Compile using:
 *       gcc -o pytest -g `pkg-config --cflags --libs glib-2.0` -I/usr/include/python2.3 -lpython2.3 pytest.c
 *       
 * pytest.py:
        class pytest:
                def __init__(self):
                        print "Initializing a pytest object"

                def test(self):
                        print "In pytest's test function"

        def test(a):
                print "In main test function, argument is \"" + a + "\""
                return pytest()
*/

/* We want Python functionality */
#include <Python.h>
#include <glib.h>

/* To make life easier for us */
#define MODULE_NAME "pytest"

gint main(guint argc, gchar *argv[]) {
        /* The module object */
        PyObject *module = NULL;
        /* Objects we need to get a reference to a function */
        PyObject *dict = NULL, *func = NULL;
        /* Stuff we need to be able to load a module not in PYTHONPATH */
        PyObject *path = NULL, *pwd = NULL;
        /* Args we offer to the called function, and a reference to the return value */
        PyObject *args = NULL, *ret = NULL;

        /* Initialize the Python framework */
        g_debug("Initializing Python");
        Py_Initialize();

        /* "pytest.py" is in ".", so we need to alter the module search path
           "." is not in it by default */
        g_debug("Setting PATH");
        /* Get the current path (this is a list) */
        path = PySys_GetObject("path");
        /* Create a value to add to the list */
        pwd = PyString_FromString(".");
        /* And add it */
        PyList_Insert(path, 0, pwd);
        /* We don't need that string value anymore, so deref it */
        Py_DECREF(pwd);

        /* Load the module */
        g_debug("Trying to import \"%s\"", MODULE_NAME);
        module = PyImport_ImportModule("" MODULE_NAME);
        /* Check whether we succeeded */
        if(module == NULL) {
                /* If not, print the error message and get out of here */
                PyErr_Print();
                PyErr_Clear();
                g_warning("Failed to initialize \"%s\"", MODULE_NAME);
                return 1;
        }

        /* Get a dict from the module
           I should look up the API, but I presume this is something like
           "function_name" => function_entry_point */
        dict = PyModule_GetDict(module);
        /* Get the entry point of our "test" function
           This is -not- the pytest:test function */
        func = PyDict_GetItemString(dict, "test");

        /* Check again whether we succeeded, and whether the function can be called */
        if(func != NULL && PyCallable_Check(func) == TRUE) {
                g_debug("Success loading global test function");
        }
        else {
                /* Something bad occured, print out the Python error and abort */
                g_debug("Failed loading %s", MODULE_NAME);
                if(PyErr_Occurred()) {
                        PyErr_Print();
                        PyErr_Clear();
                }
                return 1;
        }

        /* We want to offer some args to the test(a) function
           These args should go into a tuple */
        /* Create a tuple with one element */
        args = PyTuple_New(1);
        /* Add a new element to the tuple, at position 0, a new string with content "testarg" */
        PyTuple_SetItem(args, 0, PyString_FromString("testarg"));

        /* Call the test function, with the "args" tuple as arguments
         * "test" returns a new pytest instance, so ret may never be NULL
         */
        ret = PyObject_CallObject(func, args);
        if(ret == NULL) {
                PyErr_Print();
                PyErr_Clear();
                g_warning("Failed to call test function");
                return 1;
        }

        /* Free the returned value, and the args tuple
           We don't really free, we unref the objects.
           I should look up what the difference between XDECREF and DECREF is. DECREF seems to be a standard unref thing */
        Py_DECREF(args);

        g_debug("Calling pytest::test using the helper function");
        /* Call the "test" method on object "ret" with NULL as args */
        PyObject_CallMethod(ret, "test", NULL);

        Py_XDECREF(ret);
        ret = NULL;
        
        g_debug("Calling pytest::test, no helper function");
        g_debug("Creating a new pytest object");
        /* Create a new "pytest" instance */
        ret = PyInstance_New(PyDict_GetItemString(dict, "" MODULE_NAME), NULL, NULL);
        g_assert(ret != NULL);
        PyObject_CallMethod(ret, "test", NULL);
        Py_XDECREF(ret);
        

        return 0;
}
