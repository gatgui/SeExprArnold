// Copyright 2014 Gaetan Guidet
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <ai.h>
#include <SeExpr2/Expression.h>
#include <SeExpr2/VarBlock.h>
#include <cstring>
#include <cstdio>
#include <map>
#include <vector>
#include <string>

AI_SHADER_NODE_EXPORT_METHODS(SeExprMtd);

enum SeExpParams
{
   p_expr = 0,
   p_fparam_name,
   p_fparam_value,
   p_vparam_name,
   p_vparam_value,
   p_stop_on_error,
   p_error_value
};

struct SeExprData
{
   class ArnoldExpr** exprs; // expression objects array (up to one per thread)
   bool valid;         // whether or not the expression is valid
   bool constant;      // whether or not the expression is constant (use value member)
   bool threadsafe;    // whether or not the expression is thread safe
   bool sgdependent;   // whether or not the expression depends on shader globals
   AtCritSec mutex;    // mutex for thread unsafe shader globals dependent expressions
   AtVector value;
   unsigned int numfvars;
   unsigned int numvvars;
   std::map<std::string, unsigned int> varindex;
   bool stopOnError;

   int nthreads;
   SeExpr2::VarBlockCreator* varBlockCreator;
   int outputIndex;
   double *outputData;
   SeExpr2::VarBlock** varBlocks;
   std::string source;
};

namespace SSTR
{
   extern AtString expression;
   extern AtString fparam_name;
   extern AtString fparam_value;
   extern AtString vparam_name;
   extern AtString vparam_value;
   extern AtString stop_on_error;
   extern AtString linkable;
   extern AtString fps;
   extern AtString motion_start_frame;
   extern AtString motion_end_frame;
   extern AtString frame;
   extern AtString relative_motion_frame;
   extern AtString shutter_start;
   extern AtString shutter_end;
}

// ---

class ArnoldSgVar : public SeExpr2::ExprVarRef
{
public:
   enum
   {
      P = 0,
      Po,
      N,
      Nf,
      Ng,
      Ngf,
      Ns,
      Ro,
      Rd,
      dPdx,
      dPdy,
      dPdu,
      dPdv,
      dNdx,
      dNdy,
      dDdx,
      dDdy,
      Ld,
      Li,
      Liu,
      Lo,
      Ci,
      Vo,
      u,
      v,
      bu,
      bv,
      x,
      y,
      sx,
      sy,
      we,
      Rl,
      dudx,
      dudy,
      dvdx,
      dvdy,
      time, // between shutter open and close (in [0,1])
      area,
      frame,
      sample_frame,
      fps,
      shutter_open_time,
      shutter_close_time,
      shutter_open_frame,
      shutter_close_frame,
      Ldist,
      sc,
      Rt,
      Rr,
      Rr_refl,
      Rr_refr,
      Rr_diff,
      Rr_gloss,
      undefined,
      // some extra values to identify float/vector
      vecmin = P,
      vecmax = Vo,
      fltmin = u,
      fltmax = Rr_gloss
   };

   static int NameToEnum(const std::string &name)
   {
      static std::map<std::string, int> sNameToEnum;

      if (sNameToEnum.size() == 0)
      {
         sNameToEnum["P"] = P;
         sNameToEnum["Po"] = Po;
         sNameToEnum["N"] = N;
         sNameToEnum["Nf"] = Nf;
         sNameToEnum["Ng"] = Ng;
         sNameToEnum["Ngf"] = Ngf;
         sNameToEnum["Ns"] = Ns;
         sNameToEnum["Ro"] = Ro;
         sNameToEnum["Rd"] = Rd;
         sNameToEnum["dPdx"] = dPdx;
         sNameToEnum["dPdy"] = dPdy;
         sNameToEnum["dPdu"] = dPdu;
         sNameToEnum["dPdv"] = dPdv;
         sNameToEnum["dNdx"] = dNdx;
         sNameToEnum["dNdy"] = dNdy;
         sNameToEnum["dDdx"] = dDdx;
         sNameToEnum["dDdy"] = dDdy;
         sNameToEnum["Ld"] = Ld;
         sNameToEnum["Li"] = Li;
         sNameToEnum["Liu"] = Liu;
         sNameToEnum["Lo"] = Lo;
         sNameToEnum["Ci"] = Ci;
         sNameToEnum["Vo"] = Vo;
         sNameToEnum["u"] = u;
         sNameToEnum["v"] = v;
         sNameToEnum["bu"] = bu;
         sNameToEnum["bv"] = bv;
         sNameToEnum["x"] = x;
         sNameToEnum["y"] = y;
         sNameToEnum["sx"] = sx;
         sNameToEnum["sy"] = sy;
         sNameToEnum["we"] = we;
         sNameToEnum["Rl"] = Rl;
         sNameToEnum["dudx"] = dudx;
         sNameToEnum["dudy"] = dudy;
         sNameToEnum["dvdx"] = dvdx;
         sNameToEnum["dvdy"] = dvdy;
         sNameToEnum["time"] = time;
         sNameToEnum["area"] = area;
         sNameToEnum["frame"] = frame;
         sNameToEnum["sample_frame"] = sample_frame;
         sNameToEnum["fps"] = fps;
         sNameToEnum["shutter_open_time"] = shutter_open_time;
         sNameToEnum["shutter_close_time"] = shutter_close_time;
         sNameToEnum["shutter_open_frame"] = shutter_open_frame;
         sNameToEnum["shutter_close_frame"] = shutter_close_frame;
         sNameToEnum["Ldist"] = Ldist;
         sNameToEnum["sc"] = sc;
         sNameToEnum["Rt"] = Rt;
         sNameToEnum["Rr"] = Rr;
         sNameToEnum["Rr_refl"] = Rr_refl;
         sNameToEnum["Rr_refr"] = Rr_refr;
         sNameToEnum["Rr_diff"] = Rr_diff;
         sNameToEnum["Rr_gloss"] = Rr_gloss;
      }

      std::map<std::string, int>::iterator it = sNameToEnum.find(name);
      if (it != sNameToEnum.end())
      {
         return it->second;
      }
      else
      {
         return undefined;
      }
   }

   static const char* EnumToName(int which)
   {
      static const char* sEnumToName[] =
      {
         "P",
         "Po",
         "N",
         "Nf",
         "Ng",
         "Ngf",
         "Ns",
         "Ro",
         "Rd",
         "dPdx",
         "dPdy",
         "dPdu",
         "dPdv",
         "dNdx",
         "dNdy",
         "dDdx",
         "dDdy",
         "Ld",
         "Li",
         "Liu",
         "Lo",
         "Ci",
         "Vo",
         "u",
         "v",
         "bu",
         "bv",
         "x",
         "y",
         "sx",
         "sy",
         "we",
         "Rl",
         "dudx",
         "dudy",
         "dvdx",
         "dvdy",
         "time",
         "area",
         "frame",
         "sample_frame",
         "fps",
         "shutter_open_time",
         "shutter_close_time",
         "shutter_open_frame",
         "shutter_close_frame",
         "Ldist",
         "sc",
         "Rt",
         "Rr",
         "Rr_refl",
         "Rr_refr",
         "Rr_diff",
         "Rr_gloss",
         "undefined"
      };

      if (which < 0 || which >= undefined)
      {
         return sEnumToName[undefined];
      }
      else
      {
         return sEnumToName[which];
      }
   }

   ArnoldSgVar(int which)
      : SeExpr2::ExprVarRef(SeExpr2::ExprType().Error().Varying())
      , mWhich(which)
      , mIsVec(false)
      , mSgVarF(0)
      , mSgVarV(0)
      , mSgVarI(0)
      , mSgVarD(0)
      , mFrame(0.0f)
      , mFPS(24.0f)
      , mMotionStart(0.0f)
      , mMotionEnd(0.0f)
      , mSampleFrame(0.0f)
      , mTime(0)
      , mShutterOpenTime(0.0f)
      , mShutterCloseTime(0.0f)
      , mShutterOpenFrame(0.0f)
      , mShutterCloseFrame(0.0f)
   {
      if (mWhich < 0 || mWhich >= undefined)
      {
         mWhich = undefined;
      }
      else
      {
         mIsVec = (mWhich <= vecmax);
         setType(SeExpr2::ExprType().FP(mIsVec ? 3 : 1).Varying());
      }
      initOptionsVar();
      initCameraVar();
   }

   ArnoldSgVar(const std::string &name)
      : SeExpr2::ExprVarRef(SeExpr2::ExprType().Error().Varying())
      , mWhich(undefined)
      , mIsVec(false)
      , mSgVarF(0)
      , mSgVarV(0)
      , mSgVarI(0)
      , mSgVarD(0)
      , mSgVarB(0)
      , mSgVarC(0)
      , mFrame(0.0f)
      , mFPS(24.0f)
      , mMotionStart(0.0f)
      , mMotionEnd(0.0f)
      , mSampleFrame(0.0f)
      , mTime(0)
      , mShutterOpenTime(0.0f)
      , mShutterCloseTime(0.0f)
      , mShutterOpenFrame(0.0f)
      , mShutterCloseFrame(0.0f)
   {
      mWhich = NameToEnum(name);
      if (mWhich != undefined)
      {
         mIsVec = (mWhich <= vecmax);
         setType(SeExpr2::ExprType().FP(mIsVec ? 3 : 1).Varying());
      }
      initOptionsVar();
      initCameraVar();
   }

   virtual ~ArnoldSgVar()
   {
   }

   virtual void eval(const char **)
   {
      assert(false);
   }

   virtual void eval(double *result)
   {
      if (mWhich == sample_frame)
      {
         if (mTime)
         {
            mSampleFrame = mMotionStart + *mTime * (mMotionEnd - mMotionStart);
         }
         else
         {
            mSampleFrame = mMotionStart;
         }
      }
      
      if (mSgVarV)
      {
         result[0] = mSgVarV->x;
         result[1] = mSgVarV->y;
         result[2] = mSgVarV->z;
      }
      else if (mSgVarC)
      {
         result[0] = mSgVarC->r;
         result[1] = mSgVarC->g;
         result[2] = mSgVarC->b;
      }
      else if (mSgVarF)
      {
         result[0] = double(*mSgVarF);
      }
      else if (mSgVarD)
      {
         result[0] = *mSgVarD;
      }
      else if (mSgVarI)
      {
         result[0] = double(*mSgVarI);
      }
      else if (mSgVarU16)
      {
         result[0] = double(*mSgVarU16);
      }
      else if (mSgVarB)
      {
         result[0] = double(*mSgVarB);
      }
      else
      {
         result[0] = 0.0;
         if (mIsVec)
         {
            result[1] = 0.0;
            result[2] = 0.0;
         }
      }
   }
   
   bool getNodeConstantFloat(AtNode *node, AtString name, float &val, const char *msg=NULL) const
   {
      const AtUserParamEntry *pe = AiNodeLookUpUserParameter(node, name);
      if (pe != 0)
      {
         int type = AiUserParamGetType(pe);
         int cat = AiUserParamGetCategory(pe);
         
         if (cat == AI_USERDEF_CONSTANT)
         {
            switch (type)
            {
            case AI_TYPE_BYTE:
               val = float(AiNodeGetByte(node, name));
               break;
            case AI_TYPE_INT:
               val = float(AiNodeGetInt(node, name));
               break;
            case AI_TYPE_UINT:
               val = float(AiNodeGetUInt(node, name));
               break;
            case AI_TYPE_FLOAT:
               val = AiNodeGetFlt(node, name);
               break;
            default:
               AiMsgWarning("[seexpr] \"%s\" parameter on node \"%s\" should be a float or an integer (%s)", name.c_str(), AiNodeGetName(node), (msg ? msg : ""));
               return false;
            }
            return true;
         }
         else
         {
            AiMsgWarning("[seexpr] \"%s\" parameter on node \"%s\" must be a constant (%s)", name.c_str(), AiNodeGetName(node), (msg ? msg : ""));
            return false;
         }
      }
      else
      {
         AiMsgWarning("[seexpr] \"%s\" parameter not defined on node \"%s\" (%s)", name.c_str(), AiNodeGetName(node), (msg ? msg : ""));
         return false;
      }
   }
   
   bool getNodeConstantBool(AtNode *node, AtString name, bool &val, const char *msg=NULL) const
   {
      const AtUserParamEntry *pe = AiNodeLookUpUserParameter(node, name);
      if (pe != 0)
      {
         int type = AiUserParamGetType(pe);
         int cat = AiUserParamGetCategory(pe);
         
         if (cat == AI_USERDEF_CONSTANT)
         {
            switch (type)
            {
            case AI_TYPE_BOOLEAN:
               val = AiNodeGetBool(node, name);
               break;
            case AI_TYPE_BYTE:
               val = (AiNodeGetByte(node, name) != 0);
               break;
            case AI_TYPE_INT:
               val = (AiNodeGetInt(node, name) != 0);
               break;
            case AI_TYPE_UINT:
               val = (AiNodeGetUInt(node, name) != 0);
               break;
            case AI_TYPE_FLOAT:
               val = (AiNodeGetFlt(node, name) != 0.0f);
               break;
            default:
               AiMsgWarning("[seexpr] \"%s\" parameter on node \"%s\" should be a boolean, an integer or a float (%s)", name.c_str(), AiNodeGetName(node), (msg ? msg : ""));
               return false;
            }
            return true;
         }
         else
         {
            AiMsgWarning("[seexpr] \"%s\" parameter on node \"%s\" must be a constant (%s)", name.c_str(), AiNodeGetName(node), (msg ? msg : ""));
            return false;
         }
      }
      else
      {
         AiMsgWarning("[seexpr] \"%s\" parameter not defined on node \"%s\" (%s)", name.c_str(), AiNodeGetName(node), (msg ? msg : ""));
         return false;
      }
   }
   
   void initOptionsVar()
   {
      switch (mWhich)
      {
      case fps:
         getNodeConstantFloat(AiUniverseGetOptions(), SSTR::fps, mFPS, "Defaults to 24");
         break;
      case frame:
         getNodeConstantFloat(AiUniverseGetOptions(), SSTR::frame, mFrame, "Defaults to 0");
         break;
      case sample_frame:
      case shutter_open_frame:
      case shutter_close_frame:
         {
            bool relative = false;
            float frame = 0.0f;
            AtNode *opts = AiUniverseGetOptions();
            if (getNodeConstantBool(opts, SSTR::relative_motion_frame, relative, "Defaults to false") && relative)
            {
               getNodeConstantFloat(opts, SSTR::frame, frame, "Defaults to 0");
            }
            if (!getNodeConstantFloat(opts, SSTR::motion_start_frame, mMotionStart, "Defaults to 'frame'"))
            {
               if (relative)
               {
                  // already have value in frame variable
                  mMotionStart = frame;
               }
               else
               {
                  getNodeConstantFloat(opts, SSTR::frame, mMotionStart, "Defaults to 0");
               }
            }
            else
            {
               mMotionStart += frame;
            }
            if (!getNodeConstantFloat(opts, SSTR::motion_end_frame, mMotionEnd, "Defaults to 'motion_start_frame'"))
            {
               mMotionEnd = mMotionStart;
            }
            else
            {
               mMotionEnd += frame;
            }
         }
         break;
      default:
         break;
      }
   }
   
   void initCameraVar()
   {
      // Note: must init options var first
      switch (mWhich)
      {
      case shutter_open_time:
      case shutter_open_frame:
      case shutter_close_time:
      case shutter_close_frame:
         {
            mShutterOpenFrame = mMotionStart;
            mShutterCloseFrame = mShutterOpenFrame;
            
            AtNode *cam = AiUniverseGetCamera();
            
            if (cam)
            {
               mShutterOpenTime = AiNodeGetFlt(cam, SSTR::shutter_start);
               mShutterCloseTime = AiNodeGetFlt(cam, SSTR::shutter_end);
               
               float motionLength = mMotionEnd - mMotionStart;
               
               if (motionLength > 0.0f)
               {
                  mShutterOpenFrame = mMotionStart + mShutterOpenTime * motionLength;
                  mShutterCloseFrame = mMotionStart + mShutterCloseTime * motionLength;
               }
            }
            else
            {
               mShutterOpenTime = 0.0f;
               mShutterCloseTime = mShutterOpenTime;
            }
         }
         break;
      default:
         break;
      }
   }
   
   bool bind(AtNode *, AtShaderGlobals *sg)
   {
      mSgVarF = 0;
      mSgVarV = 0;
      mSgVarI = 0;
      mSgVarD = 0;
      mSgVarB = 0;
      mSgVarC = 0;
      mSgVarU16 = 0;

      if (mWhich != undefined)
      {
         switch (mWhich)
         {
         case P:
            mSgVarV = &(sg->P); break;
         case Po:
            mSgVarV = &(sg->Po); break;
         case N:
            mSgVarV = &(sg->N); break;
         case Nf:
            mSgVarV = &(sg->Nf); break;
         case Ng:
            mSgVarV = &(sg->Ng); break;
         case Ngf:
            mSgVarV = &(sg->Ngf); break;
         case Ns:
            mSgVarV = &(sg->Nf); break;
         case Ro:
            mSgVarV = &(sg->Ro); break;
         case Rd:
            mSgVarV = &(sg->Rd); break;
         case dPdx:
            mSgVarV = &(sg->dPdx); break;
         case dPdy:
            mSgVarV = &(sg->dPdy); break;
         case dPdu:
            mSgVarV = &(sg->dPdu); break;
         case dPdv:
            mSgVarV = &(sg->dPdv); break;
         case dNdx:
            mSgVarV = &(sg->dNdx); break;
         case dNdy:
            mSgVarV = &(sg->dNdy); break;
         case dDdx:
            mSgVarV = &(sg->dDdx); break;
         case dDdy:
            mSgVarV = &(sg->dDdy); break;
         case Ld:
            mSgVarV = &(sg->Ld); break;
         case Li:
            mSgVarC = &(sg->Li); break;
         case Liu:
            mSgVarC = &(sg->Liu); break;
         case Lo:
            mSgVarC = &(sg->Lo); break;
         case Ci:
            mSgVarC = &(sg->Ci); break;
         case Vo:
            mSgVarC = &(sg->Vo); break;
         case u:
            mSgVarF = &(sg->u); break;
         case v:
            mSgVarF = &(sg->v); break;
         case bu:
            mSgVarF = &(sg->bu); break;
         case bv:
            mSgVarF = &(sg->bv); break;
         case x:
            mSgVarI = &(sg->x); break;
         case y:
            mSgVarI = &(sg->y); break;
         case sx:
            mSgVarF = &(sg->sx); break;
         case sy:
            mSgVarF = &(sg->sy); break;
         case we:
            mSgVarF = &(sg->we); break;
         case Rl:
            mSgVarD = &(sg->Rl); break;
         case dudx:
            mSgVarF = &(sg->dudx); break;
         case dudy:
            mSgVarF = &(sg->dudy); break;
         case dvdx:
            mSgVarF = &(sg->dvdx); break;
         case dvdy:
            mSgVarF = &(sg->dvdy); break;
         case time:
            mSgVarF = &(sg->time); break;
         case area:
            mSgVarF = &(sg->area); break;
         case frame:
            mSgVarF = &mFrame; break;
         case sample_frame:
            mTime = &(sg->time);
            mSgVarF = &mSampleFrame;
            break;
         case fps:
            mSgVarF = &mFPS; break;
         case shutter_open_time:
            mSgVarF = &mShutterOpenTime; break;
         case shutter_close_time:
            mSgVarF = &mShutterCloseTime; break;
         case shutter_open_frame:
            mSgVarF = &mShutterOpenFrame; break;
         case shutter_close_frame:
            mSgVarF = &mShutterCloseFrame; break;
         case Ldist:
            mSgVarF = &(sg->Ldist); break;
         case sc:
            mSgVarB = &(sg->sc); break;
         case Rt:
            mSgVarU16 = &(sg->Rt); break;
         case Rr:
            mSgVarB = &(sg->Rr); break;
         case Rr_refl:
            mSgVarB = &(sg->Rr_refl); break;
         case Rr_refr:
            mSgVarB = &(sg->Rr_refr); break;
         case Rr_diff:
            mSgVarB = &(sg->Rr_diff); break;
         case Rr_gloss:
            mSgVarB = &(sg->Rr_gloss); break;
         default:
            AiMsgWarning("[seexpr] Unsupported shader globals \"%s\" (Default to 0)", EnumToName(mWhich));
            break;
         }
      }

      return (mSgVarV != 0 || mSgVarF != 0 || mSgVarI != 0 || mSgVarD != 0);
   }

   inline int which() const
   {
      return mWhich;
   }

   inline const char* name() const
   {
      return EnumToName(mWhich);
   }

protected:
   
   int mWhich;
   bool mIsVec;
   float *mSgVarF;
   AtVector *mSgVarV;
   int *mSgVarI;
   double *mSgVarD;
   AtByte *mSgVarB;
   AtColor *mSgVarC;
   AtUInt16 *mSgVarU16;
   float mFrame;
   float mFPS;
   float mMotionStart;
   float mMotionEnd;
   float mSampleFrame;
   float *mTime;
   float mShutterOpenTime;
   float mShutterCloseTime;
   float mShutterOpenFrame;
   float mShutterCloseFrame;
};


class ArnoldUserVar : public SeExpr2::ExprVarRef
{
public:
   enum
   {
      Float = 0,
      Vector,
      String
   };

   ArnoldUserVar(const std::string &name, int type=Float)
      : SeExpr2::ExprVarRef(SeExpr2::ExprType().Error().Varying())
      , mName(name.c_str())
      , mIsVec(false)
      , mSg(0)
      , mType(AI_TYPE_UNDEFINED)
   {
      switch (type)
      {
      case Float:
         setType(SeExpr2::ExprType().FP(1).Varying());
         break;
      case Vector:
         setType(SeExpr2::ExprType().FP(3).Varying());
         mIsVec = true;
         break;
      case String:
         setType(SeExpr2::ExprType().String().Varying());
      default:
         break;
      }
   }

   virtual ~ArnoldUserVar()
   {
   }

   void _updateType(AtParamValue *value=0)
   {
      AtParamValue tmpVal;
      if (!value)
      {
         value = &tmpVal;
      }

      if (AiUserGetRGBAFunc(mName, mSg, &(value->RGBA)))
      {
         mType = AI_TYPE_RGBA;
      }
      else if (AiUserGetRGBFunc(mName, mSg, &(value->RGB)))
      {
         mType = AI_TYPE_RGB;
      }
      else if (AiUserGetVecFunc(mName, mSg, &(value->VEC)))
      {
         mType = AI_TYPE_VECTOR;
      }
      else if (AiUserGetPntFunc(mName, mSg, &(value->PNT)))
      {
         mType = AI_TYPE_POINT;
      }
      else if (AiUserGetPnt2Func(mName, mSg, &(value->PNT2)))
      {
         mType = AI_TYPE_POINT2;
      }
      else if (AiUserGetFltFunc(mName, mSg, &(value->FLT)))
      {
         mType = AI_TYPE_FLOAT;
      }
      else if (AiUserGetIntFunc(mName, mSg, &(value->INT)))
      {
         mType = AI_TYPE_INT;
      }
      else if (AiUserGetUIntFunc(mName, mSg, &(value->UINT)))
      {
         mType = AI_TYPE_UINT;
      }
      else if (AiUserGetByteFunc(mName, mSg, &(value->BYTE)))
      {
         mType = AI_TYPE_BYTE;
      }
      else if (AiUserGetStrFunc(mName, mSg, &(value->STR)))
      {
         mType = AI_TYPE_STRING;
      }
      else
      {
         mType = AI_TYPE_UNDEFINED;
      }
   }

   virtual void eval(const char **result)
   {
      if (mSg)
      {
         bool queryVal = true;
         AtParamValue value;

         if (mType == AI_TYPE_UNDEFINED)
         {
            _updateType(&value);
            queryVal = false;
         }

         if (mType == AI_TYPE_STRING)
         {
            if (queryVal)
            {
               if (!AiUserGetStrFunc(mName, mSg, &(value.STR)))
               {
                  AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
               }
               else
               {
                  result[0] = value.STR;
                  return;
               }
            }
            else
            {
               result[0] = value.STR;
               return;
            }
         }

         AiMsgWarning("[seexpr] Unsupported type for user variable \"%s\"", mName.c_str());
      }
      else
      {
         AiMsgWarning("[seexpr] Cannot evaluate user variable \"%s\"", mName.c_str());
      }

      result[0] = "";
   }

   virtual void eval(double *result)
   {
      if (mSg)
      {
         bool queryVal = true;
         AtParamValue value;
         
         if (mType == AI_TYPE_UNDEFINED)
         {
            _updateType(&value);
            queryVal = false;
         }

         switch (mType)
         {
         case AI_TYPE_BYTE:
            {
               if (queryVal)
               {
                  if (!AiUserGetByteFunc(mName, mSg, &(value.BYTE)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               float v = float(value.BYTE);
               result[0] = v;
               if (mIsVec)
               {
                  result[1] = v;
                  result[2] = v;
               }
               return;
            }
         case AI_TYPE_INT:
            {
               if (queryVal)
               {
                  if (!AiUserGetIntFunc(mName, mSg, &(value.INT)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               float v = float(value.INT);
               result[0] = v;
               if (mIsVec)
               {
                  result[1] = v;
                  result[2] = v;
               }
               return;
            }
         case AI_TYPE_UINT:
            {
               if (queryVal)
               {
                  if (!AiUserGetUIntFunc(mName, mSg, &(value.UINT)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               float v = float(value.UINT);
               result[0] = v;
               if (mIsVec)
               {
                  result[1] = v;
                  result[2] = v;
               }
               return;
            }
         case AI_TYPE_FLOAT:
            {
               if (queryVal)
               {
                  if (!AiUserGetFltFunc(mName, mSg, &(value.FLT)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               result[0] = value.FLT;
               if (mIsVec)
               {
                  result[1] = value.FLT;
                  result[2] = value.FLT;
               }
               return;
            }
         case AI_TYPE_POINT2:
            {
               if (queryVal)
               {
                  if (!AiUserGetPnt2Func(mName, mSg, &(value.PNT2)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               result[0] = value.PNT2.x;
               if (mIsVec)
               {
                  result[1] = value.PNT2.y;
                  result[2] = 0.0;
               }
               return;
            }
         case AI_TYPE_POINT:
            {
               if (queryVal)
               {
                  if (!AiUserGetPntFunc(mName, mSg, &(value.PNT)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               result[0] = value.PNT.x;
               if (mIsVec)
               {
                  result[1] = value.PNT.y;
                  result[2] = value.PNT.z;
               }
               return;
            }
         case AI_TYPE_VECTOR:
            {
               if (queryVal)
               {
                  if (!AiUserGetVecFunc(mName, mSg, &(value.VEC)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               result[0] = value.VEC.x;
               if (mIsVec)
               {
                  result[1] = value.VEC.y;
                  result[2] = value.VEC.z;
               }
               return;
            }
         case AI_TYPE_RGB:
            {
               if (queryVal)
               {
                  if (!AiUserGetRGBFunc(mName, mSg, &(value.RGB)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               result[0] = value.RGB.r;
               if (mIsVec)
               {
                  result[1] = value.RGB.g;
                  result[2] = value.RGB.b;
               }
               return;
            }
         case AI_TYPE_RGBA:
            {
               if (queryVal)
               {
                  if (!AiUserGetRGBAFunc(mName, mSg, &(value.RGBA)))
                  {
                     AiMsgWarning("[seexpr] Failed to retrieve user variable \"%s\"", mName.c_str());
                     break;
                  }
               }
               result[0] = value.RGBA.r;
               if (mIsVec)
               {
                  result[1] = value.RGBA.g;
                  result[2] = value.RGBA.b;
               }
               return;
            }
         default:
            break;
         }

         AiMsgWarning("[seexpr] Unsupported type for user variable \"%s\"", mName.c_str());
      }
      else
      {
         AiMsgWarning("[seexpr] Cannot evaluate user variable \"%s\"", mName.c_str());
      }

      result[0] = 0.0;
      if (mIsVec)
      {
         result[1] = 0.0;
         result[2] = 0.0;
      }
   }

   bool bind(AtNode *, AtShaderGlobals *sg)
   {
      mSg = sg;
      mType = AI_TYPE_UNDEFINED;
      return true;
   }

   inline const char* name() const
   {
      return mName.c_str();
   }

protected:

   AtString mName;
   bool mIsVec;
   AtShaderGlobals *mSg;
   int mType;
};


class ArnoldShaderVar : public SeExpr2::ExprVarRef
{
public:
   ArnoldShaderVar(const std::string &name, bool isVec=false)
      : SeExpr2::ExprVarRef(SeExpr2::ExprType().FP(isVec ? 3 : 1).Varying())
      , mName(name)
      , mIsVec(isVec)
      , mIndex(0)
      , mValues(0)
   {
   }

   virtual ~ArnoldShaderVar()
   {
   }

   virtual void eval(const char **)
   {
      assert(false);
   }

   virtual void eval(double *result)
   {
      if (mValues && mIndex < mValues->nelements)
      {
         if (mIsVec)
         {
            AtVector v = AiArrayGetVec(mValues, mIndex);
            result[0] = v.x;
            result[1] = v.y;
            result[2] = v.z;
         }
         else
         {
            float v = AiArrayGetFlt(mValues, mIndex);
            result[0] = v;
         }
      }
      else
      {
         result[0] = 0.0;
         if (mIsVec)
         {
            result[1] = 0.0;
            result[2] = 0.0;
         }
      }
   }

   bool bind(AtNode *node, AtShaderGlobals *)
   {
      SeExprData *data = (SeExprData*) AiNodeGetLocalData(node);
      if (data)
      {
         std::map<std::string, unsigned int>::iterator it = data->varindex.find(mName);
         if (it != data->varindex.end())
         {
            if (it->second >= data->numfvars)
            {
               mIndex = it->second - data->numfvars;
            }
            else
            {
               mIndex = it->second;
            }
            return true;
         }
      }
      return false;
   }

   void bindShaderParams(AtArray *fvalues, AtArray *vvalues)
   {
      mValues = (mIsVec ? vvalues : fvalues);
   }

   inline const char* name() const
   {
      return mName.c_str();
   }

protected:

   std::string mName;
   bool mIsVec;
   unsigned int mIndex;
   AtArray *mValues;
};


class ArnoldExpr : public SeExpr2::Expression
{
public:
   
   ArnoldExpr()
      : SeExpr2::Expression()
      , mBound(false)
      , mBoundSg(0)
      , mNode(0)
   {
   }
   
   ArnoldExpr(AtNode *n, const std::string &e)
      : SeExpr2::Expression(e)
      , mBound(false)
      , mBoundSg(0)
      , mNode(n)
   {

      // should all all sg vars here to avoid runtime access
   }

   ArnoldExpr(AtNode *n, AtString e)
      : SeExpr2::Expression(e.c_str())
      , mBound(false)
      , mBoundSg(0)
      , mNode(n)
   {
   }
   
   virtual ~ArnoldExpr()
   {
      clearExternals();
   }
   
   virtual SeExpr2::ExprVarRef* resolveVar(const std::string& name) const
   {
      if (name.length() >= 4 && !strncmp(name.c_str(), "sg::", 4))
      {
         // -> found sg var
         std::string sgname = name.substr(4);
         ArnoldSgVar *var = new ArnoldSgVar(sgname);
         if (var->which() == ArnoldSgVar::undefined)
         {
            AiMsgWarning("[seexpr] Unsupported shader globals \"%s\"", sgname.c_str());
            return 0;
         }
         else
         {
            mSgVars.push_back(var);
            return var;
         }
      }
      else if (name.length() >= 6 && !strncmp(name.c_str(), "user::", 6))
      {
         // without any further specification, use broad vector type
         std::string uname = name.substr(6);
         ArnoldUserVar *var = new ArnoldUserVar(uname, ArnoldUserVar::Vector);
         mUserVars.push_back(var);
         return var;
      }
      else if (name.length() >= 8 && !strncmp(name.c_str(), "user_", 5))
      {
         // new type specific prefices for user attributes
         //   user_v:: -> Float[3]
         //   user_f:: -> Float
         //   user_s:: -> String
         if (!strncmp(name.c_str() + 5, "f::", 3))
         {
            std::string uname = name.substr(8);
            ArnoldUserVar *var = new ArnoldUserVar(uname, ArnoldUserVar::Float);
            mUserVars.push_back(var);
            return var;
         }
         else if (!strncmp(name.c_str() + 5, "v::", 3))
         {
            std::string uname = name.substr(8);
            ArnoldUserVar *var = new ArnoldUserVar(uname, ArnoldUserVar::Vector);
            mUserVars.push_back(var);
            return var;
         }
         else if (!strncmp(name.c_str() + 5, "s::", 3))
         {
            std::string uname = name.substr(8);
            ArnoldUserVar *var = new ArnoldUserVar(uname, ArnoldUserVar::String);
            mUserVars.push_back(var);
            return var;
         }
      }

      SeExprData *data = (SeExprData*) (mNode ? AiNodeGetLocalData(mNode) : 0);
      
      // Note: this code is only called for used variables!
      std::map<std::string, unsigned int>::const_iterator varit = data->varindex.find(name);
      if (varit != data->varindex.end())
      {
         ArnoldShaderVar *var = new ArnoldShaderVar(name, (data ? varit->second >= data->numfvars : false));
         mShaderVars.push_back(var);
         return var;
      }
      else
      {
         AiMsgWarning("[seexpr] Unknown variable \"%s\"", name.c_str());
         return 0;
      }
   }
   
   virtual SeExpr2::ExprFunc* resolveFunc(const std::string&) const
   {
      return 0;
   }

   // Note: resolveVar, resolveFunc are called when compiling the function
   //       or is that fhe first time the function is run???

   bool bindExternals(AtNode *node, AtShaderGlobals *sg)
   {
      if (!boundTo(sg))
      {
         for (std::vector<ArnoldSgVar*>::iterator it=mSgVars.begin(); it!=mSgVars.end(); ++it)
         {
            if (!(*it)->bind(node, sg))
            {
               //AiMsgError("[seexpr] Could not bind shader globals \"%s\"", (*it)->name());
               AiMsgWarning("[seexpr] Could not bind shader globals \"%s\"", (*it)->name());
               return false;
            }
         }
         for (std::vector<ArnoldUserVar*>::iterator it=mUserVars.begin(); it!=mUserVars.end(); ++it)
         {
            if (!(*it)->bind(node, sg))
            {
               //AiMsgError("[seexpr] Could not bind user variable \"%s\"", (*it)->name());
               AiMsgWarning("[seexpr] Could not bind user variable \"%s\"", (*it)->name());
               return false;
            }
         }
         for (std::vector<ArnoldShaderVar*>::iterator it=mShaderVars.begin(); it!=mShaderVars.end(); ++it)
         {
            if (!(*it)->bind(node, sg))
            {
               //AiMsgError("[seexpr] Could not bind shader variable \"%s\"", (*it)->name());
               AiMsgWarning("[seexpr] Could not bind shader variable \"%s\"", (*it)->name());
               return false;
            }
         }
      }
      mBoundSg = sg;
      mBound = true;
      return true;
   }

   bool bindShaderParams(AtArray *fvalues, AtArray *vvalues)
   {
      if (mBound)
      {
         for (std::vector<ArnoldShaderVar*>::iterator it=mShaderVars.begin(); it!=mShaderVars.end(); ++it)
         {
            (*it)->bindShaderParams(fvalues, vvalues);
         }

         return true;
      }
      else
      {
         return false;
      }
   }

   void clearExternals()
   {
      for (std::vector<ArnoldSgVar*>::iterator it=mSgVars.begin(); it!=mSgVars.end(); ++it)
      {
         delete *it;
      }
      for (std::vector<ArnoldUserVar*>::iterator it=mUserVars.begin(); it!=mUserVars.end(); ++it)
      {
         delete *it;
      }
      for (std::vector<ArnoldShaderVar*>::iterator it=mShaderVars.begin(); it!=mShaderVars.end(); ++it)
      {
         delete *it;
      }
      mSgVars.clear();
      mUserVars.clear();
      mShaderVars.clear();
      mBound = false;
   }

   inline size_t numSgVars() const { return mSgVars.size(); }
   inline size_t numUserVars() const { return mUserVars.size(); }
   inline size_t numShaderVars() const { return mShaderVars.size(); }
   inline bool boundTo(AtShaderGlobals *sg) const { return (mBound && mBoundSg == sg); }

private:

   bool mBound;
   mutable std::vector<ArnoldSgVar*> mSgVars;
   mutable std::vector<ArnoldUserVar*> mUserVars;
   mutable std::vector<ArnoldShaderVar*> mShaderVars;
   AtShaderGlobals *mBoundSg;
   AtNode *mNode;
};

// ---

node_parameters
{
   AiParameterStr(SSTR::expression, "");
   AiParameterArray(SSTR::fparam_name, AiArray(0, 0, AI_TYPE_STRING));
   AiParameterArray(SSTR::fparam_value, AiArray(0, 0, AI_TYPE_FLOAT));
   AiParameterArray(SSTR::vparam_name, AiArray(0, 0, AI_TYPE_STRING));
   AiParameterArray(SSTR::vparam_value, AiArray(0, 0, AI_TYPE_VECTOR));
   AiParameterBool(SSTR::stop_on_error, false);
   AiParameterVec("error_value", 1.0f, 0.0f, 0.0f);
}

node_initialize
{
   SeExprData *data = new SeExprData();

   data->varBlockCreator = new SeExpr2::VarBlockCreator();
   data->outputIndex = data->varBlockCreator->registerVariable("__output", SeExpr2::ExprType().FP(3).Varying());

   data->nthreads = 0;
   data->exprs = 0;
   data->outputData = 0;

   AiNodeSetLocalData(node, (void*)data);
}

node_update
{
   SeExprData *data = (SeExprData*) AiNodeGetLocalData(node);

   int nthreads = AiNodeGetInt(AiUniverseGetOptions(), "threads");

   if (data->nthreads > 0)
   {
      for (int i=0; i<data->nthreads; ++i)
      {
         if (data->exprs[i])
         {
            delete data->exprs[i];
         }
         if (data->varBlocks[i])
         {
            delete data->varBlocks[i];
         }
      }

      delete[] data->exprs;
      delete[] data->varBlocks;
      delete[] data->outputData;

      data->exprs = 0;
      data->varBlocks = 0;
      data->outputData = 0;
   }

   data->stopOnError = AiNodeGetBool(node, SSTR::stop_on_error);
   data->valid = false;
   data->constant = false;
   data->threadsafe = false;
   data->mutex = 0;
   data->sgdependent = false;
   data->numfvars = 0;
   data->numvvars = 0;
   data->value = AI_V3_ZERO;
   data->varindex.clear();
   data->nthreads = nthreads;
   data->exprs = new ArnoldExpr*[nthreads];
   data->varBlocks = new SeExpr2::VarBlock*[nthreads];
   data->outputData = new double[3 * nthreads];
   data->source = AiNodeGetStr(node, SSTR::expression);

   ArnoldExpr *expr = new ArnoldExpr(node, data->source);
   // always vector for now
   expr->setDesiredReturnType(SeExpr2::ExprType().FP(3).Varying());
   expr->setVarBlockCreator(data->varBlockCreator); // is this required?

   for (unsigned int tid=0, offset=0; tid<nthreads; ++tid, offset+=3)
   {
      data->exprs[tid] = 0;
      data->varBlocks[tid] = new SeExpr2::VarBlock(data->varBlockCreator->create());
      data->varBlocks[tid]->Pointer(data->outputIndex) = data->outputData + offset;
   }

   std::map<std::string, unsigned int>::iterator varit;
   
   AtArray *fnames = AiNodeGetArray(node, SSTR::fparam_name);
   data->numfvars = fnames->nelements;
   for (unsigned int i=0; i<fnames->nelements; ++i)
   {
      std::string var = AiArrayGetStr(fnames, i);

      varit = data->varindex.find(var);
      
      if (varit != data->varindex.end())
      {
         AiMsgWarning("[seexpr] Variable name already in use \"%s\"", var.c_str());
         data->numfvars = 0;
         data->varindex.clear();
         delete expr;
         return;
      }
      else
      {
         data->varindex[var] = i;
      }
   }
   
   AtArray *vnames = AiNodeGetArray(node, SSTR::vparam_name);
   data->numvvars = vnames->nelements;
   for (unsigned int i=0; i<vnames->nelements; ++i)
   {
      std::string var = AiArrayGetStr(vnames, i);

      varit = data->varindex.find(var);
      
      if (varit != data->varindex.end())
      {
         AiMsgWarning("[seexpr] Variable name already in use \"%s\"", var.c_str());
         data->numfvars = 0;
         data->numvvars = 0;
         data->varindex.clear();
         delete expr;
         return;
      }
      else
      {
         data->varindex[var] = data->numfvars + i;
      }
   }

   if (expr->isValid())
   {
      data->valid = true;
      data->threadsafe = expr->isThreadSafe();
      if (!data->threadsafe)
      {
         AiMsgWarning("[seexpr] Expression for node \"%s\" is not thread safe", AiNodeGetName(node));
         AiCritSecInit(&(data->mutex));
      }

      if (expr->isConstant())
      {
         // No vars or func reference (implies threadsafe)
         data->constant = true;
         data->sgdependent = false;

         // Do not need to bind externals
         expr->evalMultiple(data->varBlocks[0], data->outputIndex, 0, 1);
         
         data->value.x = data->outputData[0];
         data->value.y = data->outputData[1];
         data->value.z = data->outputData[2];
         
         delete expr;
      }
      else
      {
         // Check if expression's input are all constant
         
         char tmp[128];
         bool allParamsConstant = true;
         
         AtArray *fvalues = AiNodeGetArray(node, SSTR::fparam_value);
         if (fvalues->nelements != fnames->nelements)
         {
            if (fvalues->nelements < fnames->nelements)
            {
               AiMsgWarning("[seexpr] More float param variable names than values. Missing values will be set to 0.");
            }
            else
            {
               AiMsgWarning("[seexpr] More float param variable values than names. Extra values will be ignored.");
            }
         }
         
         for (unsigned int i=0; i<fnames->nelements; ++i)
         {
            std::string var = AiArrayGetStr(fnames, i);

            if (expr->usesVar(var))
            {
               sprintf(tmp, "fparam_value[%d]", i);
               if (AiNodeIsLinked(node, tmp))
               {
                  allParamsConstant = false;
               }
            }
         }
         
         AtArray *vvalues = AiNodeGetArray(node, SSTR::vparam_value);
         if (vvalues->nelements != vnames->nelements)
         {
            if (vvalues->nelements < vnames->nelements)
            {
               AiMsgWarning("[seexpr] More vector param variable names than values. Missing values will be set to (0, 0, 0).");
            }
            else
            {
               AiMsgWarning("[seexpr] More vector param variable values than names. Extra values will be ignored.");
            }
         }
         
         for (unsigned int i=0; i<vnames->nelements; ++i)
         {
            std::string var = AiArrayGetStr(vnames, i);

            if (expr->usesVar(var))
            {
               sprintf(tmp, "vparam_value[%d]", i);
               if (AiNodeIsLinked(node, tmp))
               {
                  allParamsConstant = false;
               }
               else
               {
                  sprintf(tmp, "vparam_value[%d].x", i);
                  if (AiNodeIsLinked(node, tmp))
                  {
                     allParamsConstant = false;
                  }
                  else
                  {
                     sprintf(tmp, "vparam_value[%d].y", i);
                     if (AiNodeIsLinked(node, tmp))
                     {
                        allParamsConstant = false;
                     }
                     else
                     {
                        sprintf(tmp, "vparam_value[%d].z", i);
                        if (AiNodeIsLinked(node, tmp))
                        {
                           allParamsConstant = false;
                        }
                     }
                  }
               }
            }
         }

         data->sgdependent = (!allParamsConstant || expr->numSgVars() > 0 || expr->numUserVars() > 0);

         if (!data->sgdependent || !data->threadsafe)
         {
            // Keep current expression as the one to evaluate
            AiMsgDebug("[seexpr] Use same expression object for all thread(s)");
            data->exprs[0] = expr;
         }
         else
         {
            // Create one expression per thread (once we have sg)
            AiMsgDebug("[seexpr] Create expression object per thread");
            delete expr;
         }
      }
   }
   else
   {
      AiMsgWarning("[seexpr] Invalid expression (%s)", expr->parseError().c_str());
      delete expr;
   }
   
   if (!data->valid)
   {
      data->numfvars = 0;
      data->numvvars = 0;
   }
}

node_finish
{
   SeExprData *data = (SeExprData*) AiNodeGetLocalData(node);
   
   if (data->nthreads > 0)
   {
      for (int i=0; i<data->nthreads; ++i)
      {
         if (data->exprs[i])
         {
            delete data->exprs[i];
         }
         if (data->varBlocks[i])
         {
            delete data->varBlocks[i];
         }
      }

      delete[] data->exprs;
      delete[] data->varBlocks;
      delete[] data->outputData;
   }

   delete data->varBlockCreator;

   if (!data->threadsafe && data->mutex)
   {
      AiCritSecClose(&(data->mutex));
   }

   delete data;
}

static AtVector Failed(AtShaderGlobals *sg, AtNode *node, SeExprData *data, bool stopOnError, const char *errMsg=0)
{
   if (!data->threadsafe)
   {
      AiCritSecLeave(&(data->mutex));
   }
   if (stopOnError)
   {
      if (errMsg)
      {
         AiMsgError("[seexpr] %s", errMsg);
      }
      else
      {
         AiMsgError("[seexpr] Failed");
      }
   }
   return AiShaderEvalParamVec(p_error_value);
}

shader_evaluate
{
   SeExprData *data = (SeExprData*) AiNodeGetLocalData(node);

   if (!data->valid)
   {
      if (data->stopOnError)
      {
         AiMsgError("[seexpr] Invalid expression");
      }
      sg->out.VEC = AiShaderEvalParamVec(p_error_value);
   }
   else
   {
      if (data->constant)
      {
         sg->out.VEC = data->value;
      }
      else
      {
         AtVector rv = AI_V3_ZERO;
         
         if (!data->threadsafe)
         {
            AiCritSecEnter(&(data->mutex));
         }
         
         ArnoldExpr *expr = 0;
         
         if (!data->threadsafe || !data->sgdependent)
         {
            expr = data->exprs[0];
         }
         else
         {
            expr = data->exprs[sg->tid];
            if (!expr)
            {
               expr = new ArnoldExpr(node, data->source);
               data->exprs[sg->tid] = expr;   
            }
         }

         if (expr && expr->isValid())
         {
            AtArray *fvalues = AiShaderEvalParamArray(p_fparam_value);
            // if (fvalues->nelements != data->numfvars)
            // {
            //    AiMsgWarning("[seexpr] fparam_name and fparam_value size mismatch (%d for %d)", data->numfvars, fvalues->nelements);
            //    sg->out.VEC = Failed(sg, node, data, data->stopOnError, "Invalid float parameters setup");
            //    return;
            // }

            AtArray *vvalues = AiShaderEvalParamArray(p_vparam_value);
            // if (vvalues->nelements != data->numvvars)
            // {
            //    AiMsgWarning("[seexpr] vparam_name and vparam_value size mismatch (%d for %d)", data->numvvars, vvalues->nelements);
            //    sg->out.VEC = Failed(sg, node, data, data->stopOnError, "Invalid vector parameters setup");
            //    return;
            // }

            if (!expr->bindExternals(node, sg))
            {
               sg->out.VEC = Failed(sg, node, data, data->stopOnError, "Could not bind external parameters");
               return;
            }

            if (!expr->bindShaderParams(fvalues, vvalues))
            {
               sg->out.VEC = Failed(sg, node, data, data->stopOnError, "Could not bind external parameters");
               return;
            }

            expr->evalMultiple(data->varBlocks[sg->tid], data->outputIndex, 0, 1);
            
            unsigned int outputDataOffset = 3 * sg->tid;

            rv.x = data->outputData[outputDataOffset + 0];
            rv.y = data->outputData[outputDataOffset + 1];
            rv.z = data->outputData[outputDataOffset + 2];
         }
         else
         {
            sg->out.VEC = Failed(sg, node, data, data->stopOnError, "Expression is NULL or invalid");
            return;
         }
         
         if (!data->threadsafe)
         {
            AiCritSecLeave(&(data->mutex));
         }
         
         sg->out.VEC = rv;
      }
   }
}
