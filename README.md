# SeExprArnold
SeExpr shader for Arnold renderer

1/ How to build

   scons with-arnold=/path/to/arnold [debug=1]

2/ How to install

   The arnold plugin will be outputed in release/arnold (or debug/arnold)
   Just add this directory to your ARNOLD_PLUGIN_PATH

3/ How to use
   
   Shader specific variables can be access using '$varname'
   Shader globals can be accessed using '$sg::varname'
   Primitive variables can be accessed using '$usrr::varname'

   List of supported shader globals:
      P
      Po
      N
      Nf
      Ng
      Ngf
      Ns
      Ro
      Rd
      dPdx
      dPdy
      dPdu
      dPdv
      dNdx (starting arnold 4.1)
      dNdy (starting arnold 4.1)
      dDdx
      dDdy
      Ld
      Li
      Liu
      Lo
      Ci
      Vo
      u
      v
      bu
      bv
      x
      y
      sx
      sy
      we
      Rl
      dudx
      dudy
      dvdx
      dvdy
      time
      area
      Ldist
      sc
      Rt
      Rr
      Rr_refl
      Rr_refr
      Rr_diff
      Rr_gloss
   
      // The following extras require 'frame', 'fps', motion_start_frame' and 'motion_end_frame'
      // to be defined on option node:
   
      fps
      frame                : render frame
      sample_frame         : 'time' in frame
      shutter_open_time    : render camera 'shutter_start'
      shutter_close_time   : render camera 'shutter_end'
      shutter_open_frame   : 'shutter_open_time' in frame
      shutter_close_frame  : 'shutter_close_time' in frame


