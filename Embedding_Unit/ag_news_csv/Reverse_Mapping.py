import pymongo
import pandas as pd
import numpy as np
import csv


def set_MongoDB(MongoDB_URI):
    # Replace YOUR_MONGODB_URI with the URI of your MongoDB instance
    client = pymongo.MongoClient(MongoDB_URI)
    # Create a new database called 'ag_news'
    db = client['ag_news']
    collection = db['articles']
    return collection

def store_data(collection, Path):
    with open(Path, 'r') as file:
        reader = csv.reader(file)
        next(reader)  # Skip header row
        for row in reader:
            article = {
                'title': row[1],
                'content': row[2],
                'category': row[0],
                'ID': row[3]
            }
            collection.insert_one(article)

def get_documents_by_ids(collection, id_list):

    documents = list(collection.find({"ID": {"$in": id_list}}))

    return documents[:]['content']

def __main__():
    collection = set_MongoDB('mongodb+srv://abdelrahmanfawzy:voE5L9onJ2xW94wY@cluster0.tnz5xmg.mongodb.net/test')
    #store_data(collection, './AG_News.csv')
    documents = get_documents_by_ids(collection, ['1', '2', '3'])
    print(documents)

if __name__ == '__main__':
    __main__()