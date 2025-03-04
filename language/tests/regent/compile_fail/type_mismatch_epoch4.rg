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
-- type_mismatch_epoch4.rg:30: type mismatch between int32 and {}
--       y += g(4)
--       ^

import "regent"

task g(x : int) : int
  return x
end

task f()
  var y = 3
  must_epoch
    for i = 0, 3 do
      y += g(4)
    end
  end
end
