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
-- check_parallelizable_inadmissible4.rg:27: vectorization failed: found an inadmissible statement
--     @x = 1
--     ^

import "regent"

task f(r : region(ispace(int1d), int))
where reads writes(r) do
  var x : &int
  __demand(__vectorize)
  for e in r do
    @x = 1
  end
end
