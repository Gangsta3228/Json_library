import os
import sys

# Добавляем путь к модулю в список sys.path
module_path = os.path.abspath('build\\lib.win-amd64-cpython-310')
sys.path.append(module_path)

import cjson
import json
import ujson

import time

ujson.use_speedups = True


def measure_time_loads(library, json_str):
    start_time = time.time()
    json_loads = library.loads(json_str)
    end_time = time.time()
    execution_time = end_time - start_time
    return execution_time

def measure_time_dumps(library, json_obj):
    start_time = time.time()
    json_dumps = library.dumps(json_obj)
    end_time = time.time()
    execution_time = end_time - start_time
    return execution_time

# Загрузка тестового JSON-файла
with open('test_json_file.json') as file:
    json_str = file.read()

# Десериализация JSON-строки
json_obj = json.loads(json_str)

# Измерение времени выполнения для разных библиотек при десериализации
json_loads_time = measure_time_loads(json, json_str)
ujson_loads_time = measure_time_loads(ujson, json_str)
cjson_loads_time = measure_time_loads(cjson, json_str)

# Измерение времени выполнения для разных библиотек при сериализации
json_dumps_time = measure_time_dumps(json, json_obj)
ujson_dumps_time = measure_time_dumps(ujson, json_obj)
cjson_dumps_time = measure_time_dumps(cjson, json_obj)

# Вывод результатов
print(f"Время выполнения loads с использованием json: {json_loads_time} сек.")
print(f"Время выполнения loads с использованием ujson: {ujson_loads_time} сек.")
print(f"Время выполнения loads с использованием cjson: {cjson_loads_time} сек.")
print(f"Время выполнения dumps с использованием json: {json_dumps_time} сек.")
print(f"Время выполнения dumps с использованием ujson: {ujson_dumps_time} сек.")
print(f"Время выполнения dumps с использованием cjson: {cjson_dumps_time} сек.")

