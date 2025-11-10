project(
    'u6502',
    'c',
    version: '0.1',
    default_options: ['c_std=gnu99', 'warning_level=2'],
    license: 'MIT',
)

fs = import('fs')

incdir = include_directories('include')

lib = library(
    'u6502',
    'src/u6502/u6502.c',
    install: true,
    include_directories: incdir,
)


executable(
    'example',
    'examples/lib1.c',
    install: false,
    link_with: lib,
    include_directories: incdir,
)

example_code = fs.read('examples/lib1.c')

man_u6502 = configure_file(
    input: 'man/u6502.3.in',
    output: 'u6502.3',
    configuration: {'example_code': example_code},
)

install_man(
    man_u6502,
    'man/U6502_delete.3',
    'man/U6502_disassemble.3',
    'man/U6502_dump.3',
    'man/U6502_getCallback.3',
    'man/U6502_getVector.3',
    'man/U6502_irq.3',
    'man/U6502_new.3',
    'man/U6502_nmi.3',
    'man/U6502_reset.3',
    'man/U6502_run.3',
    'man/U6502_setCallback.3',
    'man/U6502_setVector.3',
)
