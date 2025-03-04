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
  var c = ispace(int1d, 4)
  var r = region(ispace(int1d, 8), int)
  var p = partition(equal, r, c)

  var x = 123
  __parallelize_with p
  do
    var x = 456
    regentlib.assert(x == 456, "test failed")
  end
  regentlib.assert(x == 123, "test failed")
end
regentlib.start(main)
