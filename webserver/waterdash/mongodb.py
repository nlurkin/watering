'''
Created on 18-Jun-2020

@author: Nicolas Lurkin
'''

import pymongo
from bson.objectid import ObjectId
from tzlocal import get_localzone
from datetime import datetime
import pytz
tz = get_localzone()


def to_utc(dt = None):
    if dt is None:
        return tz.localize(datetime.now()).astimezone(pytz.utc)
    return tz.localize(dt).astimezone(pytz.utc)


def from_utc(dt):
    return None  # tz.localize(datetime.now()).astimezone(pytz.utc)


class myMongoClient(object):

    def __init__(self, hostname, port, username = None, password = None):
        self.hostname = hostname
        self.port = port
        self.client = None
        self.dbuser = username
        self.dbpasswd = password

        self.sensors_db = None
        self.advertised_cur_db = None
        self.advertised_all_db = None
        self.dashboard_db = None

    def connect(self):
        self.client = pymongo.MongoClient(self.hostname, self.port)

        if self.dbuser is not None:
            self.client["perm"].authenticate(self.dbuser, self.dbpasswd)

        self.sensors_db = self.client["sensors"]["definition"]
        self.dashboard_db = self.client["dashboard"]["dashboard_config"]
        self.advertised_cur_db = self.client["advertised"]["current"]
        self.advertised_all_db = self.client["advertised"]["all"]

    def close(self):
        self.client.close()

    def get_sensor_by_id(self, sensor_id):
        if isinstance(sensor_id, list):
            return {str(_["_id"]): _ for _ in self.sensors_db.find({"_id": {"$in": [ObjectId(_) for _ in sensor_id]}})}
        else:
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
            return self.dashboard_db.find({"_id": {"$in": [ObjectId(_) for _ in dashboard_id]}})
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
        return self.client["sensors"][sensor_name].find_one({"day": {"$lte": day}},
                                                            {"samples": 1, "setpoint": 1, "_id": 0},
                                                            sort = [("day", pymongo.DESCENDING), ("last", pymongo.DESCENDING)])

    def get_sensors_list(self):
        return list(self.sensors_db.find({}))

    def get_controllers_list(self):
        return list(self.sensors_db.find({"controller": True}))

    def update_sensor_values(self, sensor_doc, sensor_name, val):
        sensor_coll = self.client["sensors"][sensor_name]
        dt = tz.localize(datetime.now())
        ts = dt.timestamp()
        day = dt.strftime("%Y-%m-%d")

        sample = {"val": val, "ts": ts}
        sensor_coll.update_one({"sensorid": str(sensor_doc["_id"]), "nsamples": {"$lt": 200}, "day": day},
                              {"$push": { "samples": sample},
                               "$min": { "first": ts},
                               "$max": { "last": ts},
                               "$inc": { "nsamples": 1}
                               },
                              upsert = True)

    def update_controller_values(self, sensor_doc, sensor_name, val):
        sensor_coll = self.client["sensors"][sensor_name]

        dt = to_utc()
        ts = dt.timestamp()
        day = dt.strftime("%Y-%m-%d")

        sample = {"val": val, "ts": ts}
        sensor_coll.update_one({"sensorid": str(sensor_doc["_id"]), "nsamples": {"$lt": 200}, "day": day},
                              {"$push": { "setpoint": sample},
                               "$min": { "first": ts},
                               "$max": { "last": ts},
                               "$setOnInsert": { "nsamples": 0}
                               },
                              upsert = True)

    def get_controller_values(self, sensor_name, day):
        doc = self.client["sensors"][sensor_name].find_one({"day": {"$lte": day}, "setpoint": {"$exists": True}},
                                                            {"setpoint": 1, "_id": 0},
                                                            sort = [("day", pymongo.DESCENDING), ("last", pymongo.DESCENDING)])
        if doc:
            return doc["setpoint"]
        return None

    def clear_advertised_current(self):
        self.advertised_cur_db.drop()
        self.advertised_cur_db = self.client["advertised"]["current"]

    def add_advertised_current(self, sensor_data, sensor_name):
        if self.advertised_cur_db.find_one(sensor_data) is not None:
            # Already seen recently
            print("Already seen")
            return

        sensor_doc = self.advertised_cur_db.insert_one(sensor_data)
        if sensor_doc:
            del sensor_data['_id']
            if self.advertised_all_db.find_one(sensor_data) is not None:
                # Already seen someday
                return
            else:
                self.advertised_all_db.insert_one(sensor_data)
            return sensor_doc.inserted_id
        else:
            return None

    def get_advertised_current_list(self):
        return list(self.client["advertised"]["current"].find({}, {"_id": 0}))

    def get_advertised_all_list(self):
        return list(self.advertised_all_db.find({}, {"_id": 0}))

