#!/bin/env python
'''
Created on 19-Jun-2020

@author: Nicolas Lurkin
'''

from flask import Flask, request, g
from flask import make_response, jsonify
from werkzeug.exceptions import abort
from werkzeug.serving import WSGIRequestHandler

from mongodb import myMongoClient
from datetime import datetime

server = Flask(__name__)


def get_db():
    if "db" not in g:
        g.db = myMongoClient("localhost", 27017)
        g.db.connect()

    return g.db


def close_db(e = None):
    db = g.pop('db', None)

    if db is not None:
        db.close()


@server.route("/", methods = ["GET"])
def home():
    print("Home route")
    return "<h1>Welcome to the waterdash Data-Server</h1>"


@server.route('/api/v1/<sensor_name>', methods = ["POST"])
def api_sensor(sensor_name):
    db = get_db()
    sensor_doc = db.sensors_db.find_one({"sensor": sensor_name})
    
    if not sensor_doc:
        return abort(404, description = f"Sensor {sensor_name} does not exist")

    print(sensor_name, request.data)
    if request.method == "POST":
        sensor_coll = db.client["sensors"][sensor_name]
        ts = datetime.now().timestamp()
        day = datetime.now().strftime("%Y-%m-%d")
        val = request.data.decode("utf8")
        if sensor_doc["data-type"] == "float":
            val = float(val)
        elif sensor_doc["data-type"] == "bool":
            val = int(val)
            
        sample = {"val": val, "ts": ts}
        sensor_coll.update_one({"sensorid": str(sensor_doc["_id"]), "nsamples": {"$lt": 200}, "day": day},
                              {"$push": { "samples": sample},
                               "$min": { "first": ts},
                               "$max": { "last": ts},
                               "$inc": { "nsamples": 1}
                               },
                              upsert = True)
        return jsonify({"status": 'Success'}), 200


@server.errorhandler(404)
def not_found(error):
    return make_response(jsonify({'error': str(error)}), 404)


if __name__ == '__main__':
    WSGIRequestHandler.protocol_version = "HTTP/1.1"
    server.run(host="192.168.1.20", port = 8000, debug = True)
