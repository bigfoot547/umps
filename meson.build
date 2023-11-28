project('umps', 'c')

git_program = find_program('git', required : false)
if git_program.found()
  res = run_command(['git', 'describe', '--dirty=-dirty', '--always'], check : true, capture : true)

  # trim version string down so it won't ever be too large for the UI
  prog_version = res.stdout().strip().substring(0, 48)

  message('Found version string', prog_version)
else
  prog_version = '???'
  message('Git not found. Falling back to version', prog_version)
endif

conf_data = configuration_data()

opt_ncurses_narrow = get_option('ncurses_narrow')

if opt_ncurses_narrow
  curses_dep = dependency('ncurses')
  ncurses_is_wide = false
else
  curses_dep = dependency('ncursesw', required : false)
  if not curses_dep.found()
    curses_dep = dependency('ncurses')
    ncurses_is_wide = false
  else
    ncurses_is_wide = true
  endif
endif

if ncurses_is_wide
  conf_data.set('NCURSES_WIDE', true)
  conf_data.set('NCURSES_INCLUDE', '<ncursesw/ncurses.h>')
else
  conf_data.set('NCURSES_WIDE', false)
  conf_data.set('NCURSES_INCLUDE', '<ncurses.h>')
endif

conf_data.set_quoted('UMPS_VERSION', prog_version)
conf_data.set_quoted('UMPS_NAME', 'UMPS')

subdir('src')
executable('umps', umps_srcs, dependencies : [curses_dep], include_directories : umps_config_inc,
  override_options : {'b_ndebug' : 'if-release', 'c_std' : 'c99' })
