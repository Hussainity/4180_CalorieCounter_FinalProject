from myfitnesspal import Client
import json

def handler(event, context):
    client = Client('hussainm@gatech.edu', 'MFP4180')
    print(event)
    _id = client.get_food_search_results(event['queryStringParameters']['query'])[0].mfp_id
    item = client.get_food_item_details(_id)
    x = vars(item)
    x.popitem()
    return {
            'statusCode': 200,
            'body': json.dumps(x)
          }
