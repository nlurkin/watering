#!/usr/bin/env python3
from http.server import HTTPServer, BaseHTTPRequestHandler
from os.path import curdir, sep
import urllib.request
import json
from datetime import datetime, timedelta

clientAddress = "http://192.168.1.9:80/"
clientAddress = "http://localhost:8001/"


class MyHandler(BaseHTTPRequestHandler):
    arduino_buffer = ""
    MQTT_pub = {}

    def do_OPTIONS(self):
        self.send_response(200, "ok")
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, OPTIONS, PUT, DELETE')
        self.send_header("Access-Control-Allow-Headers", "X-Requested-With")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        url = urllib.parse.urlparse(self.path)

        if url.path == "/console":
            self.send_response(200, "OK")
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            self.wfile.write(bytes(MyHandler.arduino_buffer, "utf8"))

            return
        elif url.path == "/MQTT":
            self.send_response(200, "OK")
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Content-type', 'text/html')
            self.end_headers()

            queries = urllib.parse.parse_qs(url.query)
            to_send = {}
            if len(queries) == 0:
                to_send = MyHandler.MQTT_pub
            else:
                elmt = queries["element"][0]
                if "from" in queries:
                    date_from = datetime.strptime(queries["from"][0], '%Y-%m-%d %H:%M:%S')
                else:
                    date_from = datetime.today() - timedelta(days = 1)
                if elmt in MyHandler.MQTT_pub:
                    to_send = {"x": [x[0].strftime('%Y-%m-%d %H:%M:%S') for x in MyHandler.MQTT_pub[elmt] if x[0] > date_from], "y": [x[1] for x in MyHandler.MQTT_pub[elmt]if x[0] > date_from]}

            self.wfile.write(bytes(json.dumps(to_send, default = str), "utf8"))

            return

        try:
            sendReply = False
            if self.path.endswith(".html"):
                mimetype = 'text/html'
                sendReply = True

            if sendReply:
                with open(curdir + sep + self.path) as fd:
                    self.send_response(200, "OK")
                    self.send_header('Content-type', mimetype)
                    self.end_headers()
                    self.wfile.write(bytes(fd.read(), "utf8"))

        except IOError:
            self.send_error(404, 'File Not Found: %s' % self.path)
            self.end_headers()

        return

    def do_POST(self):
        '''Post output from arduino'''
        self.send_response(200, "OK")
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-type', 'text/html')
        self.send_header("Connection", "keep-alive")
        self.send_header("Content-Length", 0)
        self.end_headers()

        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        post_data = post_data.decode("utf8").strip()
        if post_data[:6] == "#MQTT#":
            topic, value = post_data[6:].split(":")
            topic = topic.strip()
            if topic not in MyHandler.MQTT_pub:
                MyHandler.MQTT_pub[topic] = []
            MyHandler.MQTT_pub[topic.strip()].append((datetime.now(), value.strip()))
        else:
            MyHandler.arduino_buffer += post_data.strip() + "\n"
        return

    def do_PUT(self):
        '''Send commands to ARDUINO'''
        self.send_response(200, "OK")
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()

        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        url = clientAddress

        req = urllib.request.Request(url = url, data = post_data, method = 'PUT')
        with urllib.request.urlopen(req):
            pass

    def do_DELETE(self):
        '''Empty buffer'''
        self.send_response(200, "OK")
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()

        MyHandler.arduino_buffer = ""


def run(server_class = HTTPServer, handler_class = MyHandler):
    server_address = ('', 8000)
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()


run()
