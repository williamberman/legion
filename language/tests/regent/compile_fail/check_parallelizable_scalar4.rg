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
-- check_parallelizable_scalar4.rg:28: vectorization failed: found a loop-carried dependence
--       s = 1
--       ^

import "regent"

task f(r : region(ispace(int1d), int))
where reads writes(r) do
  var s = 0
  __demand(__vectorize)
  for e1 in r.ispace do
    for e2 in r.ispace do
      s = 1
    end
  end
end
