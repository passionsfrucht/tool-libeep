pymod = import('python')
python = pymod.find_installation(required: false, pure: false)

if python.found()
    python.extension_module('pyeep', 'pyeep.c',
                            include_directories: eep_h,
                            link_with: EepStatic,
                            dependencies: python.dependency(),
                            subdir: 'libeep',
                            install: true)
    python.install_sources(['__init__.py', 'oldeep.py', 'demo_read_cnt.py', 'demo_write_cnt.py'], subdir: 'libeep')
endif
