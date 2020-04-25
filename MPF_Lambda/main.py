from myfitnesspal import Client
import json

client = Client('hussainm@gatech.edu', 'MFP4180')
_id = client.get_food_search_results("thin mints girl scout")[0].mfp_id
item = client.get_food_item_details(_id)
x = vars(item)
x.popitem()
print(x)
print(json.dumps(x))

