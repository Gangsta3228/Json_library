from faker import Faker
import json
from decimal import Decimal
from datetime import date

fake = Faker()

# Генерация большого количества JSON-объектов
data = [fake.profile() for _ in range(30000)]

# Функция-кодировщик JSON для объектов Decimal и date
def custom_encoder(obj):
    if isinstance(obj, Decimal):
        return float(obj)
    elif isinstance(obj, date):
        return obj.isoformat()
    raise TypeError(f'Object of type {obj.__class__.__name__} is not JSON serializable')

# Преобразование в строку JSON с использованием функции-кодировщика
json_str = json.dumps(data, default=custom_encoder)

# Запись в файл
with open('test_json_file.json', 'w') as file:
    file.write(json_str)