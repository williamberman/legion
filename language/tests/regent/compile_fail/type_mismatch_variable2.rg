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
-- type_mismatch_variable2.rg:24: ptr expected region(int32) as argument 2, got region(ispace(int2d), int32)
--   var x : ptr(int, r)
--     ^

import "regent"

task main()
  var r = region(ispace(int2d, {2, 2}), int)
  var x : ptr(int, r)
end
regentlib.start(main)
