project(
  'speedtime', 'c', license: 'CNPL-NAv7+', version: '2022.04.03',
  default_options: ['c_std=gnu11', 'buildtype=release', 'warning_level=3'])

add_global_arguments('-fomit-frame-pointer', '-march=native', language: 'c')

src = [
  'src/main.c',
  'src/keyboard.c',
  'src/splits.c',
  'src/timing.c',
  'src/utils.c', ]

executable('speedtime', src, dependencies: dependency('threads'))

man_vars = configuration_data()
man_vars.set('version', meson.project_version())
install_man(configure_file(
  input: 'extras/speedtime.1.in', output: 'speedtime.1', configuration: man_vars))
