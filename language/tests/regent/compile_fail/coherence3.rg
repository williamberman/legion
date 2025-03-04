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
-- coherence3.rg:40: conflicting coherence modes: atomic($r.g.d.a) and simultaneous($r.g)
-- where atomic(r.g.d.a), simultaneous(r.g) do
--                                   ^

import "regent"

struct s {
  a : int,
  b : int,
  c : int,
}

struct t {
  d : s,
  e : s,
  f : s,
}

struct u {
  g : t,
  h : t,
}

task f(r : region(u))
where atomic(r.g.d.a), simultaneous(r.g) do
end
