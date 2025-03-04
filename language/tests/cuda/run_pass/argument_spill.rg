-- Copyright 2022 Stanford University, NVIDIA
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

-- runs-with:
-- [["-ll:gpu", "1"]]

import "regent"

__demand(__cuda)
task f(r : region(ispace(int1d), int))
where reads writes(r)
do
  var arr : int[1088]
  arr[0] = 1234
  arr[1024] = 4321
  for e in r do
    r[e] = arr[0] + arr[1024]
  end
end

task toplevel()
  var r = region(ispace(int1d, 5), int)
  f(r)
  regentlib.assert(r[0] == 5555, "test failed")
end

regentlib.start(toplevel)
