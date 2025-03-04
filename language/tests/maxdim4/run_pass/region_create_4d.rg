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

task f(r : region(ispace(int4d), double))
where reads(r) do
  var t : double = 0.0
  for i in r do
    t += r[i]
  end
  return t
end

task main()
  var r = region(ispace(int4d, { 2, 2, 2, 2 }), double)
  fill(r, 1)
  var t = f(r)
  regentlib.assert(t == 2 * 2 * 2 * 2, "test failed")
end
regentlib.start(main)
