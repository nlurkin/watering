'''
Created on 18-Jun-2020

@author: Nicolas Lurkin
'''

import pymongo
from bson.objectid import ObjectId


class myMongoClient(object):

    def __init__(self, hostname, port):
        self.hostname = hostname
        self.port = port
        self.client = None
        self.sensors_db = None
        self.dashboard_db = None

    def connect(self):
        self.client = pymongo.MongoClient(self.hostname, self.port)

        self.sensors_db = self.client["sensors"]["definition"]
        self.dashboard_db = self.client["dashboard"]["dashboard_config"]

    def close(self):
        self.client.close()

    def get_sensor_by_id(self, sensor_id):
        return self.sensors_db.find_one({"_id": ObjectId(sensor_id)})
    
    def get_sensor_by_name(self, sensor_name):
        return self.sensors_db.find_one({"sensor": sensor_name})
    
    def add_sensor(self, sensor_doc):
        sensor_doc = self.sensors_db.insert_one(sensor_doc)
        if sensor_doc:
            return sensor_doc.inserted_id
        else:
            return None
        
    def update_sensor_by_id(self, sensor_id, updated_data):
        self.sensors_db.update_one({"_id": ObjectId(sensor_id)}, {"$set": updated_data})
        
    def get_dashboard_by_id(self, dashboard_id):
        if isinstance(dashboard_id, list):
            return self.sensors_db.find({"_id": {"$in": [ObjectId(_) for _ in dashboard_id]}})
        else:
            return self.dashboard_db.find_one({"_id": ObjectId(dashboard_id)})
    
    def get_dashboard_by_name(self, dashboard_name):
        return self.dashboard_db.find_one({"name": dashboard_name})
    
    def add_dashboard(self, dashboard_doc):
        dashboard_doc = self.dashboard_db.insert_one(dashboard_doc)
        if dashboard_doc:
            return dashboard_doc.inserted_id
        else:
            return None

    def update_dashboard_by_id(self, dashboard_id, updated_data):
        self.dashboard_db.update_one({"_id": ObjectId(dashboard_id)}, {"$set": updated_data})
        
        
    def get_sensors_dropdown(self):
        return [{"label":sensor["sensor"], "value":str(sensor["_id"])} for sensor in self.sensors_db.find({}, {"sensor":1, "_id": 1})]
    
    def get_dashboards_dropdown(self):
        return [{"label": db["name"], "value": str(db["_id"])} for db in self.dashboard_db.find({}, {"name":1, "_id": 1})]
    
    
    def get_sensor_values(self, sensor_name, day):
        return self.client["sensors"][sensor_name].find_one({"day": {"$lte": day}}, {"samples": 1, "_id": 0})
    
    
    
    
    
    
    
    
    
    