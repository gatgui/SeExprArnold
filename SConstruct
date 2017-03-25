import sys
import glob
import excons
from excons.tools import arnold

arniver = arnold.Version(asString=False)
if arniver[0] < 4 or (arniver[0] == 4 and (arniver[1] < 2 or (arniver[1] == 2 and arniver[2] < 12))):
  print("SeExprArnold requires at least Arnold 4.2.12.2")
  sys.exit(1)

# SeExpr v3 requires c++11
excons.SetArgument("use-c++11", 1)

env = excons.MakeBaseEnv()

if sys.platform != "win32":
  env.Append(CPPFLAGS=" -Wno-unused-parameter")

#excons.Call("SeExpr", overrides={"static": 1})

#ARGUMENTS["static"] = "1"
SConscript("SeExpr/SConstruct")
Import("RequireSeExpr2")

prjs = [
  {"name"    : "seexpr",
   "prefix"  : "arnold",
   "type"    : "dynamicmodule",
   "ext"     : arnold.PluginExt(),
   "srcs"    : glob.glob("src/*.cpp"),
   #"incdirs" : ["SeExpr/src"],
   #"defs"    : ["SEEXPR_WIN32"] if sys.platform == "win32" else [],
   #"libs"    : ["SeExpr2"],
   "install" : {"arnold": ["src/seexpr.mtd"],
                "maya": ["maya/aiSeexprTemplate.py"]},
   "custom"  : [arnold.Require, RequireSeExpr2]
  }
]

excons.DeclareTargets(env, prjs)

Default("seexpr")
