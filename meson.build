project(
  'ewine',
  'c',
  version: '0.1',
  license: 'MIT',
)

cjson = dependency('libcjson')

conf_data = configuration_data()
conf_data.set_quoted('PROGRAM_NAME', meson.project_name())
conf_data.set_quoted('VERSION', meson.project_version())

configure_file(output: 'build_config.h', configuration: conf_data)

source_files = ['main.c', 'config.h', 'config.c', 'data.h', 'data.c', 'util.h', 'util.c']

executable(meson.project_name(), source_files, dependencies: [cjson])
