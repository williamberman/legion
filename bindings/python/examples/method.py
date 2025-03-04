#!/usr/bin/env python3

# Copyright 2022 Stanford University
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from __future__ import print_function

from pygion import task

class A:
    # Note: @task is like @staticmethod so no 'self' argument
    @task
    def static():
        print("Hello from A.static()!")

@task
def main():
    A.static()

    a = A()
    a.static()

if __name__ == '__main__':
    main()
