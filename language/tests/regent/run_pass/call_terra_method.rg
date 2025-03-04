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

struct s {
  x : int
}

terra s:f(y : int)
  self.x = self.x + y
  return self.x
end

task g() : int
  var y = s { x = 0 }
  var a = y:f(1)
  var b = y:f(10)
  var c = y:f(100)
  return a + b + c
end

task main()
  regentlib.assert(g() == 123, "test failed")
end
regentlib.start(main)
