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

import "regent"

struct point {
  x : int,
  y : int,
  z : int,
}

task f(p : point) : int
  return p.x + p.y + p.z
end

task g(off : int) : point
  return point { off + 1, off + 20, off + 300 }
end

task main()
  regentlib.assert(f(g(0)) == 321, "test failed")
end
regentlib.start(main)
