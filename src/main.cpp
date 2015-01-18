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

extern AtNodeMethods *agSeExprMtd;

node_loader
{
   if (i == 0)
   {
      node->name = "seexpr";
      node->node_type = AI_NODE_SHADER;
      node->output_type = AI_TYPE_VECTOR;
      node->methods = agSeExprMtd;
      strcpy(node->version, AI_VERSION);
      return true;
   }
   else
   {
      return false;
   }
}
