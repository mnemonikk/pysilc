from distutils.core import setup, Extension

ext_modules = [
    Extension('silc', ['src/pysilc.c'],
              libraries = ['silc', 'silcclient'],
              extra_compile_args = ['-g'],
              include_dirs = ['/usr/include/silc-toolkit'],
              depends = ['src/pysilc_callbacks.c',
                         'src/pysilc_channel.c',
                         'src/pysilc_user.c',
                         'src/pysilc_macros.h']),
]

setup(name = 'silc',
      version = '0.1',
      description = 'Python Binding for SILC Toolkit',
      author = 'Alastair Tse',
      author_email = 'alastair@tse.id.au',
      url = 'http://www.liquidx.net/pysilc/',
      license = 'BSD',
      ext_package = '',
      ext_modules = ext_modules
)

    
