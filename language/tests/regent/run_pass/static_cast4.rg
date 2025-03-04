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

local c = regentlib.c

task main()
  var t = region(ispace(ptr, 5), int)
  var tp = partition(equal, t, ispace(int1d, 2))
  var t0 = tp[0]
  var t1 = tp[1]

  var x = dynamic_cast(ptr(int, t), 0)
  var x0 = static_cast(ptr(int, t0), x)
  regentlib.assert(isnull(x0), "test failed")
end
regentlib.start(main)
