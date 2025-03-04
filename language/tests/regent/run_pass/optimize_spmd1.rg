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

-- runs-with:
-- [
--   ["-ll:cpu", "4", "-fflow-spmd", "1"],
--   ["-ll:cpu", "2", "-fflow-spmd", "1", "-fflow-spmd-shardsize", "2"]
-- ]

import "regent"

-- This tests the SPMD optimization of the compiler with:
--   * disjoint regions
--   * multiple read-write tasks
--   * constant slice bounds
--   * variable time bounds

local c = regentlib.c

task inc(r : region(int), y : int)
where reads(r), writes(r) do
  for x in r do
    @x += y
  end
end

task check(r : region(int))
where reads(r) do
  for x in r do
    regentlib.c.printf("%d\n", @x)
    regentlib.assert(@x == 70963, "test failed")
  end
end

__demand(__replicable)
task main()
  var r = region(ispace(ptr, 3), int)
  var x0 = dynamic_cast(ptr(int, r), 0)
  var x1 = dynamic_cast(ptr(int, r), 1)
  var x2 = dynamic_cast(ptr(int, r), 2)

  var rc = c.legion_coloring_create()
  c.legion_coloring_add_point(rc, 0, __raw(x0))
  c.legion_coloring_add_point(rc, 1, __raw(x1))
  c.legion_coloring_add_point(rc, 2, __raw(x2))
  var p = partition(disjoint, r, rc)
  c.legion_coloring_destroy(rc)

  for x in r do
    @x = 70000
  end

  var tinit = 0
  var tfinal = 3

  __demand(__spmd)
  for t = tinit, tfinal do
    for i = 0, 3 do
      inc(p[i], 1)
    end
    for i = 0, 3 do
      inc(p[i], 20)
    end
    for i = 0, 3 do
      inc(p[i], 300)
    end
  end

  check(r)
end
regentlib.start(main)
