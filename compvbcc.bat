mkdir amiga_vbcc

rem set CFLAGS=-c99 -O2 -speed -cpu=68020 -DVIEWWINDOWWIDTH=30 -DVIEWWINDOWHEIGHT=64  -DHORIZONTAL_RESOLUTION=HORIZONTAL_RESOLUTION_LO -DVERTICAL_RESOLUTION_DOUBLED -lamiga -lauto
set CFLAGS=-c99 -O4 -speed -cpu=68020  -dontwarn=51 -DVIEWWINDOWWIDTH=30  -DHORIZONTAL_RESOLUTION=HORIZONTAL_RESOLUTION_LO -DVIEWWINDOWHEIGHT=64 -lamiga -lauto

rem -dontwarn 51 disables "non-portable bitfield" warnings

@set GLOBOBJS=
@set GLOBOBJS=%GLOBOBJS% d_items.c
@set GLOBOBJS=%GLOBOBJS% d_main.c
@set GLOBOBJS=%GLOBOBJS% g_game.c
rem @set GLOBOBJS=%GLOBOBJS% i_amiga.c
@set GLOBOBJS=%GLOBOBJS% i_amiga_chunky_8_by_y.c
@set GLOBOBJS=%GLOBOBJS% info.c
@set GLOBOBJS=%GLOBOBJS% m_random.c
@set GLOBOBJS=%GLOBOBJS% p_doors.c
@set GLOBOBJS=%GLOBOBJS% p_enemy.c
@set GLOBOBJS=%GLOBOBJS% p_floor.c
@set GLOBOBJS=%GLOBOBJS% p_inter.c
@set GLOBOBJS=%GLOBOBJS% p_lights.c
@set GLOBOBJS=%GLOBOBJS% p_map.c
@set GLOBOBJS=%GLOBOBJS% p_maputl.c
@set GLOBOBJS=%GLOBOBJS% p_mobj.c
@set GLOBOBJS=%GLOBOBJS% p_plats.c
@set GLOBOBJS=%GLOBOBJS% p_pspr.c
@set GLOBOBJS=%GLOBOBJS% p_scroll.c
@set GLOBOBJS=%GLOBOBJS% p_setup.c
@set GLOBOBJS=%GLOBOBJS% p_sight.c
@set GLOBOBJS=%GLOBOBJS% p_spec.c
@set GLOBOBJS=%GLOBOBJS% p_switch.c
@set GLOBOBJS=%GLOBOBJS% p_tick.c
@set GLOBOBJS=%GLOBOBJS% p_user.c
@set GLOBOBJS=%GLOBOBJS% r_data.c
@set GLOBOBJS=%GLOBOBJS% r_draw.c
@set GLOBOBJS=%GLOBOBJS% r_plane.c
@set GLOBOBJS=%GLOBOBJS% r_things.c
@set GLOBOBJS=%GLOBOBJS% st_stuff.c
@set GLOBOBJS=%GLOBOBJS% tables.c
@set GLOBOBJS=%GLOBOBJS% v_video.c
@set GLOBOBJS=%GLOBOBJS% w_wad.c
@set GLOBOBJS=%GLOBOBJS% z_bmallo.c
@set GLOBOBJS=%GLOBOBJS% z_zone.c

vc +aos68k %GLOBOBJS% %CFLAGS% -o amiga_vbcc\DOOMTD32.EXE

