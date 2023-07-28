import os
import sys
import unittest

module_path = os.path.abspath('build\\lib.win-amd64-cpython-310')
sys.path.append(module_path)

import cjson
import json
import ujson


class JsonTestCase(unittest.TestCase):
    def test_loads(self):
        json_string = '{"key": "value"}'
        expected_result = {"key": "value"}
        result = cjson.loads(json_string)
        self.assertEqual(result, expected_result)

    def test_dumps(self):
        data = {"key": "value"}
        expected_result = '{"key": "value"}'
        result = cjson.dumps(data)
        self.assertEqual(result, expected_result)

    def test_loads_with_another_parsers(self):
        json_str = '{"hello": 10, "world": "value"}'
        expected_result = {"hello": 10, "world": "value"}

        json_doc = json.loads(json_str)
        ujson_doc = ujson.loads(json_str)
        cjson_doc = cjson.loads(json_str)

        self.assertEqual(json_doc, expected_result)
        self.assertEqual(ujson_doc, expected_result)
        self.assertEqual(cjson_doc, expected_result)

    def test_dumps_with_another_parsers(self):
        test_str = '{"hello": 10, "world": "value"}'

        json_doc = json.loads(test_str)
        cjson_doc = cjson.loads(test_str)
        json_str = json.dumps(json_doc)
        cjson_str = cjson.dumps(cjson_doc)

        self.assertEqual(test_str, cjson_str)
        self.assertEqual(json_str, cjson_str)


if __name__ == '__main__':
    unittest.main()
