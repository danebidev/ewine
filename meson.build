project(
  'ewine',
  'c',
  version: '0.1',
  license: 'MIT',
)

cc = meson.get_compiler('c')

conf_data = configuration_data()
conf_data.set_quoted('PROGRAM_NAME', meson.project_name())
conf_data.set_quoted('VERSION', meson.project_version())

configure_file(output: 'build_config.h', configuration: conf_data)

if get_option('debug')
  extra_cflags = ['-g3', '-Wall', '-Wextra']
  add_project_arguments(cc.get_supported_arguments(extra_cflags), language: 'c')
endif

cjson = dependency('libcjson')

source_files = [
  'main.c',
  'config.h',
  'config.c',
  'data.h',
  'data.c',
  'util.h',
  'util.c',
  'list.h',
  'list.c',
  'run.h',
  'run.c',
  'create.h',
  'create.c',
  'components.h',
  'components.c',
  'remove.h',
  'remove.c',
]

executable(meson.project_name(), source_files, dependencies: [cjson])
