import sys
import glob
import excons
from excons.tools import arnold

excons.SetArgument("static", 1)
excons.SetArgument("no-arch", 1)

env = excons.MakeBaseEnv()

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
