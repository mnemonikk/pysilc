from distutils.core import setup, Extension
from os.path import isfile

try:
    if isfile("MANIFEST"):
        os.unlink("MANIFEST")
except:
    pass

ext_modules = [
    Extension('silc', ['src/pysilc.c'],
              libraries = ['silc', 'silcclient'],
              extra_compile_args = ['-g'],
              library_dirs = ['/usr/local/lib'],
              include_dirs = ['/usr/local/include/silc',
                              '/usr/include/silc-toolkit'],
              depends = ['src/pysilc_callbacks.c',
                         'src/pysilc_channel.c',
                         'src/pysilc_user.c',
                         'src/pysilc_macros.h',
                         'src/pysilc.h']),
]

setup(name = 'pysilc',
      version = '0.4',
      description = 'Python Binding for SILC Toolkit',
      author = 'Alastair Tse',
      author_email = 'alastair@tse.id.au',
      url = 'http://www.liquidx.net/pysilc/',
      license = 'BSD',
      ext_package = '',
      ext_modules = ext_modules
)

    
