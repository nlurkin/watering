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
import urllib.request

server = Flask(__name__)

clientAddress = "http://192.168.0.27:80/"


def get_db():
    if "db" not in g:
        g.db = myMongoClient("localhost", 27017)
        g.db.connect()

    return g.db


def close_db(e = None):
    db = g.pop('db', None)

    if db is not None:
        db.close()


@server.route("/control", methods = ["GET"])
def check_control():
    db = get_db()
    ctrl_list = db.get_controllers_list()
    day = datetime.now().strftime("%Y-%m-%d")
    for controller in ctrl_list:
        ctrl_values = db.get_sensor_values(controller["sensor"], day)
        if not "setpoint" in ctrl_values:
            continue
        last_sp = ctrl_values["setpoint"][-1]

        if not "samples" in ctrl_values or ctrl_values["samples"][-1]["val"] != last_sp["val"]:
            req = urllib.request.Request(url = clientAddress, data = f"/api/v1/{controller['sensor']}:{last_sp['val']}".encode("ASCII"), method = 'PUT')
            with urllib.request.urlopen(req):
                pass

            return "<h1>Control updated</h1>"
    return "<h1>Control checked</h1>"


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
        ts = datetime.now().timestamp()
        day = datetime.now().strftime("%Y-%m-%d")
        val = request.data.decode("utf8")
        if sensor_doc["data-type"] == "float":
            val = float(val)
        elif sensor_doc["data-type"] == "bool":
            val = int(val)
        db.update_sensor_values(sensor_doc, sensor_name, val, day, ts)

        return jsonify({"status": 'Success'}), 200


@server.errorhandler(404)
def not_found(error):
    return make_response(jsonify({'error': str(error)}), 404)


if __name__ == '__main__':
    WSGIRequestHandler.protocol_version = "HTTP/1.1"
    server.run(host = "192.168.0.18", port = 8000, debug = True)
