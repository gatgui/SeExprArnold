import sys
import glob
import excons
from excons.tools import arnold


env = excons.MakeBaseEnv()

arniver = arnold.Version(asString=False)
if arniver[0] < 4 or (arniver[0] == 4 and (arniver[1] < 2 or (arniver[1] == 2 and arniver[2] < 12))):
  print("SeExprArnold requires at least Arnold 4.2.12.2")
  sys.exit(1)

# SeExpr v3 requires c++11
excons.SetArgument("use-c++11", 1)

if sys.platform != "win32":
  env.Append(CPPFLAGS=" -Wno-unused-parameter")

excons.Call("SeExpr")

Import("RequireSeExpr2")

prjs = [
  {"name"    : "seexpr",
   "prefix"  : "arnold",
   "type"    : "dynamicmodule",
   "ext"     : arnold.PluginExt(),
   "srcs"    : glob.glob("src/*.cpp"),
   "install" : {"arnold": ["src/seexpr.mtd"],
                "maya": ["maya/aiSeexprTemplate.py"]},
   "custom"  : [arnold.Require, RequireSeExpr2]
  }
]

excons.DeclareTargets(env, prjs)

