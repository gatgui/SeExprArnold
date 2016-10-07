import sys
import glob
import excons
from excons.tools import arnold

arniver = arnold.Version(asString=False)
if arniver[0] < 4 or (arniver[0] == 4 and (arniver[1] < 2 or (arniver[1] == 2 and arniver[2] < 12))):
  print("SeExprArnold requires at least Arnold 4.2.12.0")
  sys.exit(1)

excons.SetArgument("static", 1)
excons.SetArgument("no-arch", 1)

env = excons.MakeBaseEnv()

if sys.platform != "win32":
  env.Append(CPPFLAGS=" -Wno-unused-parameter")

SConscript("SeExpr/SConstruct")

prjs = [
  {"name"    : "seexpr",
   "prefix"  : "arnold",
   "type"    : "dynamicmodule",
   "ext"     : arnold.PluginExt(),
   "srcs"    : glob.glob("src/*.cpp"),
   "libs"    : ["SeExpr"],
   "install" : {"arnold": "src/seexpr.mtd",
                "maya": "maya/aiSeexprTemplate.py"},
   "custom"  : [arnold.Require]
  }
]

excons.DeclareTargets(env, prjs)

Default("seexpr")
