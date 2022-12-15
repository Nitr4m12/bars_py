import unittest
from src.create import write_bars
from src.bars_ import read_bars

class TestCreate(unittest.TestCase):

    def test_write(self):
        write_bars("./new.bars")

    def test_read(self):
        read_bars("./old.bars")

if __name__ == '__main__':
    unittest.main()
