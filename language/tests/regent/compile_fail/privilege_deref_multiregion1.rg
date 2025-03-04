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
-- privilege_deref_multiregion1.rg:24: invalid privilege reads($s) for dereference of ptr(int32, $r, $s)
--   return @x
--          ^

import "regent"

task f(r : region(int), s : region(int), x : ptr(int, r, s))
where reads(r) do
  return @x
end
f:compile()
