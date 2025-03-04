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
-- type_mismatch_call_region3.rg:29: type mismatch in argument 3: expected ptr(int32, $r) but got ptr(int32, $t)
--   f(r, s, u)
--   ^

import "regent"

task f(x : region(int), y : region(int), z : ptr(int, x)) end

task g()
  var r = region(ispace(ptr, 5), int)
  var s = region(ispace(ptr, 5), int)
  var t = s
  var u = dynamic_cast(ptr(int, t), 0)
  f(r, s, u)
end

g()
