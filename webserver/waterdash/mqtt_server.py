#!/bin/env python
"""
Created on 19-Jun-2020

@author: Nicolas Lurkin
"""

# TODO fix timezones
# TODO merge mongoDB documents when reading: maybe use first>now-24h or now-requested_range instead of using day

import logging
import re
import time

import paho.mqtt.client as mqtt

from data.config import broker_creds, mongo_creds
from mongodb import myMongoClient, to_utc

db = None
client = None


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("arduino/#")


def on_message(client, userdata, msg):
    if msg.payload[0] == 0:
        msg.payload = msg.payload[2:]
    if msg.topic.startswith("arduino/cmd"):
        # These are our own messages
        pass
    elif msg.topic.startswith("arduino/"):
        sensor = msg.topic.replace("arduino/", "")
        if sensor == "advertise":
            advertise(msg.payload)
        else:
            api_sensor(sensor, msg.payload)
    else:
        print(
            "Unknown topic: "
            + msg.topic
            + " "
            + str(msg.payload)
            + " "
            + msg.payload.decode("UTF8")
        )


def on_log(client, userdata, level, buff):
    print("log: ", buff)


def get_db():
    global db
    if db is None:
        print("Connecting to DB", end="")
        db = myMongoClient(
            mongo_creds["server"],
            27017,
            mongo_creds["app_name"],
            mongo_creds["username"],
            mongo_creds["password"],
        )
        print("..")
        db.connect()
        print("Connected")

    return db


def get_client():
    global client

    if client is None:
        client = mqtt.Client()
        client.on_connect = on_connect
        client.on_message = on_message
        client.on_log = on_log

        client.username_pw_set(broker_creds["username"], broker_creds["password"])

        client.connect(broker_creds["server"], 1883, 60)
        client.loop_start()

    return client


def teardown_db():
    global db

    if db is not None:
        db.close()
        db = None


def teardown_client():
    global client

    if client is not None:
        client.disconnect()
        client = None


def api_sensor(sensor_name, value):
    db = get_db()
    sensor_doc = db.sensors_db.find_one({"sensor": sensor_name})

    if not sensor_doc:
        print(f"Sensor {sensor_name} does not exist")
        return

    print(sensor_name, value)

    val = value.decode("utf8")
    print(val)
    if sensor_doc["data-type"] == "float":
        val = float(val)
    elif sensor_doc["data-type"] == "bool":
        val = int(val)
    db.update_sensor_values(sensor_doc, sensor_name, val)


def check_control():
    db = get_db()
    ctrl_list = db.get_controllers_list()
    day = to_utc().strftime("%Y-%m-%d")
    for controller in ctrl_list:
        ctrl_values = db.get_sensor_values(controller["sensor"], day)
        if not "setpoint" in ctrl_values:
            continue
        last_sp = ctrl_values["setpoint"][-1]

        if (
            not "samples" in ctrl_values
            or ctrl_values["samples"][-1]["val"] != last_sp["val"]
        ):
            client = get_client()
            client.publish(
                f"arduino/cmd/{controller['sensor']}",
                payload=f"{last_sp['val']}".encode("ASCII"),
                retain=True,
            )


def control_loop():

    while True:
        check_control()
        time.sleep(10)


def get_dtype(charID):
    if charID == "B":
        return "bool"
    elif charID == "D":
        return "float"
    elif charID == "I":
        return "int"
    else:
        return "???"


def advertise(data):
    db = get_db()
    sensors = data.decode("utf8").strip(";").split(";")

    for sensor in sensors:
        m = re.findall("(.*):\(([DIB]),([01])\)", sensor)
        if len(m) > 0:
            m = m[0]
            data_dict = {
                "sensor": m[0],
                "data-type": get_dtype(m[1]),
                "controller": True if m[2] == "1" else False,
            }
            db.add_advertised_current(data_dict, m[0])


if __name__ == "__main__":
    logging.info("Starting MQTT server")
    get_db().clear_advertised_current()

    try:
        get_client()
        control_loop()
    except KeyboardInterrupt:
        teardown_db()
        teardown_client()
