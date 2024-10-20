project(
  'ewine',
  'c',
  version: '0.1',
  license: 'MIT',
)

conf_data = configuration_data()
conf_data.set_quoted('PROGRAM_NAME', meson.project_name())
conf_data.set_quoted('VERSION', meson.project_version())
configure_file(output: 'config.h', configuration: conf_data)

executable(meson.project_name(), 'main.c')
