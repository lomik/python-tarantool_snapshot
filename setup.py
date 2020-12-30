import os
import sys
import platform

if len(sys.argv) > 1 and sys.argv[1] == 'test':
    import setuptools

from distutils.core import setup, Extension

BUILD_STATIC = True

# for static build with tarantool sources
TARANTOOL_REV = "3536b7002e5ba88d3cdd0ebfbf668762a89dc72c" # release 1.4.8

def sh(command):
    import subprocess
    ret = subprocess.call(command,shell=True)
    if ret != 0:
        raise ValueError("command failed: %s" % command)
    return ret

def download_tarantool_src():
    import subprocess

    src_dir = os.path.join(os.path.dirname(__file__),'tarantool_src')
    if not os.path.exists(src_dir):
        sh("git clone https://github.com/tarantool/tarantool.git %s" % src_dir)
        sh("cd %s && git checkout %s" % (src_dir, TARANTOOL_REV))
        sh("rm -f %s/test/tarantool" % src_dir) # this broken file does not pass verification

    return src_dir


sources = ["tarantool_snapshot.c",]
include_dirs = []
library_dirs = []
extra_link_args = []

if BUILD_STATIC:
    tarantool_src_dir = download_tarantool_src()
    include_dirs += [
        tarantool_src_dir,
        os.path.join(tarantool_src_dir,"include"),
        os.path.join(tarantool_src_dir,"connector","c","include"),
    ]
    sources += [
        os.path.join(tarantool_src_dir,'connector','c','tntrpl','tnt_snapshot.c'),
        os.path.join(tarantool_src_dir,'connector','c','tnt','tnt_mem.c'),
        os.path.join(tarantool_src_dir,'connector','c','tntrpl','tnt_log.c'),
        os.path.join(tarantool_src_dir,'connector','c','tnt','tnt_tuple.c'),
        os.path.join(tarantool_src_dir,'connector','c','tnt','tnt_stream.c'),
        os.path.join(tarantool_src_dir,'connector','c','tnt','tnt_request.c'),
        os.path.join(tarantool_src_dir,'connector','c','tnt','tnt_iter.c'),
        os.path.join(tarantool_src_dir,'connector','c','tnt','tnt_enc.c'),
        os.path.join(tarantool_src_dir,'connector','c','tnt','tnt_reply.c'),
        os.path.join(tarantool_src_dir,'third_party','crc32.c'),
    ]
else:
    extra_link_args += ["-ltarantoolrpl",]

module1 = Extension('tarantool_snapshot',
                    include_dirs = include_dirs,
                    library_dirs = library_dirs,
                    sources = sources,
                    extra_link_args = extra_link_args)

if platform.python_implementation() == "PyPy":
    interpreter = "pypy"
else:
    interpreter = "python"

if sys.version_info.major == 3:
    interpreter += "3"

setup (name = '%s-tarantool-snapshot' % interpreter,
    description = 'Tarantool snapshot reader',
    version='1.0',
    author='Lomonosov Roman',
    author_email='r.lomonosov@gmail.com',
    url='https://github.com/lomik/python-tarantool_snaphot',
    packages=[],
    ext_modules = [module1])

