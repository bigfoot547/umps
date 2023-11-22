project('umps', 'c')

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

subdir('src')
executable('umps', umps_srcs, dependencies : [curses_dep], include_directories : umps_config_inc,
  override_options : {'b_ndebug' : 'if-release'})
