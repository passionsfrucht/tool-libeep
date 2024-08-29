project('libeep', 'c', version: '3.3.179')

version = meson.project_version().split('.')
major = version[0]
minor = version[1]
patch = version[2]
eep_version = ['-DLIBEEP_VERSION_MAJOR=' + major, '-DLIBEEP_VERSION_MINOR=' + minor, '-DLIBEEP_VERSION_PATCH=' + patch]

eep_h = include_directories('src')
eep_c = [
    'src/libavr/avr.c',
    'src/libavr/avrcfg.c',
    'src/libcnt/cnt.c',
    'src/libcnt/cntutils.c',
    'src/libcnt/evt.c',
    'src/libcnt/raw3.c',
    'src/libcnt/rej.c',
    'src/libcnt/riff64.c',
    'src/libcnt/riff.c',
    'src/libcnt/seg.c',
    'src/libcnt/trg.c',
    'src/libeep/eepio.c',
    'src/libeep/eepmem.c',
    'src/libeep/eepmisc.c',
    'src/libeep/eepraw.c',
    'src/libeep/val.c',
    'src/libeep/var_string.c',
    'src/v4/eep.c'
]

if host_machine.system() == 'windows'
    flags = [ eep_version,
             '-D_CRT_SECURE_NO_WARNINGS',
             '-DNOMINMAX',
             '-DSTRICT',
             '-D_USE_MATH_DEFINES',
             '-DVC_EXTRALEAN',
             '-DWIN32_LEAN_AND_MEAN',
             '-DWIN32' #tested for in eepio.c but not defined when using ninja on windows
             ]
    eep_def = 'windows/Eep.def'
    m_dep = ''
else
    flags = [eep_version, '-Wall', '-Wextra', '-Werror=switch']
    eep_def = ''

    # linking with the math library
    cc = meson.get_compiler('c')
    m_dep = cc.find_library('m', required: true)
endif


# the meson docs actually recommend to create the library as
# eep = library('eep', eep_c, ...)
# instead of specifying static/shared and leave this decision to the client
EepStatic = static_library('EepStatic', eep_c,
                           include_directories: eep_h,
                           dependencies: m_dep,
                           c_args: flags,
                           install: true)

Eep = shared_library('Eep', eep_c,
                     include_directories: eep_h,
                     dependencies: m_dep,
                     c_args: flags,
                     vs_module_defs: eep_def,
                     install: true)

executable('demo_read_avr', 'test/demo_read_avr.c', include_directories: eep_h, link_with: EepStatic)
executable('demo_read_cnt', 'test/demo_read_cnt.c', include_directories: eep_h, link_with: EepStatic)
executable('demo_read_evt', 'test/demo_read_evt.c', include_directories: eep_h, link_with: EepStatic)
executable('demo_v4_read_cnt', 'test/demo_v4_read_cnt.c', include_directories: eep_h, link_with: EepStatic)
executable('demo_v4_write_cnt', 'test/demo_v4_write_cnt.c', include_directories: eep_h, link_with: EepStatic)
executable('demo_write_avr', 'test/demo_write_avr.c', include_directories: eep_h, link_with: EepStatic, dependencies: m_dep)
executable('demo_write_cnt', 'test/demo_write_cnt.c', include_directories: eep_h, link_with: EepStatic, dependencies: m_dep)
executable('libeep_info', 'test/libeep_info.c', include_directories: eep_h, link_with: EepStatic)

pkg = import('pkgconfig')
pkg.generate(Eep,
             name: 'libeep',
             url: 'https://gitlab.com/smeeze/libeep/',
             description: 'A.N.T. erp package',
             libraries: m_dep)

subdir('python')
