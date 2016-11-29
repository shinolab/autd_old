from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = [Extension("pyautd3",
                     ["pyautd3.pyx"],
                     language='c++',
                     extra_objects=["libautd3.a"],
                     )]

setup(
    name = "pyautd3",
    cmdclass = {'build_ext': build_ext},
    ext_modules = ext_modules,
    author = 'Seki Inoue',
    description = 'autd3 wrapper for python',
    license = 'Apache',
    keywords = 'autd haptics mid-air ultrasound',
    url = 'http://gitlab.hapis.k.u-tokyo.ac.jp/inoue/autd3',
    zip_safe = False,
    )
