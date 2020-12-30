![Build](https://github.com/lomik/python-tarantool_snapshot/workflows/Build/badge.svg)

## Description

[Tarantool](https://github.com/tarantool/tarantool) snapshot reader.

FOR TARANTOOL 1.5

Version for tarantool 1.6 and later: https://github.com/viciious/python-tarantool16_snaphot

## Build and installation

```sh
git clone https://github.com/lomik/python-tarantool_snapshot.git
cd python-tarantool_snapshot
make install PYTHON=python3
# or
make bdist_rpm PYTHON=python3
rpm -ivh dist/python3-tarantool-snapshot-1.0-1.x86_64.rpm
```

## Usage

```python

import tarantool_snapshot

count = 0
for space_id, tuple_data in tarantool_snapshot.iter("/snaps/00000000010388786179.snap"):
  count += 1

print count

```
