
## Preconditions

```nose``` Python testing framework is required.

Installation using ```apt-get``` package manager :
```
apt-get install python3-nose2
```

## Running tests

You may need to set up the PYTHON path:
```
export PYTHONPATH=${PROJECT_BINARY_DIR}/opencog/cython
```
or, if installed:
```
export PYTHONPATH=/usr/local/lib/python3.9/dist-packages/opencog:${PYTHON}
```

Some test functions are in `tests/cython/bindlink/test_functions.py`
Thus, you'll also need:
```
export PYTHONPATH=tests/cython/bindlink PROJECT_SOURCE_DIR=.
```

Then from atomspace root source dir execute:

```
nose2-3 -vs ./tests/cython/
nose2-3 -vs ./tests/cython/atomspace/
nose2-3 -vs ./tests/cython/bindlink/
nose2-3 -vs ./tests/cython/guile/
nose2-3 -vs ./tests/cython/utilities/
```
