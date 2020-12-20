#!/bin/env python
'''
Created on 19-Jun-2020

@author: Nicolas Lurkin
'''

# TODO fix timezones
# TODO merge mongoDB documents when reading: maybe use first>now-24h or now-requested_range instead of using day

from flask import Flask, request, g
from flask import make_response, jsonify
from werkzeug.exceptions import abort
from werkzeug.serving import WSGIRequestHandler

from mongodb import myMongoClient
from datetime import datetime
import requests
import pytz
import pprint
import re

import logging

# These two lines enable debugging at httplib level (requests->urllib3->http.client)
# You will see the REQUEST, including HEADERS and DATA, and RESPONSE with HEADERS but without DATA.
# The only thing missing will be the response.body which is not logged.
try:
    import http.client as http_client
except ImportError:
    # Python 2
    import httplib as http_client
http_client.HTTPConnection.debuglevel = 1

# You must initialize logging, otherwise you'll not see debug output.
logging.basicConfig()
logging.getLogger().setLevel(logging.DEBUG)
requests_log = logging.getLogger("requests.packages.urllib3")
requests_log.setLevel(logging.DEBUG)
requests_log.propagate = True

server = Flask(__name__)

clientAddress = "http://192.168.0.27:80/"


def get_db():
    if "db" not in g:
        g.db = myMongoClient("192.168.0.18", 27017)
        g.db.connect()

    return g.db


@server.teardown_appcontext
def teardown_db(exception):
    db = g.pop('db', None)

    if db is not None:
        db.close()


@server.route("/api/v1/controlcheck", methods = ["GET"])
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
            req = requests.put(url = clientAddress,
                                         data = f"/api/v1/{controller['sensor']}:{last_sp['val']}\r\n".encode("ASCII"),
                                         headers = {"Content-Type": "text/html",
                                                    "User-Agent": None})

            return "<h1>Control updated</h1>"
    return "<h1>Control checked</h1>"

def get_dtype(charID):
    if charID == 'B':
        return "bool"
    elif charID == "D":
        return "float"
    elif charID == "I":
        return "int"
    else:
        return "???"
    
@server.route("/api/v1/advertise", methods = ["POST"])
def advertise():
    db = get_db()
    sensors = request.data.decode("utf8").strip(";").split(";")

    for sensor in sensors:
        m = re.findall("(.*):\(([DIB]),([01])\)", sensor)
        if len(m)>0:
            m = m[0]
            data_dict = {"sensor": m[0], "data-type": get_dtype(m[1]), "controller": True if m[2] == "1" else False}
            print(data_dict)
            db.add_advertised_current(data_dict, m[0])
    return jsonify({"status": 'Success'}), 200


@server.route("/api/v1/advertised/<which>", methods = ["GET"])
def list_advertised(which):
    db = get_db()

    if which == "current":
        slist = db.get_advertised_current_list()
    elif which == "all":
        slist = db.get_advertised_all_list()
    else:
        return make_response(jsonify({'error': str(error)}), 404)

    return jsonify(slist), 200


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
        ts = pytz.utc.localize(datetime.now()).timestamp()
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
    db = myMongoClient("192.168.0.18", 27017)
    db.connect()
    db.clear_advertised_current()
    db.close()
    server.run(host = "192.168.0.15", port = 8000, debug = True)
