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
-- dynamic_cast_fail.rg:40: $p_aliased is not a disjoint partition

import "regent"

task main()
  var r = region(ispace(int1d, 4), int)
  var x0 = dynamic_cast(int1d(int, r), 0)
  var x1 = dynamic_cast(int1d(int, r), 1)
  var x2 = dynamic_cast(int1d(int, r), 2)
  var x3 = dynamic_cast(int1d(int, r), 3)
  var s = region(ispace(int1d, 4), int1d(int, r))
  var y0 = dynamic_cast(int1d(int1d(int, r), s), 0)
  var y1 = dynamic_cast(int1d(int1d(int, r), s), 1)
  var y2 = dynamic_cast(int1d(int1d(int, r), s), 2)
  var y3 = dynamic_cast(int1d(int1d(int, r), s), 3)

  @y0 = x0
  @y1 = x2
  @y2 = x2
  @y3 = x3

  var q = partition(equal, s, ispace(int1d, 4))
  var p_aliased = image(r, q, s)
  var cs = p_aliased.colors
  var p_disjoint = dynamic_cast(partition(disjoint, r, cs), p_aliased)
end
regentlib.start(main)
