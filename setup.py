#!/usr/bin/env python

from setuptools import Extension, setup
from pkg_resources import require

name         = 'polyadicts'
version      = '0.0.1'
description  = 'Addicted to data encapsulation'
author       = 'Jeremy R. Fishman'
author_email = 'jeremy.r.fishman@gmail.com'
url          = 'http://www.github.com/jfishman/polyadicts'
license      = 'GNU GPL v3'
test_suite   = 'pt.test.suite'

long_description = '''Polyadts: addicted to data encapsulation.

Polyadts is implemented in C and Python.  Any feedback, questions, or
comments, should go to jeremy.r.fishman@gmail.com.

Polyadts is Copyright (c) by Jeremy R. Fishman and is distributed
under the GNU General Public License (GPL) version 3 or at your option
any later version.  This software comes with absolutely no warranties,
either expressed or implied.'''

install_requires = [
    ]

trove_classifiers = [
    'Development Status :: 2 - Pre-Alpha'
    'Environment :: Console',
    'Environment :: Other Environment',
    'Intended Audience :: Developers',
    'License :: OSI Approved :: GNU General Public License (GPL)',
    'Natural Language :: English',
    'Operating System :: Microsoft :: Windows',
    'Operating System :: MacOS :: MacOS X',
    'Operating System :: POSIX :: Linux',
    'Programming Language :: Python :: 3.1',
    'Programming Language :: Python',
    'Programming Language :: C',
    'Topic :: Database',
    'Topic :: Internet',
    'Topic :: Software Development',
    'Topic :: System',
    'Topic :: System :: Archiving',
    'Topic :: System :: Filesystems',
    'Topic :: System :: Networking',
    'Topic :: System :: Software Distribution',
    ]

capi = Extension(
    'polyadicts',
    ['polyadictsmodule.c',
     'polyadicobjects.c',
     'polyadics.c',
     'varint.c',
     ],
)

setup(
    name=name,
    version=version,
    description=description,
    long_description=long_description,
    author=author,
    author_email=author_email,
    url=url,
    license=license,
    install_requires=install_requires,
    classifiers=trove_classifiers,
    test_suite=test_suite,
    ext_modules=[capi],
    include_package_data=True,
    )