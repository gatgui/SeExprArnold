import sys
import glob
import excons
import excons.config
from excons.tools import arnold

env = excons.MakeBaseEnv()

arniver = arnold.Version(asString=False)
if arniver[0] < 4 or (arniver[0] == 4 and (arniver[1] < 2 or (arniver[1] == 2 and arniver[2] < 12))):
  print("SeExprArnold requires at least Arnold 4.2.12.0")
  sys.exit(1)

prefix = excons.GetArgument("prefix", "")
name = "%sseexpr" % prefix
spl = name.split("_")
maya_name = spl[0] + "".join(map(lambda x: x[0].upper() + x[1:], spl[1:]))
opts = {"PREFIX": prefix, "SEEXPR_MAYA_NODENAME": maya_name}

GenerateMtd = excons.config.AddGenerator(env, "mtd", opts)
GenerateMayaAE = excons.config.AddGenerator(env, "mayaAE", opts)
mtd = GenerateMtd("src/%s.mtd" % name, "src/seexpr.mtd.in")
ae = GenerateMayaAE("maya/%sTemplate.py" % maya_name, "maya/SeexprTemplate.py.in")


if sys.platform != "win32":
  env.Append(CPPFLAGS=" -Wno-unused-parameter")

excons.Call("SeExpr")

prjs = [
  {"name"    : "seexpr",
   "prefix"  : "arnold",
   "type"    : "dynamicmodule",
   "ext"     : arnold.PluginExt(),
   "srcs"    : glob.glob("src/*.cpp"),
   "libs"    : ["SeExpr"],
   "install" : {"arnold": mtd,
                "maya": ae},
   "custom"  : [arnold.Require]
  }
]

excons.DeclareTargets(env, prjs)

Default("seexpr")
