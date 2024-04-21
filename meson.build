project('wayland-test', 'c',
          version: '0.1.0',
    meson_version: '>= 0.59.0',
  default_options: [ 'warning_level=2',
                     'c_std=gnu11',
                   ],
)

# Custom target to generate xdg-shell protocol files
wl_scanner = find_program('wayland-scanner')

xdg_shell_public = custom_target(
    'xdg_shell_public',
    output: 'xdg-shell.c',
    input: files(['xdg-shell.xml']),
    command: [wl_scanner, 'public-code', '@INPUT@', '@OUTPUT@'],
    build_by_default: true
)


xdg_shell_header = custom_target(
    'xdg_shell_header',
    output: 'xdg-shell.h',
    input: files(['xdg-shell.xml']),
    command: [wl_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
    build_by_default: true
)

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

# Specify dependencies
dependencies = [
  dependency('wayland-client'),
  dependency('wayland-protocols', version: '1.24', required: false),  # Adjust version as needed
  dependency('egl'),
  dependency('gl'),
  dependency('wayland-egl'),
  dependency('libportal'),
  dependency('epoxy'),
  m_dep
]


# Define the executable and link with dependencies
executable('wayland-test', ['xdg-shell.c', 'wayland-test.c'],
           dependencies: dependencies,
           install: true,
           install_dir: 'bin')  # Adjust install_dir as needed
