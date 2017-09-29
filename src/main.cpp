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
#include <cstring>

extern const AtNodeMethods *SeExprMtd;

namespace SSTR
{
   AtString expression("expression");
   AtString fparam_name("fparam_name");
   AtString fparam_value("fparam_value");
   AtString vparam_name("vparam_name");
   AtString vparam_value("vparam_value");
   AtString stop_on_error("stop_on_error");
   AtString linkable("linkable");
   AtString fps("fps");
   AtString motion_start_frame("motion_start_frame");
   AtString motion_end_frame("motion_end_frame");
   AtString frame("frame");
   AtString relative_motion_frame("relative_motion_frame");
   AtString shutter_start("shutter_start");
   AtString shutter_end("shutter_end");
}

node_loader
{
   if (i == 0)
   {
      node->name = "seexpr";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_VECTOR;
      node->methods = SeExprMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   }
   else
   {
      return false;
   }
}
