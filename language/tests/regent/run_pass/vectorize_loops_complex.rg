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

task main()
  var r = region(ispace(int1d, 32), complex)

  fill(r, complex {1.0, 2.0})

  -- __demand(__vectorize)
  for x in r do
    var y = @x + 1
    @x = @x * y - 4
  end

  for x in r do
    regentlib.assert(x.real == -6.0, "test failed")
    regentlib.assert(x.imag ==  6.0, "test failed")
  end
end
regentlib.start(main)
