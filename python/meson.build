pymod = import('python')
python = pymod.find_installation(required: false)

if python.found()
    python.extension_module('pyeep', 'pyeep.c',
                            include_directories: eep_h,
                            link_with: EepStatic,
                            dependencies: python.dependency(),
                            install: true)
endif
