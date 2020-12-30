import os
import unittest
import tarantool_snapshot

class TestTarantoolSnapshot(unittest.TestCase):

  def test_iter(self):
    d = os.path.dirname(__file__)
    filename = os.path.join(d,"00000000006180244890.snap")

    messages = tarantool_snapshot.iter(filename)
    messages = list(tarantool_snapshot.iter(filename))

    self.assertEqual(len(messages), 1)
    self.assertEqual(messages[0][0], 23)
    self.assertEqual(len(messages[0][1][0]), 10)
    self.assertEqual(len(messages[0][1][1]), 16)
    self.assertEqual(len(messages[0][1][2]), 9)
    self.assertEqual(len(messages[0][1][3]), 1049)
    self.assertEqual(messages[0][1][0].decode("utf-8"), "v:b52a1d2d")

  def test_wrong_file(self):
    try:
      list(tarantool_snapshot.iter("not_exists_file.snap"))
    except tarantool_snapshot.SnapshotError:
      self.assertTrue(True)
      return
    self.assertTrue(False)
