-- Copyright 2022 Stanford University
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

-- fails-with:
-- type_mismatch_projection5.rg:31: type mismatch: expected string for renaming but found table
--   var s = r.{[name]=x}
--              ^

import "regent"

struct fs
{
  x : int;
}

local name = regentlib.field_path("x", "y")

task f()
  var r = region(ispace(int1d, 5), fs)
  var s = r.{[name]=x}
end
f:compile()
