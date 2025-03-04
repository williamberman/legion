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

extern task f()
f:compile()

extern task g() : int
g:compile()

extern task h(r : region(int)) where reads(r) end
h:compile()

extern task i(r : region(int)) : int where reads writes(r) end
i:compile()

-- These are just declarations, nothing left to test.
task main()
end
regentlib.start(main)
